/*  =====================================================================
    $File: win32_handmade.cpp
    $Creation Date: 2014-12-27
    $Last Modified: 2014-12-28 17:14
    $Revision: $
    $Creator: Cristi�n Donoso $
    $Notice: (c) Copyright 2014 Cristi�n Donoso $
    =====================================================================

    TODO(Cristi�n): THIS IS NOT A FINAL PLATFORM LAYER
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
#include <windows.h>
#include "common_types.h"

// Platform specific code
#include "handmade.cpp"
#include "platform_layer/win32/win32_x_input_wrapper.cpp"
#include "platform_layer/win32/win32_graphics_wrapper.cpp"
#include "platform_layer/win32/win32_direct_sound_wrapper.cpp"
#include "platform_layer/win32/win32_file_io.cpp"


// Global Sound Buffer Management
global_variable win32_sound_output gSoundOutput;
global_variable uint64 gPerformanceCounterFrequency;
global_variable bool32 mainLoopIsRunning;

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
    case WM_QUIT:
    {
      mainLoopIsRunning = false;
    } break;
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
        mainLoopIsRunning = false;
      } break;
    case WM_DESTROY:
      {
        //TODO:(Cristi�n): Handle this with an error - Recreate window?
        mainLoopIsRunning = false;
      } break;
    // We use the switch to grab all the keys messages into one block
    // (they all cascade into WM_KEYUP)
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
      {
        ASSERT("Keyboard input came through a dispatch event");
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
      // NOTE(Cristi�n): Sice we specified CS_OWNDC for our WNDCLASS, we can just
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
       * NOTE(Cristi�n): VirtualAlloc by default clears the memory to 0,
       *                 unless MEM_RESET is specified.
       */

      // We allocalte the buffer
      // TODO(Cristi�n): Pool with Graphics Virtual Alloc
      gSoundOutput.bufferMemory = VirtualAlloc(0, gSoundOutput.bufferSize,
                                               MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

      // An index that counts how many samples we've outputed. We can use the module operator
      // to make a running index of our buffer
      Win32InitDirectSound(windowHandle, &gSoundOutput);

      // NOTE(Cristi�n): Test Code
      Win32ClearBuffer(&gSoundOutput);
      //Win32FillSoundBuffer(&gSoundOutput, 0, gSoundOutput.bufferSize);
      Win32PlayDirectSound();

      /**
       *  GAME MEMORY INITIALIZATION
       */
#if HANDMADE_INTERNAL
      // NOTE(Cristi�n): Specify the base address for memory allocation
      LPVOID baseAddress = (LPVOID)TERABYTES(2);
#else
      LPVOID baseAddress = 0;
#endif
      game_memory gameMemory = {};
      gameMemory.permanentStorageSize = MEGABYTES(64);
      gameMemory.transientStorageSize = GIGABYTES(4);

      // TODO(Cristi�n): Handle varios memory footprints
      //                 Use system metric on *physical* memory
      uint64 totalSize = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
      gameMemory.permanentStorage = VirtualAlloc(baseAddress,
                                                 (size_t)totalSize,
                                                 MEM_RESERVE|MEM_COMMIT,
                                                 PAGE_READWRITE);
      gameMemory.transientStorage = ((uint8 *)gameMemory.permanentStorage +
                                    gameMemory.permanentStorageSize);


      if(!gSoundOutput.bufferMemory ||
         !gameMemory.permanentStorage ||
         !gameMemory.transientStorageSize)
      {
        // NOTE(Cristi�n): We weren't able to allocate all the memory needed
        // TODO(Cristi�n): Loggin'
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

      mainLoopIsRunning = true;
      while(mainLoopIsRunning)
      {

        // TODO(Cristi�n): Zeroing macro
        // NOTE(Cristi�n): We can't zero everything because the up/down state
        //                 will be wrong
        game_controller_input *oldKeyboardController =
          GetController(oldInput, 0);
        game_controller_input *newKeyboardController =
          GetController(newInput, 0);
        game_controller_input zeroController = {};
        *newKeyboardController = zeroController;
        newKeyboardController->isConnected = true;

        // We preserve the state of the buttons between the swapping
        // game inputs
        for(int buttonIndex = 0;
            buttonIndex < ARRAY_COUNT(newKeyboardController->buttons);
            buttonIndex++)
        {
          newKeyboardController->buttons[buttonIndex].endedDown =
            oldKeyboardController->buttons[buttonIndex].endedDown;
        }

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
          win32_keyboard_process_result result =
            Win32ProcessKeyboardMessages(message,
                                         oldKeyboardController,
                                         newKeyboardController);

          // We manage the results from the keyboard process
          if (result.quit)
          {
            mainLoopIsRunning = false;
          }

          if (result.unprocessed)
          {
            DefWindowProcA(windowHandle,
                           message.message,
                           message.wParam,
                           message.lParam);
          }
        }

        // Xinput is a polling based API
        // TODO(Cristi�n): Should we pull this more frequently?
        DWORD maxControllerCount = XUSER_MAX_COUNT + 1;
        if (maxControllerCount > ARRAY_COUNT(oldInput->controllers))
        {
          maxControllerCount = ARRAY_COUNT(oldInput->controllers);
        }

        // We iterate over all the controllers (gamepads)
        for(DWORD controllerIndex = 1;
            controllerIndex < maxControllerCount;
            controllerIndex++)
        {
          XINPUT_STATE controllerState;
          // NOTE(Cristi�n): We traduce to our controller index because
          // currently the keyboard is game_input 0
          int internalControllerIndex = controllerIndex - 1;
          game_controller_input *oldController =
            GetController(oldInput, internalControllerIndex);
          game_controller_input *newController =
            GetController(newInput, internalControllerIndex);

          if(XInputGetState(internalControllerIndex,
                            &controllerState) == ERROR_SUCCESS) // Amazing success key code name
          {
            newController->isConnected = true;
            // TODO(Cristi�n): See if controllerState.swPacketNumber incrementes too rapidly
            x_input_gamepad_state gamepadState = GetGamepadState(&controllerState);
            Win32ProcessGamepadState(oldController,
                                     newController,
                                     gamepadState);
          }
          else
          {
            // NOTE(Cristi�n): The controller is not available
            newController->isConnected = false;
          }

        }

        // The video buffer to be passed to the game
        game_offscreen_buffer gameOffscreenBuffer = {};
        gameOffscreenBuffer.memory = gBackBuffer.memory;
        gameOffscreenBuffer.width = gBackBuffer.width;
        gameOffscreenBuffer.height = gBackBuffer.height;
        gameOffscreenBuffer.pitch = gBackBuffer.pitch;

        // The sound ouput to be passed to the game
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

        /**
         * We call the game with the game memory, the graphics buffer,
         * the sound output and the current input
         */
        GameUpdateAndRender(&gameMemory,
                            &gameOffscreenBuffer,
                            &gameSoundOutput,
                            newInput);

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

        // TODO(Cristi�n): Remove from production
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
        // TODO(Cristi�n): Clear the newInput??
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


