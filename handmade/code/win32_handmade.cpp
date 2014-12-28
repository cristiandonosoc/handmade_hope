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

//TODO:(Cristián): This is a global for now
global_variable bool RUNNING;
global_variable BITMAPINFO bitmapInfo;
global_variable void *bitmapMemory;
global_variable int bitmapWidth;
global_variable int bitmapHeight;
global_variable int bytesPerPixel = 4;

internal void
Win32RenderWeirdGradient(int xOffset, int yOffset)
{
  int pitch = bitmapWidth * bytesPerPixel;
  uint8* row = (uint8 *)bitmapMemory;
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
      uint8 blue = (x + xOffset);
      uint8 green = (y + yOffset);
      uint8 red = 0;
      uint8 pad = 0;

      *pixel++ = (green << 8) | blue;
    }
    row += pitch;
  }
}

/**
 * (Re)Creates a Device Independent Bitmap to match the current window size
 */
internal void
ResizeDIBSection(int width, int height)
{
  //TODO(Cristián): Bulletproof this
  // Maybe don't free first, free after, then free first if that fails

  // TODO(Cristián): Free our DIBSection
  // We check whether we have created a bitmap
  if(bitmapMemory)
  {
    VirtualFree(bitmapMemory, 0, MEM_RELEASE);
  }

  bitmapWidth = width;
  bitmapHeight = height;

  // We set the BITMAPINFO data
  bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
  bitmapInfo.bmiHeader.biWidth = bitmapWidth;
  bitmapInfo.bmiHeader.biHeight = -bitmapHeight; // Negative height means top down bitmap (BITMAPINFO MSDN)
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;

  // NOTE(Cristián): There was a confusion on whether to use StretchDIBits or
  // BitBlt for passing the bitmap from our program to windows. The second method
  // is actually a transfer from DC to DC. With the first we can pass a pointer to
  // the bit map in memory with no reference to a DC. This means we can allocate
  // and maintain the memory ourselves.

  // We allocate the memory necesary for the buffer
  int bitmapMemorySize = bytesPerPixel * bitmapWidth * bitmapHeight;
  bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  //TODO:(Cristián): Probably clear screen to black
}

internal void
Win32UpdateWindow(HDC deviceContext, RECT *clientRect, int x, int y, int width, int height)
{
  // NOTE(Cristián): We are avoiding dirty (partial) rectangles for now
  int windowWidth = clientRect->right - clientRect->left;
  int windowHeight = clientRect->bottom - clientRect->top;

  StretchDIBits(
    deviceContext,
    /*
    x, y, width, height,
    x, y, width, height,
    */
    0, 0, bitmapWidth, bitmapHeight,
    0, 0, windowWidth, windowHeight,
    bitmapMemory,
    &bitmapInfo,
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
        RECT clientRect;
        GetClientRect(windowHandle, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        ResizeDIBSection(width, height);
      } break;
    case WM_ACTIVATEAPP:
      {
        OutputDebugStringA("WM_ACTIVATEAPP\n");
      } break;
    case WM_CLOSE:
      {
        //TODO:(Cristián): Handle this with a message to the user
        RUNNING = false;
      } break;
    case WM_DESTROY:
      {
        //TODO:(Cristián): Handle this with an error - Recreate window?
        RUNNING = false;
      } break;
    case WM_PAINT:
      {
        PAINTSTRUCT paint;
        // We paint a black n' white epillepsy!
        HDC deviceContext = BeginPaint(windowHandle, &paint);
        int x = paint.rcPaint.left;
        int y = paint.rcPaint.top;
        int width = paint.rcPaint.right - paint.rcPaint.left;
        int height = paint.rcPaint.bottom - paint.rcPaint.top;

        RECT clientRect;
        GetClientRect(windowHandle, &clientRect);

        Win32UpdateWindow(deviceContext, &clientRect, x, y, width, height);
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
    HINSTANCE prevhInstance,
    LPSTR commandLine,
    int showCode)
{
  // In C++, 0 Initialization of a struct is as follows
  WNDCLASS windowClass = {};
  /* The WNDCLASS struct definition
     UINT      style;
     WNDPROC   lpfnWndProc;
     int       cbClsExtra;
     int       cbWndExtra;
     HINSTANCE hInstance;
     HICON     hIcon;
     HCURSOR   hCursor;
     HBRUSH    hbrBackground;
     LPCTSTR   lpszMenuName;
     LPCTSTR   lpszClassName;
     */
  //TODO(Cristián): Check if CS_HREDRAW and CS_VREDRAW still matter
  windowClass.style = CS_OWNDC;
  windowClass.lpfnWndProc = Win32MainWindowCallback;
  windowClass.hInstance = hInstance;
  //windowClass.hIcon;
  windowClass.lpszClassName = "HandmadeHopeWindowClass";

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
      // ** MESSAGE LOOP **
      // We retrieve the messages from windows via the message queue
      RUNNING = true;
      int xOffset = 0;
      int yOffset = 0;
      while(RUNNING)
      {
        // Windows may call the callback even outside the loop
        MSG message;
        while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
        {
          if(message.message == WM_QUIT)
          {
            RUNNING = false;
          }

          TranslateMessage(&message);
          DispatchMessageA(&message);
        }

        HDC deviceContext = GetDC(windowHandle);
        RECT clientRect;
        GetClientRect(windowHandle, &clientRect);
        int windowWidth = clientRect.right - clientRect.left;
        int windowHeight = clientRect.bottom - clientRect.top;

        Win32RenderWeirdGradient(xOffset, yOffset);
        Win32UpdateWindow(deviceContext, &clientRect, 0, 0, windowWidth, windowHeight);
        ReleaseDC(windowHandle, deviceContext);
        xOffset++;
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
