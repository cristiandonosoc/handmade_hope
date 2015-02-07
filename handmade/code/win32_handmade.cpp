/*  =====================================================================
    $File: win32_handmade.cpp
    $Creation Date: 2014-12-27
    $Last Modified: 2014-12-28 17:14
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2014 Cristián Donoso $
    =====================================================================

    TODO(Cristián): THIS IS NOT A FINAL PLATFORM LAYER
                    THINGS THAT ARE STILL MISSING:
    - Saved game locations
    - Getting a handle to our own executable file
    - Asset loading path
    - Threading
    - Raw Input (Support for multiple keyboards)
    - Sleep / Time Begin Period
    - ClipCursor() (for multimonitor support)
    - WM_SETCURSOR (control cursor visibility)
    - WM_ACTIVATEAPP (for when we are not the active application)
    - Query Cancel Autoplay
    - Blit speed improvements (BitBlt)
    - Hardware Acceleration (OpenGL || DirectX || BOTH?)
    - Get Keyboard Layout (for french keyboards, international WASD support)

    ... and many more

    ===================================================================== */

// Defines our own common types
#include "common_types.h"
#include "handmade.cpp"

// Platform specific code
#include <windows.h>
#include "platform_layer/win32/win32_x_input_wrapper.cpp"
#include "platform_layer/win32/win32_graphics_wrapper.cpp"
#include "platform_layer/win32/win32_direct_sound_wrapper.cpp"


// Global Sound Buffer Management
global_variable win32_sound_output gSoundOutput;
global_variable uint64 gPerformanceCounterFrequency;

// Platform Load File
internal game_file
DEBUG_PlatformReadEntireFile(char *fileName)
{
  game_file gameFile = {};
  HANDLE fileHandle = CreateFileA(fileName,
      GENERIC_READ,
      FILE_SHARE_READ,
      0,
      OPEN_EXISTING,
      0,
      0);
  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    // TODO(Cristián): Better error code
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_READ_ENTIRE_FILE_RETURN;
  }

  LARGE_INTEGER fileSize64;
  if(!GetFileSizeEx(fileHandle, &fileSize64))
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_READ_ENTIRE_FILE_CREATE_FILE_CLEANUP;
  }

  uint32 fileSize32 = SafeTruncateUInt64(fileSize64.QuadPart);
  gameFile.content = VirtualAlloc(0,
                                  fileSize32,
                                  MEM_RESERVE|MEM_COMMIT,
                                  PAGE_READWRITE);
  if (!gameFile.content)
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_READ_ENTIRE_FILE_CREATE_FILE_CLEANUP;
  }


  DWORD bytesRead = 0;
  if(!ReadFile(fileHandle,
               gameFile.content,
               fileSize32,
               &bytesRead,
               0))
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_READ_ENTIRE_FILE_GET_FILE_MEMORY_CLEANUP;
  }

  if (bytesRead != fileSize32)
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_READ_ENTIRE_FILE_GET_FILE_MEMORY_CLEANUP;
  }

  // NOTE(Cristián): FileRead was successful.
  //                 Cleanup code is below the return
  gameFile.contentSize = fileSize32;
  gameFile.valid = true;
  goto LABEL_PLATFORM_READ_ENTIRE_FILE_CREATE_FILE_CLEANUP;

LABEL_PLATFORM_READ_ENTIRE_FILE_GET_FILE_MEMORY_CLEANUP:
  DEBUG_PlatformFreeGameFile(&gameFile);
LABEL_PLATFORM_READ_ENTIRE_FILE_CREATE_FILE_CLEANUP:
  CloseHandle(fileHandle);
LABEL_PLATFORM_READ_ENTIRE_FILE_RETURN:
  return(gameFile);
}
internal void
DEBUG_PlatformFreeGameFile(game_file *gameFile)
{
  if(gameFile->content)
  {
    VirtualFree(gameFile->content, 0, MEM_RELEASE);
  }
  *gameFile = {};
}

internal bool32
DEBUG_PlatformWriteEntireFile(char *fileName,
                              uint32 memorySize,
                              void *fileMemory)
{
  bool32 successfulWrite = false;
  HANDLE fileHandle = CreateFileA(fileName,
      GENERIC_WRITE,
      0,
      0,
      CREATE_ALWAYS,
      0,
      0);
  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    // TODO(Cristián): Better error code
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_WRITE_ENTIRE_FILE_RETURN;
  }

  DWORD bytesWritten;
  if(!WriteFile(fileHandle,
                fileMemory,
                memorySize,
                &bytesWritten,
                0))
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_WRITE_ENTIRE_FILE_CREATE_FILE_CLEANUP;
  }

  if (bytesWritten != memorySize)
  {
    // TODO(Cristián): Loggin'
    goto LABEL_PLATFORM_WRITE_ENTIRE_FILE_CREATE_FILE_CLEANUP;
  }

  // NOTE(Cristián): WriteFile was successful.
  //                 Cleanup code is below the return
  successfulWrite = true;

LABEL_PLATFORM_WRITE_ENTIRE_FILE_CREATE_FILE_CLEANUP:
  CloseHandle(fileHandle);
LABEL_PLATFORM_WRITE_ENTIRE_FILE_RETURN:
  return(successfulWrite);
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
            // TODO(Cristián): Move this to the game layer
            //gSoundOutput.ModifyBufferToneHz(-2);
          }
          else if(vKeyCode == VK_RIGHT)
          {
            // TODO(Cristián): Move this to the game layer
            //gSoundOutput.ModifyBufferToneHz(2);
          }
          else if(vKeyCode == VK_ESCAPE)
          {
          }
          else if(vKeyCode == VK_SPACE)
          {
          }
          else // All other keys are treated by default
          {
            result = DefWindowProcA(windowHandle, message, wParam, lParam);
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

  LARGE_INTEGER perfCounterFrequecyResult;
  QueryPerformanceFrequency(&perfCounterFrequecyResult);
  gPerformanceCounterFrequency = perfCounterFrequecyResult.QuadPart;

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

      /**
       *  SOUND INITIALIZATION
       */

      gSoundOutput.samplesPerSecond = 48000; // 48kHz
      gSoundOutput.latency = gSoundOutput.samplesPerSecond / 15;   // 15 frames of latency
      gSoundOutput.nChannels = 2;
      gSoundOutput.bytesPerBlock = gSoundOutput.nChannels * sizeof(int16);
      gSoundOutput.bufferSize = gSoundOutput.samplesPerSecond * gSoundOutput.bytesPerBlock;

      /**
       * NOTE(Cristián): VirtualAlloc by default clears the memory to 0,
       *                 unless MEM_RESET is specified.
       */

      // We allocalte the buffer
      // TODO(Cristián): Pool with Graphics Virtual Alloc
      gSoundOutput.bufferMemory = VirtualAlloc(0, gSoundOutput.bufferSize,
                                               MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

      // An index that counts how many samples we've outputed. We can use the module operator
      // to make a running index of our buffer
      Win32InitDirectSound(windowHandle, &gSoundOutput);

      // NOTE(Cristián): Test Code
      Win32ClearBuffer(&gSoundOutput);
      //Win32FillSoundBuffer(&gSoundOutput, 0, gSoundOutput.bufferSize);
      Win32PlayDirectSound();

      /**
       *  GAME MEMORY INITIALIZATION
       */
#if HANDMADE_INTERNAL
      // NOTE(Cristián): Specify the base address for memory allocation
      LPVOID baseAddress = (LPVOID)TERABYTES(2);
#else
      LPVOID baseAddress = 0;
#endif
      game_memory gameMemory = {};
      gameMemory.permanentStorageSize = MEGABYTES(64);
      gameMemory.transientStorageSize = GIGABYTES(4);

      // TODO(Cristián): Handle varios memory footprints
      //                 Use system metric on *physical* memory
      uint64 totalSize = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
      gameMemory.permanentStorage = VirtualAlloc(baseAddress,
                                                 totalSize,
                                                 MEM_RESERVE|MEM_COMMIT,
                                                 PAGE_READWRITE);
      gameMemory.transientStorage = ((uint8 *)gameMemory.permanentStorage +
                                    gameMemory.permanentStorageSize);


      if(!gSoundOutput.bufferMemory ||
         !gameMemory.permanentStorage ||
         !gameMemory.transientStorageSize)
      {
        // NOTE(Cristián): We weren't able to allocate all the memory needed
        // TODO(Cristián): Loggin'
        return(0);
      }


      // ** MESSAGE LOOP **
      // We retrieve the messages from windows via the message queue
      LARGE_INTEGER lastCounter;
      QueryPerformanceCounter(&lastCounter);

      // We put the amount of cycles made by the processor
      uint64 lastCycleCount = __rdtsc();

      game_input gameInputs[2] = {};
      game_input *oldInput = &gameInputs[0];
      game_input *newInput = &gameInputs[1];

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
        int32 maxControllerCount = XUSER_MAX_COUNT;
        if (maxControllerCount > ARRAY_COUNT(oldInput->controllers))
        {
          maxControllerCount = ARRAY_COUNT(oldInput->controllers);
        }
        for(DWORD controllerIndex = 0;
            controllerIndex < maxControllerCount;
            controllerIndex++)
        {
          DWORD result;
          XINPUT_STATE controllerState;
          game_controller_input *oldController = &oldInput->controllers[controllerIndex];
          game_controller_input *newController = &newInput->controllers[controllerIndex];

          if(XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) // Amazing success key code name
          {
            // TODO(Cristián): See if controllerState.swPacketNumber incrementes too rapidly

            x_input_gamepad_state gamepadState = GetGamepadState(&controllerState);

            /**
             * TODO(Cristián): Handle the deadzone of controller correctly
            #define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
            #define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
            #define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30
            */

            // We generate the game_input
            newController->isAnalog = true;
            real32 x = ((real32)(gamepadState.leftThumbX + 32768) /
                       32768.0f) - 1.0f;
            newController->startX = oldController->endX;
            newController->endX = x;

            real32 y = ((real32)(gamepadState.leftThumbY + 32768) /
                       32768.0f) - 1.0f;
            newController->startY = oldController->endY;
            newController->endY = y;

            Win32ProcessButtonState(&oldController->a, &newController->a, gamepadState.aButton);
            Win32ProcessButtonState(&oldController->b, &newController->b, gamepadState.bButton);
            Win32ProcessButtonState(&oldController->x, &newController->x, gamepadState.xButton);
            Win32ProcessButtonState(&oldController->y, &newController->y, gamepadState.yButton);
            Win32ProcessButtonState(&oldController->leftShoulder, &newController->leftShoulder,
                                    gamepadState.leftTrigger);
            Win32ProcessButtonState(&oldController->rightShoulder, &newController->rightShoulder,
                                    gamepadState.rightTrigger);


          }
          else
          {
            // NOTE(Cristián): The controller is not available
          }

        }

        game_offscreen_buffer gameBuffer = {};
        gameBuffer.memory = gBackBuffer.memory;
        gameBuffer.width = gBackBuffer.width;
        gameBuffer.height = gBackBuffer.height;
        gameBuffer.pitch = gBackBuffer.pitch;

        bool32 validSound = false;
        game_sound_output_buffer gameSoundOutput = {};
        if(Win32SetupSoundBuffer(&gSoundOutput))
        {

          gameSoundOutput.bufferMemory = gSoundOutput.bufferMemory;
          gameSoundOutput.samplesPerSecond = gSoundOutput.samplesPerSecond;
          gameSoundOutput.sampleCount = gSoundOutput.bytesToWrite /
                                        gSoundOutput.bytesPerBlock;

          validSound = true;
        }

        GameUpdateAndRender(&gameMemory, &gameBuffer, &gameSoundOutput, newInput);

        if (validSound)
        {
          Win32FillSoundBuffer(&gSoundOutput,
                               &gameSoundOutput);
        }


        win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
        Win32TransferBufferToWindows(deviceContext,
                                     &gBackBuffer,
                                     dimension.width, dimension.height);


        uint64 endCycleCount = __rdtsc();

        LARGE_INTEGER endCounter;
        QueryPerformanceCounter(&endCounter);

#if 0
        uint64 counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
        real32 msPerFrame = 1000 * (real32)counterElapsed / gPerformanceCounterFrequency;
        uint32 fps = 1000 / msPerFrame;

        uint64 cyclesElapsed = endCycleCount - lastCycleCount;
        real32 megaCycles = cyclesElapsed / (1000.0f * 1000.0f);

        // TODO(Cristián): Remove from production
        char buffer[256];
        //wsprintf(buffer, "ms / frame: %d ms\n", msPerFrame);
        sprintf(buffer, "%f ms\t| %d FPS\t| %d MegaCycles\n", msPerFrame, fps, megaCycles);
        OutputDebugStringA(buffer);
#endif

        // We update the lastCounter
        lastCounter = endCounter;
        lastCycleCount = endCycleCount;

        // We swap the controllers
        game_input *tempInputState = newInput;
        newInput = oldInput;
        oldInput = tempInputState;
        // TODO(Cristián): Clear the newInput??
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


