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

struct Win32OffScreenBuffer
{
  BITMAPINFO info;
  void *memory;
  int width;
  int height;
  int pitch;
};

struct Win32WindowDimension {
  int width;
  int height;
};

internal Win32WindowDimension
Win32GetWindowDimension(HWND windowHandle)
{
  Win32WindowDimension result;

  RECT clientRect;
  GetClientRect(windowHandle, &clientRect);
  result.width = clientRect.right - clientRect.left;
  result.height = clientRect.bottom - clientRect.top;

  return(result);
}

//TODO:(Cristián): This is a global for now
global_variable bool gRunning;
global_variable Win32OffScreenBuffer gBackBuffer;

internal void
Win32RenderWeirdGradient(Win32OffScreenBuffer buffer, int blueOffset, int greenOffset)
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
 * @param buffer  A pointer to the Win32OffScreenBuffer. It is important that it is
 *                a pointer because it will modify the buffer and the changes must endure.
 *                (This makes it difficult to the compiler to inline, but is les error-prone).
 */
internal void
Win32ResizeDIBSection(Win32OffScreenBuffer *buffer, int width, int height)
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
                             Win32OffScreenBuffer buffer,
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
        Win32WindowDimension dimension = Win32GetWindowDimension(windowHandle);
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

        Win32RenderWeirdGradient(gBackBuffer, blueOffset, greenOffset);

        Win32WindowDimension dimension = Win32GetWindowDimension(windowHandle);
        Win32TransferBufferToWindows(deviceContext,
                                     gBackBuffer,
                                     dimension.width, dimension.height);
        blueOffset++;
        greenOffset += 2;
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
