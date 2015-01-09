/*  =====================================================================
    $File: win32_handmade.cpp
    $Creation Date: 2014-12-27
    $Last Modified: 2014-12-28 17:14
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2014 Cristián Donoso $
    ===================================================================== */

#include <windows.h>
#include <dsound.h>

// Defines our own common types
#include "common_types.h"
#include "x_input_wrapper.cpp"
#include "graphics_wrapper.cpp"

/******** GLOBAL VARIABLES **********/

global_variable LPDIRECTSOUNDBUFFER gSecondaryBuffer;

// TODO(Cristián): Remove this as a global variable
global_variable int32 gToneHz = 440;

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
  // NOTE(Cristián): Load the library
  HMODULE DirectSoundLibrary = LoadLibraryA("dsound.dll");
  if(!DirectSoundLibrary) { return; } //TODO(Cristián): Diagnostics

  // Get a DirectSound object
  direct_sound_create *directSoundCreate = (direct_sound_create *)
    GetProcAddress(DirectSoundLibrary, "DirectSoundCreate");

  // TODO(Cristián): Check that this works on XP - DirectSound 8 or 7
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
    return; // TODO(Cristián): Diagnostics
  }

  if(!SUCCEEDED(directSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY)))
  {
    return; // TODO(Cristián): Diagnostics
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
  // TODO(Cristián): See if we need DSBCAPS_GLOBALFOCUS
  bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

  LPDIRECTSOUNDBUFFER primaryBuffer;
  if(!SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
  {
    return; // TODO(Cristián): Diagnostics
  }
  if(!SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) { return; } // TODO(Cristián): Diagnostics

  // "Create" a secondary buffer
  DSBUFFERDESC secBufferDescription = {};
  secBufferDescription.dwSize = sizeof(secBufferDescription);
  secBufferDescription.dwFlags = 0;
  secBufferDescription.dwBufferBytes = bufferSize;
  secBufferDescription.lpwfxFormat = &waveFormat;
  // The gSecondaryBuffer pointer is defined globally
  if(!SUCCEEDED(directSound->CreateSoundBuffer(&secBufferDescription, &gSecondaryBuffer, 0)))
  {
    return; // TODO(Cristián): Diagnostics
  }

  // NOTE(Cristián): Start it playing
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
      // NOTE(Cristián): Sice we specified CS_OWNDC for our WNDCLASS, we can just
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

      // NOTE(Cristián): Test Code
      gSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
      int32 toneVolume = 7000;
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

        // TODO(Cristián): Remove this hz update
        squareWavePeriod = samplesPerSecond / gToneHz;
        halfSquareWavePeriod = squareWavePeriod / 2;

        // We cast the region pointer into int16 pointers (it is a DWORD) so we can
        // write into each channel of the sound buffer
        int16 *sampleOut = (int16 *)region1;
        int32 region1SampleCount = region1Size / bytesPerBlock;
        // TODO(Cristián): Assert that region sizes are valid (sample multiple)
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
        // TODO(Cristián): Assert that region sizes are valid (sample multiple)
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
      // TODO(Cristián): Loggin'
    }
  }
  else
  {
    // TODO(Cristián): Loggin'
  }

  return(0);
}
