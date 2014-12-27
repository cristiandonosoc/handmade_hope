#include <windows.h>

/**
 * The callback to be received from the Win32 Window call
 */
LRESULT CALLBACK
MainWindowCallback(HWND windowHandle,
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
        OutputDebugStringA("WM_SIZE\n");
      } break;
    case WM_DESTROY:
      {
        OutputDebugStringA("WM_DESTROY\n");
      } break;
    case WM_CLOSE:
      {
        OutputDebugStringA("WM_CLOSE\n");
      } break;
    case WM_ACTIVATEAPP:
      {
        OutputDebugStringA("WM_ACTIVATEAPP\n");
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
        static DWORD operation = WHITENESS;
        PatBlt(deviceContext, x, y, width, height, operation);
        if(operation == WHITENESS) { operation = BLACKNESS; }
        else { operation = WHITENESS; }
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
  windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
  windowClass.lpfnWndProc = MainWindowCallback;
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
      // We retrieve the messages from windows via the message queue
      MSG message;
      for(;;)
      {
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
