/*  =====================================================================
    $File: win32_handmade.cpp
    $Creation Date: 2014-12-27
    $Last Modified: 2014-12-28 17:14
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2014 Cristián Donoso $
    ===================================================================== */

#include <windows.h>
#include <stdint.h>
#include <xinput.h>

// We rename static to some aliases to make more transparent the use of each
#define internal static           // Makes functions scoped to the 'translation unit'
#define global_variable static    // A variable available to all (or many) scopes
#define local_persist static      // A scoped-variable that survives such scope

// Convenient typedef taken from stdint.h
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_offscreen_buffer
{
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int pitch;
};

struct win32_window_dimension {
  int width;
  int height;
};

/**
 * We make a windows binding ourselves. We this we bypass an import librabry.
 * We do this in the case of XInput because the required library changes according to
 * the version of Windows the user is using, so we would like the game to function
 * independent of the dll that's really required.
 *
 * What we do is we define types that serve as pointers to the functions we are trying
 * to override. The we define global variables of such types, so we have global pointers
 * that point to the functions in the Windows API.
 * Then we define there global variables as the real meaning of XInput methods, so we
 * never use the ones defined in xinput.h
 */
// We define a macro that generates the methods definition
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
// We generate the types for the pointers of the methods
typedef X_INPUT_GET_STATE(x_input_get_state);
typedef X_INPUT_SET_STATE(x_input_set_state);
// We generate stub functions for those methods
X_INPUT_GET_STATE(XInputGetStateStub) { return(0); }
X_INPUT_SET_STATE(XInputSetStateStub) { return(0); }
// We generate global pointers to the functions and at first we assign them
// to the harmless stubs. This way the game won't crash if the dll is not loaded.
global_variable x_input_get_state *DynamicXInputGetState = XInputGetStateStub;
global_variable x_input_set_state *DynamicXInputSetState = XInputSetStateStub;
#define XInputGetState DynamicXInputGetState
#define XInputSetState DynamicXInputSetState

/**
 * Tries to load the XInput libraries
 */
internal void
Win32LoadXInput()
{
  // We use xinput1_3.dll because it is more widespread installed
  // in Windows machines than xinput1_3.dll
  HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll");
  if(XInputLibrary)
  {
    XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
    XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
  }
}

internal win32_window_dimension
Win32GetWindowDimension(HWND windowHandle)
{
  win32_window_dimension result;

  RECT clientRect;
  GetClientRect(windowHandle, &clientRect);
  result.width = clientRect.right - clientRect.left;
  result.height = clientRect.bottom - clientRect.top;

  return(result);
}

//TODO:(Cristián): This is a global for now
global_variable bool gRunning;
global_variable win32_offscreen_buffer gBackBuffer;

internal void
Win32RenderWeirdGradient(win32_offscreen_buffer buffer, int blueOffset, int greenOffset)
{
  // TODO(Cristián): Let's see what the optimizer does if it is passed by reference
  // instead of a pointer.
  int bitmapWidth = buffer.width;
  int bitmapHeight = buffer.height;
  int pitch = buffer.pitch;
  void *memory = buffer.memory;

  uint8* row = (uint8 *)memory;
  for(int y = 0;
      y < bitmapHeight;
      y++)
  {
    uint32 *pixel = (uint32 *)row;
    for(int x = 0;
        x < bitmapWidth;
        x++)
    {
      /*
        Pixel in Memory: BB GG RR xx
        Little Endian Architecture
        Pixel in Register: 0xXXRRGGBB
       */
      uint8 blue = (x + blueOffset);
      uint8 green = (y + greenOffset);
      uint8 red = 0;
      uint8 pad = 0;

      *pixel++ = blue | (green << 8);
    }
    row += pitch;
  }
}

/**
 * (Re)Creates a Device Independent Bitmap to match the current window size
 * @param buffer  A pointer to the win32_offscreen_buffer. It is important that it is
 *                a pointer because it will modify the buffer and the changes must endure.
 *                (This makes it difficult to the compiler to inline, but is les error-prone).
 */
internal void
Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height)
{
  //TODO(Cristián): Bulletproof this
  // Maybe don't free first, free after, then free first if that fails

  // TODO(Cristián): Free our DIBSection
  // We check whether we have created a bitmap
  if(buffer->memory)
  {
    VirtualFree(buffer->memory, 0, MEM_RELEASE);
  }

  // We update the buffer width
  buffer->width = width;
  buffer->height = height;
  int bytesPerPixel = 4;

  // We set the BITMAPINFO data
  buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
  buffer->info.bmiHeader.biWidth = width;
  buffer->info.bmiHeader.biHeight = -height; // Negative height means top down bitmap (BITMAPINFO MSDN)
  buffer->info.bmiHeader.biPlanes = 1;
  buffer->info.bmiHeader.biBitCount = 32;
  buffer->info.bmiHeader.biCompression = BI_RGB;

  // NOTE(Cristián): There was a confusion on whether to use StretchDIBits or
  // BitBlt for passing the bitmap from our program to windows. The second method
  // is actually a transfer from DC to DC. With the first we can pass a pointer to
  // the bit map in memory with no reference to a DC. This means we can allocate
  // and maintain the memory ourselves.

  // We allocate the memory necesary for the buffer
  int bitmapMemorySize =  bytesPerPixel *
                          buffer->width *
                          buffer->height;
  buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  buffer->pitch = buffer->width * bytesPerPixel;

  //TODO:(Cristián): Probably clear screen to black
}

internal void
Win32TransferBufferToWindows(HDC deviceContext,
                             win32_offscreen_buffer buffer,
                             int windowWidth, int windowHeight)
{
  // TODO(Cristián): aspect ratio correction
  // TODO(Cristián): Play with stretch mode
  // NOTE(Cristián): We are avoiding dirty (partial) rectangles for now
  StretchDIBits(
    deviceContext,
    /*
    x, y, destWidth, destHeight,
    x, y, originWidth, originHeight,
    */
    0, 0, windowWidth, windowHeight,
    0, 0, buffer.width, buffer.height,
    buffer.memory,
    &buffer.info,
    DIB_RGB_COLORS, SRCCOPY);
}

/**
 * The callback to be received from the Win32 Window call
 */
LRESULT CALLBACK
Win32MainWindowCallback(HWND windowHandle,
                        UINT message,
                        WPARAM wParam,
                        LPARAM lParam
)
{
  LRESULT result = 0;
  switch(message)
  {
    case WM_SIZE:
      {
      } break;
    case WM_ACTIVATEAPP:
      {
        OutputDebugStringA("WM_ACTIVATEAPP\n");
      } break;
    case WM_CLOSE:
      {
        //TODO:(Cristián): Handle this with a message to the user
        gRunning = false;
      } break;
    case WM_DESTROY:
      {
        //TODO:(Cristián): Handle this with an error - Recreate window?
        gRunning = false;
      } break;
    case WM_PAINT:
      {
        // We get the windows paint device context (and its dimensions)
        PAINTSTRUCT paint;
        HDC deviceContext = BeginPaint(windowHandle, &paint);
        /*
          // We do not care for the dirty rectangle because this is a game
          // that renders the whole screen each frame. Otherwise, we could
          // use the paint parameters to just update the part of the window
          // that needs to be repainted.
          int x = paint.rcPaint.left;
          int y = paint.rcPaint.top;
          int width = paint.rcPaint.right - paint.rcPaint.left;
          int height = paint.rcPaint.bottom - paint.rcPaint.top;
        */
        win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
        Win32TransferBufferToWindows(deviceContext,
                                     gBackBuffer,
                                     dimension.width, dimension.height);
        EndPaint(windowHandle, &paint);
      } break;
    default:
      {
        //OutputDebugStringA("DEFAULT CASE\n");
        // In this case, we let windows handle the default result for this message
        result = DefWindowProc(windowHandle, message, wParam, lParam);
      } break;
  }
  return(result);
}

/**
 * Main entrance for the program from the C-Runtime Library
 */
int CALLBACK
WinMain(HINSTANCE hInstance,
        HINSTANCE prevHInstance,
        LPSTR commandLine,
        int showCode)
{
  // We initialize the XInput functions pointers
  Win32LoadXInput();

  // In C++, 0 Initialization of a struct is as follows
  WNDCLASS windowClass = {};
  windowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
  windowClass.lpfnWndProc = Win32MainWindowCallback;
  windowClass.hInstance = hInstance;
  //windowClass.hIcon;
  windowClass.lpszClassName = "HandmadeHopeWindowClass";

  // We setup the backbuffer
  Win32ResizeDIBSection(&gBackBuffer, 1280, 740);

  if(RegisterClassA(&windowClass))
  {
    HWND windowHandle = CreateWindowExA(
      0,
      windowClass.lpszClassName,
      "Handmade Hope",
      WS_OVERLAPPEDWINDOW|WS_VISIBLE,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      0,
      0,
      hInstance,
      0
    );
    if(windowHandle)
    {
      // NOTE(Cristián): Sice we specified CS_OWNDC for our WNDCLASS, we can just
      // get one device context and use it forever, because we don't need to return it.
      HDC deviceContext = GetDC(windowHandle);
      // ** MESSAGE LOOP **
      // We retrieve the messages from windows via the message queue
      int blueOffset = 0;
      int greenOffset = 0;
      gRunning = true;
      while(gRunning)
      {
        /**
         * We create the MSG inside the loop instead of outside to use correct
         * 'lexical' scoping. This means we protect outselves from referencing this pointer
         * outside this scope, which would be potentially a bug because this MSG only makes
         * sense in this loop.
         *
         * We can do this because the MSG is a simple structure, so the compiler will (probably)
         * optimize it in such a way it should not make much difference whether it is defined
         * inside the scope or outside it.
         *
         * It is importante to note that is we use C++ constructors/destructors IT DOES MAKE
         * A DIFFERENCE whether we put it in a loop because the spec requires it to call those
         * callbacks everytime the structure (or class) is created/destroyed. This can mean a lot
         * of code of overhead if defined inside the loop. But this is not such case.
         */
        MSG message;
        // Windows may call the callback even outside the loop
        while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
        {
          if(message.message == WM_QUIT)
          {
            gRunning = false;
          }

          TranslateMessage(&message);
          DispatchMessageA(&message);
        }

        // Xinput is a polling based API
        // TODO(Cristián): Should we pull this more frequently?
        for(DWORD controllerIndex = 0;
            controllerIndex < XUSER_MAX_COUNT;
            controllerIndex++)
        {
          DWORD result;
          XINPUT_STATE controllerState;

          if(XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) // Amazing success key code name
          {
            // TODO(Cristián): See if controllerState.swPacketNumber incrementes too rapidly
            // We import the whole gamepad state
            XINPUT_GAMEPAD *pad = &controllerState.Gamepad;
            bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
            bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool leftThumb = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
            bool rightThumb = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
            bool leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool aButton = (pad->wButtons & XINPUT_GAMEPAD_A);
            bool bButton = (pad->wButtons & XINPUT_GAMEPAD_B);
            bool yButton = (pad->wButtons & XINPUT_GAMEPAD_Y);
            bool xButton = (pad->wButtons & XINPUT_GAMEPAD_X);

            // We inport the sticks
            int16 leftThumbX = pad->sThumbLX;
            int16 leftThumbY = pad->sThumbLY;
            int16 rightThumbX = pad->sThumbRX;
            int16 rightThumbY = pad->sThumbRY;

            // We import the triggers
            int16 leftTrigger = pad->bLeftTrigger;
            int16 rightTrigger = pad->bRightTrigger;

            // We assign acceleration
            blueOffset += leftThumbX / 4096;
            greenOffset -= leftThumbY / 4096; // We invert the Y because the screen is also inverted

            XINPUT_VIBRATION xInputVibration = {};
            xInputVibration.wLeftMotorSpeed = aButton ? 65535 : 0;
            xInputVibration.wRightMotorSpeed = bButton ? 65535 : 0;

            XInputSetState(controllerIndex, &xInputVibration);
          }
          else
          {
            // NOTE(Cristián): The controller is not available
          }
 
        }

        Win32RenderWeirdGradient(gBackBuffer, blueOffset, greenOffset);

        win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
        Win32TransferBufferToWindows(deviceContext,
                                     gBackBuffer,
                                     dimension.width, dimension.height);
        //blueOffset++;
        //greenOffset += 2;
      }
    }
    else
    {
      // TODO(Cristián): Loggin'
    }
  }
  else
  {
    // TODO(Cristián): Loggin'
  }

  return(0);
}
