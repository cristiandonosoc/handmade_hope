/*  =====================================================================
    $File: win32_handmade.cpp
    $Creation Date: 2014-12-27
    $Last Modified: 2014-12-28 17:14
    $Revision: $
    $Creator: Cristi�n Donoso $
    $Notice: (c) Copyright 2014 Cristi�n Donoso $
    ===================================================================== */

#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>

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

typedef int32 bool32;

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

/******** GLOBAL VARIABLES **********/

global_variable bool32 gRunning;
global_variable win32_offscreen_buffer gBackBuffer;
global_variable LPDIRECTSOUNDBUFFER gSecondaryBuffer;

// TODO(Cristi�n): Remove this as a global variable
global_variable int32 gToneHz = 440;

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
X_INPUT_GET_STATE(XInputGetStateStub) { return(ERROR_DEVICE_NOT_CONNECTED); }
X_INPUT_SET_STATE(XInputSetStateStub) { return(ERROR_DEVICE_NOT_CONNECTED); }
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
  // We try to load 1.4 first (Windows 8), and then we try 1.3
  HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
  if(!XInputLibrary) { LoadLibraryA("xinput1_3.dll"); }
  if(!XInputLibrary) { return; } // TODO(Cristi�n): Diagnostics

  // Here we make a 'late-binding', where we go look at the address of library
  // loaded dynamically
  XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
  if(!XInputGetState) { XInputGetState = XInputGetStateStub; }
  XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
  if(!XInputSetState) { XInputSetState = XInputSetStateStub; }

  // TODO(Cristi�n): Diagnostics
}

/**
 * We create our DirectSound API handler pointer.
 */
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

/**
 * We initialize the Direct Sound buffers.
 * The first (or primary) is merely a handle and initialization of the memory in the sound card.
 * The second is the actual buffer in memory the system will be writing into to create sound.
 */
internal void
Win32InitDirectSound(HWND windowHandle,
                     int32 samplesPerSecond,
                     int32 bytesPerSample,
                     int32 nChannels,
                     int32 bufferLength)
{
  // NOTE(Cristi�n): Load the library
  HMODULE DirectSoundLibrary = LoadLibraryA("dsound.dll");
  if(!DirectSoundLibrary) { return; } //TODO(Cristi�n): Diagnostics

  // Get a DirectSound object
  direct_sound_create *directSoundCreate = (direct_sound_create *)
    GetProcAddress(DirectSoundLibrary, "DirectSoundCreate");

  // TODO(Cristi�n): Check that this works on XP - DirectSound 8 or 7
  /**
   * What happens here is that Windows gives us a LPDIRECTSOUND object that is defined
   * in the dsound.h file. Afterwards we don't need to call GetProcAddress to get the calls
   * to the DirectSound object methods because we make use of its vTable.
   *
   * Basically, this object has it's methods defined as virtual methods. This means that
   * it has a method to a global table associated with the class (or struct) that has all the
   * pointers to its virtual methods. This way we can get the pointers to the methods by referencing
   * the vTable.
   *
   * This means that at RUNTIME we can get the pointers to the functions of the object without
   * having to actually have its definitions compiled. This is the standard way Microsoft implements
   * its COM (Component Object Model) layer in order to send functions outside the 'dll boundary'
   */
  LPDIRECTSOUND directSound;
  if(!directSoundCreate || !SUCCEEDED(directSoundCreate(0, &directSound, 0)))
  {
    return; // TODO(Cristi�n): Diagnostics
  }

  if(!SUCCEEDED(directSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY)))
  {
    return; // TODO(Cristi�n): Diagnostics
  }

  int32 bufferSize = bufferLength * nChannels * samplesPerSecond * bytesPerSample;

  // We set the format for the buffers
  WAVEFORMATEX waveFormat = {};
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nChannels = nChannels;
  waveFormat.wBitsPerSample = bytesPerSample << 3; // *8
  // Size (in bytes) of a sample block
  waveFormat.nBlockAlign = (waveFormat.nChannels * bytesPerSample);
  waveFormat.nSamplesPerSec = samplesPerSecond;
  waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;
  waveFormat.cbSize = 0;

  // "Create" a primary buffer
  // This buffer is a handle to the sound card which Windows will write according to
  // our data in the secondary buffer
  DSBUFFERDESC bufferDescription = {};
  bufferDescription.dwSize = sizeof(bufferDescription);
  // TODO(Cristi�n): See if we need DSBCAPS_GLOBALFOCUS
  bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

  LPDIRECTSOUNDBUFFER primaryBuffer;
  if(!SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
  {
    return; // TODO(Cristi�n): Diagnostics
  }
  if(!SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) { return; } // TODO(Cristi�n): Diagnostics

  // "Create" a secondary buffer
  DSBUFFERDESC secBufferDescription = {};
  secBufferDescription.dwSize = sizeof(secBufferDescription);
  secBufferDescription.dwFlags = 0;
  secBufferDescription.dwBufferBytes = bufferSize;
  secBufferDescription.lpwfxFormat = &waveFormat;
  // The gSecondaryBuffer pointer is defined globally
  if(!SUCCEEDED(directSound->CreateSoundBuffer(&secBufferDescription, &gSecondaryBuffer, 0)))
  {
    return; // TODO(Cristi�n): Diagnostics
  }

  // NOTE(Cristi�n): Start it playing
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

/**
 * Writes a 'weird' gradient into a memory buffer
 * @param *buffer A pointer to the buffer info struct. We can pass it by reference also
 *                because the struct does not contain the buffer itself, but a pointer, so
 *                this method does not ACTUALLY modify the struct, but the memory it points to.
 *                We pass a pointer in order to not copy a fairly big structure into memory via
 *                the stack.
 */
internal void
Win32RenderWeirdGradient(win32_offscreen_buffer *buffer, int blueOffset, int greenOffset)
{
  // TODO(Cristi�n): Let's see what the optimizer does if it is passed by reference
  // instead of a pointer.
  int bitmapWidth = buffer->width;
  int bitmapHeight = buffer->height;
  int pitch = buffer->pitch;
  void *memory = buffer->memory;

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
 * @param *buffer A pointer to the win32_offscreen_buffer. It is important that it is
 *                a pointer because it will modify the buffer and the changes must endure.
 *                (This makes it difficult to the compiler to inline, but is les error-prone).
 */
internal void
Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height)
{
  //TODO(Cristi�n): Bulletproof this
  // Maybe don't free first, free after, then free first if that fails

  // TODO(Cristi�n): Free our DIBSection
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

  // NOTE(Cristi�n): There was a confusion on whether to use StretchDIBits or
  // BitBlt for passing the bitmap from our program to windows. The second method
  // is actually a transfer from DC to DC. With the first we can pass a pointer to
  // the bit map in memory with no reference to a DC. This means we can allocate
  // and maintain the memory ourselves.

  // We allocate the memory necesary for the buffer
  int bitmapMemorySize =  bytesPerPixel *
                          buffer->width *
                          buffer->height;
  buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
  buffer->pitch = buffer->width * bytesPerPixel;

  //TODO:(Cristi�n): Probably clear screen to black
}

/**
 * Transfer our buffer into windows via StretchDIBits syscall.
 * @param *buffer A pointer to the buffer info struct. We use a pointer because
 *                it is a fairly big struct so that if the compiler doesn't inline
 *                the code, it would require a 'big' memory copy in the stack. So
 *                we send the pointer even though we do not modify the origin struct.
 */
internal void
Win32TransferBufferToWindows(HDC deviceContext,
                             win32_offscreen_buffer *buffer,
                             int windowWidth, int windowHeight)
{
  // TODO(Cristi�n): aspect ratio correction
  // TODO(Cristi�n): Play with stretch mode
  // NOTE(Cristi�n): We are avoiding dirty (partial) rectangles for now
  StretchDIBits(
    deviceContext,
    /*
    x, y, destWidth, destHeight,
    x, y, originWidth, originHeight,
    */
    0, 0, windowWidth, windowHeight,
    0, 0, buffer->width, buffer->height,
    buffer->memory,
    &buffer->info,
    DIB_RGB_COLORS, SRCCOPY);
}

/**
 * The callback to be received from the Win32 Window call
 */
LRESULT CALLBACK
Win32MainWindowCallback(HWND windowHandle,
                        UINT message,
                        WPARAM wParam,
                        LPARAM lParam)
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
        //TODO:(Cristi�n): Handle this with a message to the user
        gRunning = false;
      } break;
    case WM_DESTROY:
      {
        //TODO:(Cristi�n): Handle this with an error - Recreate window?
        gRunning = false;
      } break;
    // We use the switch to grab all the keys messages into one block
    // (they all cascade into WM_KEYUP)
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
      {
        uint32 vKeyCode = wParam;
        // Here we force the booleans to be 0 or 1 because
        // we want the case where the both are active to be ignored,
        // Without the forcing, both active can actually be different
        // and we would enter anyway
        bool keyWasDown = (lParam & (1 << 30) != 0);
        bool keyIsDown = ((lParam & (1 << 31) == 0));
        // We ignore the key that keeps pressed
        if(keyWasDown != keyIsDown)
        {
          if(vKeyCode == 'W')
          {
          }
          else if(vKeyCode == 'A')
          {
          }
          else if(vKeyCode == 'S')
          {
          }
          else if(vKeyCode == 'D')
          {
          }
          else if(vKeyCode == VK_UP)
          {
          }
          else if(vKeyCode == VK_DOWN)
          {
          }
          else if(vKeyCode == VK_LEFT)
          {
            gToneHz -= 2;
          }
          else if(vKeyCode == VK_RIGHT)
          {
            gToneHz += 2;
          }
          else if(vKeyCode == VK_ESCAPE)
          {
          }
          else if(vKeyCode == VK_SPACE)
          {
          }
          else // All other keys are treated by default
          {
            result = DefWindowProc(windowHandle, message, wParam, lParam);
          }
        }
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
                                     &gBackBuffer,
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
  WNDCLASSA windowClass = {};
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
      // NOTE(Cristi�n): Sice we specified CS_OWNDC for our WNDCLASS, we can just
      // get one device context and use it forever, because we don't need to return it.
      HDC deviceContext = GetDC(windowHandle);

      int32 samplesPerSecond = 48000; // 48 kHz
      int32 nChannels = 2;  // Stereo Sound
      int32 bytesPerSample = 2;
      int32 bytesPerBlock = nChannels * bytesPerSample;
      int32 bufferLength = 1;
      int32 bufferSize = bufferLength * samplesPerSecond * bytesPerBlock;

      // An index that counts how many samples we've outputed. We can use the module operator
      // to make a running index of our buffer
      uint32 runningBlockIndex = 0;
      Win32InitDirectSound(windowHandle, samplesPerSecond, bytesPerSample, nChannels, bufferLength);

      // NOTE(Cristi�n): Test Code
      gSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
      int32 toneVolume = 5000;
      int32 squareWavePeriod = samplesPerSecond / gToneHz;
      int32 halfSquareWavePeriod = squareWavePeriod / 2;

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
        // TODO(Cristi�n): Should we pull this more frequently?
        for(DWORD controllerIndex = 0;
            controllerIndex < XUSER_MAX_COUNT;
            controllerIndex++)
        {
          DWORD result;
          XINPUT_STATE controllerState;

          if(XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) // Amazing success key code name
          {
            // TODO(Cristi�n): See if controllerState.swPacketNumber incrementes too rapidly
            // We import the whole gamepad state
            XINPUT_GAMEPAD *pad = &controllerState.Gamepad;
            bool32 up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool32 down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool32 left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool32 right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool32 start = (pad->wButtons & XINPUT_GAMEPAD_START);
            bool32 back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool32 leftThumb = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
            bool32 rightThumb = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
            bool32 leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool32 rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool32 aButton = (pad->wButtons & XINPUT_GAMEPAD_A);
            bool32 bButton = (pad->wButtons & XINPUT_GAMEPAD_B);
            bool32 yButton = (pad->wButtons & XINPUT_GAMEPAD_Y);
            bool32 xButton = (pad->wButtons & XINPUT_GAMEPAD_X);

            // We inport the sticks
            int16 leftThumbX = pad->sThumbLX;
            int16 leftThumbY = pad->sThumbLY;
            int16 rightThumbX = pad->sThumbRX;
            int16 rightThumbY = pad->sThumbRY;

            // We import the triggers
            int16 leftTrigger = pad->bLeftTrigger;
            int16 rightTrigger = pad->bRightTrigger;

            // We assign acceleration
            blueOffset += leftThumbX >> 12;
            greenOffset -= leftThumbY >> 12; // We invert the Y because the screen is also inverted

            XINPUT_VIBRATION xInputVibration = {};
            xInputVibration.wLeftMotorSpeed = aButton ? 65535 : 0;
            xInputVibration.wRightMotorSpeed = bButton ? 65535 : 0;

            XInputSetState(controllerIndex, &xInputVibration);
          }
          else
          {
            // NOTE(Cristi�n): The controller is not available
          }

        }

        Win32RenderWeirdGradient(&gBackBuffer, blueOffset, greenOffset);
        win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
        Win32TransferBufferToWindows(deviceContext,
                                     &gBackBuffer,
                                     dimension.width, dimension.height);

        // We get the cursor IN BYTES of where the system is playing and writing in the
        // sound buffer
        DWORD playCursor;
        DWORD writeCursor;
        if(!SUCCEEDED(gSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
        {
          // If we can't get the current position, then the buffer died
          continue;
        }

        DWORD bytesToWrite;
        DWORD byteToLock = (runningBlockIndex * bytesPerBlock) % bufferSize;
        if(byteToLock <= playCursor) { bytesToWrite = playCursor - byteToLock; }
        else { bytesToWrite = bufferSize - (byteToLock - playCursor); }

        VOID *region1;
        DWORD region1Size;
        VOID *region2;
        DWORD region2Size;
        if(!SUCCEEDED(gSecondaryBuffer->Lock(byteToLock, bytesToWrite,
                               &region1, &region1Size,
                               &region2, &region2Size,
                               0)))
        {
          // If we can't lock the buffer, then the buffer died
          continue;
        }

        /**
         * We write into the buffer by writing and advancing the output pointer
         * We make two writes because we created 2 channels, which makes the buffer to look like this:
         * [int16 int16] [int16 int16] ...
         * [LEFT  RIGHT] [LEFT  RIGHT] ...
         * [  SAMPLE   ] [  SAMPLE   ] ...
         *
         */

        // TODO(Cristi�n): Remove this hz update
        squareWavePeriod = samplesPerSecond / gToneHz;
        halfSquareWavePeriod = squareWavePeriod / 2;

        // We cast the region pointer into int16 pointers (it is a DWORD) so we can
        // write into each channel of the sound buffer
        int16 *sampleOut = (int16 *)region1;
        int32 region1SampleCount = region1Size / bytesPerBlock;
        // TODO(Cristi�n): Assert that region sizes are valid (sample multiple)
        for(int32 sampleIndex = 0;
            sampleIndex < region1SampleCount;
            sampleIndex++)
        {
          // We check into which part of the square Cycle we are
          int16 sampleValue = ((runningBlockIndex / halfSquareWavePeriod) % 2) ? toneVolume : -toneVolume;

          *sampleOut++ = sampleValue;
          *sampleOut++ = sampleValue;
          runningBlockIndex++;
        }
        sampleOut = (int16 *)region2;
        int32 region2SampleCount = region2Size / bytesPerBlock;
        // TODO(Cristi�n): Assert that region sizes are valid (sample multiple)
        for(int32 sampleIndex = 0;
            sampleIndex < region2SampleCount;
            sampleIndex++)
        {
          // We check into which part of the square Cycle we are
          int16 sampleValue = ((runningBlockIndex / halfSquareWavePeriod) % 2) ? toneVolume : -toneVolume;

          *sampleOut++ = sampleValue;
          *sampleOut++ = sampleValue;
          runningBlockIndex++;
        }

        gSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);

      }
    }
    else
    {
      // TODO(Cristi�n): Loggin'
    }
  }
  else
  {
    // TODO(Cristi�n): Loggin'
  }

  return(0);
}
