/*  =====================================================================
    $File: win32_handmade.cpp
    $Creation Date: 2014-12-27
    $Last Modified: 2014-12-28 16:13
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2014 Cristián Donoso $
    ===================================================================== */

#include <windows.h>

// We rename static to some aliases to make more transparent the use of each
#define internal static           // Makes functions scoped to the 'translation unit'
#define global_variable static    // A variable available to all (or many) scopes
#define local_persist static      // A scoped-variable that survives such scope

//TODO:(Cristián): This is a global for now 
global_variable bool RUNNING;
global_variable BITMAPINFO bitmapInfo;
global_variable void *bitmapMemory;
global_variable HBITMAP bitmapHandle;
global_variable HDC bitmapDeviceContext;

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
  if(bitmapHandle)
  {
    DeleteObject(bitmapHandle);
  }

  if(!bitmapDeviceContext)
  {
    //TODO(Cristián): Should we recreate these under certain special circumstances
    bitmapDeviceContext = CreateCompatibleDC(0);
  }

  // We set the BITMAPINFO data
  bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
  bitmapInfo.bmiHeader.biWidth = width;
  bitmapInfo.bmiHeader.biHeight = height;
  bitmapInfo.bmiHeader.biPlanes = 1;
  bitmapInfo.bmiHeader.biBitCount = 32;
  bitmapInfo.bmiHeader.biCompression = BI_RGB;

  //TODO(Cristián): Apparently it's possible to allocate the memory ourselves and 
  // not asking windows for it.

  bitmapHandle = CreateDIBSection(
    bitmapDeviceContext,
    &bitmapInfo,
    DIB_RGB_COLORS,
    &bitmapMemory,
    0, 0);
}

internal void
Win32UpdateWindow(HDC deviceContext, int x, int y, int width, int height)
{
  StretchDIBits(
    deviceContext,
    x, y, width, height,
    x, y, width, height,
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
        RECT rect;
        GetClientRect(windowHandle, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
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
        Win32UpdateWindow(deviceContext, x, y, width, height);
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
      while(RUNNING)
      {
        // Windows may call the callback even outside the loop
        MSG message;
        BOOL messageReturn = GetMessageA(&message, 0, 0, 0);
        if(messageReturn > 0)
        {
          TranslateMessage(&message);
          DispatchMessageA(&message);
        }
        else
        {
          break;  // -1 means an error message, so we end the loop
        }
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
