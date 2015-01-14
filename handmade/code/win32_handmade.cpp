/*  =====================================================================
    $File: win32_handmade.cpp
    $Creation Date: 2014-12-27
    $Last Modified: 2014-12-28 17:14
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2014 Cristián Donoso $
    ===================================================================== */

#include <windows.h>

// Defines our own common types
#include "win32_common_types.h"
#include "win32_x_input_wrapper.cpp"
#include "win32_graphics_wrapper.cpp"

#include "win32_direct_sound_wrapper.cpp"
// Global Sound Buffer Management
global_variable win32_sound_output gSoundOutput;

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
        //TODO:(Cristián): Handle this with a message to the user
        gRunning = false;
      } break;
    case WM_DESTROY:
      {
        //TODO:(Cristián): Handle this with an error - Recreate window?
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
            gSoundOutput.ModifyBufferToneHz(-2);
          }
          else if(vKeyCode == VK_RIGHT)
          {
            gSoundOutput.ModifyBufferToneHz(2);
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
      // NOTE(Cristián): Sice we specified CS_OWNDC for our WNDCLASS, we can just
      // get one device context and use it forever, because we don't need to return it.
      HDC deviceContext = GetDC(windowHandle);

      gSoundOutput.SetSamplesPerSecond(48000); // 48kHz
      gSoundOutput.latency = gSoundOutput.GetSamplesPerSecond() / 15;   // 15 frames of latency
      gSoundOutput.nChannels = 2;
      gSoundOutput.bytesPerBlock = gSoundOutput.nChannels * sizeof(int16);
      gSoundOutput.bufferSize = gSoundOutput.GetSamplesPerSecond() * gSoundOutput.bytesPerBlock;

      // An index that counts how many samples we've outputed. We can use the module operator
      // to make a running index of our buffer
      uint32 runningBlockIndex = 0;
      Win32InitDirectSound(windowHandle, &gSoundOutput);

      // NOTE(Cristián): Test Code
      Win32FillSoundBuffer(&gSoundOutput, 0, gSoundOutput.bufferSize);
      Win32PlayDirectSound();

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

            x_input_gamepad_state gamepadState = GetGamepadState(&controllerState);

            // We assign acceleration
            blueOffset += gamepadState.leftThumbX >> 12;
            greenOffset -= gamepadState.leftThumbY >> 12; // We invert the Y because the screen is also inverted

            XINPUT_VIBRATION xInputVibration = {};
            xInputVibration.wLeftMotorSpeed = gamepadState.aButton ? 65535 : 0;
            xInputVibration.wRightMotorSpeed = gamepadState.bButton ? 65535 : 0;

            XInputSetState(controllerIndex, &xInputVibration);
          }
          else
          {
            // NOTE(Cristián): The controller is not available
          }

        }

        Win32RenderWeirdGradient(&gBackBuffer, blueOffset, greenOffset);
        win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
        Win32TransferBufferToWindows(deviceContext,
                                     &gBackBuffer,
                                     dimension.width, dimension.height);

        Win32RunDirectSoundSample(&gSoundOutput);

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


