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
#include <windows.h>
#include "common_types.h"

// App Global Variables
global_variable bool32 gMainLoopIsRunning;
global_variable uint64 gPerformanceCounterFrequency;
global_variable bool32 gPauseApp;

#include "utils/string.cpp"

#include "handmade.h"
#include "win32_handmade.h"

// Platform specific code
#include "platform_layer/win32/input_recording.cpp"
#include "platform_layer/win32/x_input_wrapper.cpp"
#include "platform_layer/win32/direct_sound_wrapper.cpp"
#include "platform_layer/win32/graphics_wrapper.cpp"
#include "platform_layer/win32/file_io.cpp"
#include "platform_layer/win32/dll_loading.cpp"
#if HANDMADE_INTERNAL
#include "platform_layer/win32/debug.cpp"
#endif


/**
 * Main entrance for the program from the C-Runtime Library
 */
int CALLBACK
WinMain(HINSTANCE hInstance,
        HINSTANCE prevHInstance,
        LPSTR commandLine,
        int showCode)
{

  // The struct that holds the state of the platform
  win32_state win32State = {};

  /**
   * FILENAME AND PATH INITIALIZATION
   */
  DWORD sizeOfFileName = GetModuleFileNameA(0,
                                            win32State.exeDirPath,
                                            sizeof(win32State.exeDirPath));
  // We search for the filename pointer of the exe path
  char* onePastLastSlash = Utils::String::ScanForLastCharacter(win32State.exeDirPath, '\\');

  // We copy the filename
  win32State.filename = Utils::String::CopyString(onePastLastSlash);
  // We eliminate the filename, so now exeDirPath is now only the dir
  *onePastLastSlash = 0;

  char* sourceDllName = "handmade.dll";
  char* targetDllName = "handmade_temp.dll";
  // TODO(Cristián): use smart-pointers, so we free when we exit of scope
  char* sourceDllPath = Utils::String::CatStrings(win32State.exeDirPath, sourceDllName);
  char* targetDllPath = Utils::String::CatStrings(win32State.exeDirPath, targetDllName);

  // We initialize the XInput functions pointers
  Win32LoadXInput();

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

  // NOTE(Cristián): We set the windows scheduler granularity to 1ms
  //                 so that the thread gets waken at that interval
  UINT desiredSchedulerMs = 1;
  bool32 sleepIsGranular = (timeBeginPeriod(desiredSchedulerMs) == TIMERR_NOERROR);

  // TODO(Cristián): How do we reliably query this?
  //int monitorRefreshHz = 60;
#define gameUpdateHz 30
#define framesOfLatency 3
  real32 targetSecondsPerFrame = 1.0f / (real32)gameUpdateHz;

  // DEBUG(Cristián)
  int debugMarkerIndex = 0;
  win32_debug_time_marker debugMarkerCursors[gameUpdateHz / 2] = {0};

  if(RegisterClassA(&windowClass))
  {
    HWND windowHandle = CreateWindowExA(0,
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
                                        0);

    if(windowHandle)
    {
      // NOTE(Cristián): Sice we specified CS_OWNDC for our WNDCLASS, we can just
      // get one device context and use it forever, because we don't need to return it.
      HDC deviceContext = GetDC(windowHandle);

      /**
       *  SOUND INITIALIZATION
       */

      gSoundOutput.samplesPerSecond = 48000; // 48kHz
      gSoundOutput.latency = framesOfLatency *
        (gSoundOutput.samplesPerSecond / gameUpdateHz);
      gSoundOutput.nChannels = 2;
      gSoundOutput.bytesPerBlock = gSoundOutput.nChannels *
                                   sizeof(int16);
      gSoundOutput.bufferSize = gSoundOutput.samplesPerSecond *
                                gSoundOutput.bytesPerBlock;
#define soundSafetyMs 10
      gSoundOutput.safetyBytes = ((soundSafetyMs *
                                   gSoundOutput.samplesPerSecond *
                                   gSoundOutput.bytesPerBlock) /
                                  1000);
      gSoundOutput.expectedSoundBytesPerFrame =
        ((gSoundOutput.samplesPerSecond *
          gSoundOutput.bytesPerBlock) /
         gameUpdateHz);


      // Whether the sound this frame was valid
      bool32 validSound = false;

      /**
       * NOTE(Cristián): VirtualAlloc by default clears the memory to 0,
       *                 unless MEM_RESET is specified.
       */

      // We allocalte the buffer
      // TODO(Cristián): Pool with Graphics Virtual Alloc
      gSoundOutput.bufferMemory =
        VirtualAlloc(0,
                     gSoundOutput.bufferSize,
                     MEM_RESERVE|MEM_COMMIT,
                     PAGE_READWRITE);

      // An index that counts how many samples we've outputed. We can use the module operator
      // to make a running index of our buffer
      Win32InitDirectSound(windowHandle, &gSoundOutput);
      // TODO(Cristián): This is test code?
      Win32ClearBuffer(&gSoundOutput);
      Win32PlayDirectSound();

#if 0
      // NOTE(Cristián): This tests the playCursor/writeCursor update frequency.
      // On this machine it updates every 480 blocks
      bool testSound = true;
      while(testSound)
      {
        DWORD playCursor;
        DWORD writeCursor;
        if(SUCCEEDED(gSecondaryBuffer
          ->GetCurrentPosition(&playCursor,
                               &writeCursor)))
        {
          char buffer[256];
          sprintf_s(buffer,
                  "PC: %u, WC: %u\n",
                  playCursor,
                  writeCursor);
          OutputDebugStringA(buffer);
        }
      }
#endif

      /**
       *  GAME MEMORY INITIALIZATION
       */

      /*
       The game memory has the following memory layout:
        _________________________________________
       |                                        |
       |  PERMANENT STORAGE                     |
       |  (game state) ~64 MB                   |
       |________________________________________|
       |                                        |
       |  PERMANENT STORAGE SNAPSHOT COPY       |
       |  (for game looping) ~64 MB             |
       |________________________________________|
       |                                        |
       |  FRAME SNAPSHOTS                       |
       |  (for game looping) ~< 1 MB            |
       |________________________________________|
       |                                        |
       |  TRANSIENT STORE                       |
       |  (assets) Possibly > 2 GB              |
       |________________________________________|

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

      // PERMANENT STORE SNAPSHOT COPY

      // FRAME SNAPSHOT INITIALIZATION
      // -----------------------------
      // Frame snapshots begin right after the permanent storage
      // If sizeof(win32_frame_snapshot) == 1KB (1024)
      // then total size needed = 1800KB (for 60 seconds)
      win32State.snapshotMax = 60*30; // 60 seconds of recording at 30 fps
      win32State.snapshotsMemorySize = win32State.snapshotMax *
                                       sizeof(win32_frame_snapshot);

      // TODO(Cristián): Handle varios memory footprints
      //                 Use system metric on *physical* memory
      uint64 totalSize = gameMemory.permanentStorageSize +
                         gameMemory.permanentStorageSize + // snapshot copy
                         win32State.snapshotsMemorySize +
                         gameMemory.transientStorageSize;

      // Permanent storage pointer
      // TODO(Cristián): Use MEM_LARGE_PAGES and call AdjustTokenPrivileges
      gameMemory.permanentStorage = VirtualAlloc(baseAddress,
                                                 (size_t)totalSize,
                                                 MEM_RESERVE|MEM_COMMIT,
                                                 PAGE_READWRITE);

      // Permanent Snapshot pointer
      win32State.permanentStorage = gameMemory.permanentStorage;
      win32State.permanentSnapshot = ((uint8*)gameMemory.permanentStorage +
                                      gameMemory.permanentStorageSize);
      win32State.permanentSnapshotSize = gameMemory.permanentStorageSize;

      // Frame Snapshots pointer
      win32State.snapshots = (win32_frame_snapshot*)
                             ((uint8*)gameMemory.permanentStorage +
                              gameMemory.permanentStorageSize +
                              gameMemory.permanentStorageSize); // snapshot copy

      // Transient storage pointer
      gameMemory.transientStorage =
        ((uint8*)gameMemory.permanentStorage +
         gameMemory.permanentStorageSize +
         gameMemory.permanentStorageSize + // snapshot copy
         win32State.snapshotsMemorySize);

      /**
       * PLATFORM SERVICES FUNCTIONS POINTERS INITIALIZATION
       */
      gameMemory.DEBUGPlatformReadEntireFileFunction = DEBUGPlatformReadEntireFile;
      gameMemory.DEBUGPlatformFreeGameFileFunction = DEBUGPlatformFreeGameFile;
      gameMemory.DEBUGPlatformWriteEntireFileFunction = DEBUGPlatformWriteEntireFile;

      /**
       * GAME SOUND INITIALIZATION
       */
      if(!gSoundOutput.bufferMemory ||
         !gameMemory.permanentStorage ||
         !gameMemory.transientStorageSize)
      {
        // NOTE(Cristián): We weren't able to allocate all the memory needed
        // TODO(Cristián): Loggin'
        return(0);
      }

      LARGE_INTEGER lastCounter = Win32GetWallClock();
      // We put the amount of cycles made by the processor
      //uint64 lastCycleCount = __rdtsc();


      /**
       * INPUT INITIALIZATION
       */

      game_input gameInputs[2] = {};
      game_input* oldInput = &gameInputs[0];
      game_input* newInput = &gameInputs[1];

      // We copy the newInput into the win32State
      win32State.gameInput = newInput;

      /**
       * GAME CODE INITIALIZATION
       */
      win32_game_code currentGameCode = Win32LoadGameCode(sourceDllPath,
                                                          targetDllPath);
      int frameDelay = 20;
      int delayFrames = 0;
#define gameCodeLoadCounterLimit 120

      // ** MESSAGE LOOP **
      // We retrieve the messages from windows via the message queue
      gMainLoopIsRunning = true;
      while(gMainLoopIsRunning)
      {

        // TODO(Cristián): Find out WHY there is a process keeping a handle on the dll
        FILETIME newDllWriteTime = Win32GetLastWriteTime(sourceDllPath);
        // NOTE(Cristián): If one of the times is 0 (nullptr), the result
        // will be 0 (same as if they're equal)
        if(CompareFileTime(&currentGameCode.lastWriteTime,
                           &newDllWriteTime) != 0)
        {
          if(delayFrames++ == frameDelay)
          {
             Win32UnloadGameCode(&currentGameCode);
             currentGameCode = Win32LoadGameCode(sourceDllPath, targetDllPath);
             delayFrames = 0;
          }
        }
        // if(gameCodeLoadCounter++ > gameCodeLoadCounterLimit)
        // {
        //   Win32UnloadGameCode(&currentGameCode);
        //   currentGameCode = Win32LoadGameCode();
        //   gameCodeLoadCounter = 0;
        // }

        // TODO(Cristián): Zeroing macro
        // NOTE(Cristián): We can't zero everything because the up/down state
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
         * A DIFFERENCE (RAII) whether we put it in a loop because the spec requires it to call those
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
                                         newKeyboardController,
                                         &win32State);

          // We manage the results from the keyboard process
          if (result.quit)
          {
            gMainLoopIsRunning = false;
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
        // TODO(Cristián): Should we pull this more frequently?
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
          // NOTE(Cristián): We traduce to our controller index because
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
            // TODO(Cristián): See if controllerState.swPacketNumber incrementes too rapidly
            x_input_gamepad_state gamepadState = GetGamepadState(&controllerState);
            Win32ProcessGamepadState(oldController,
                                     newController,
                                     gamepadState);
          }
          else
          {
            // NOTE(Cristián): The controller is not available
            newController->isConnected = false;
          }

        }

        if(gPauseApp)
        {
          continue;
        }

        /**
         * APP LOGIC UPDATE
         */

        // The video buffer to be passed to the game
        game_offscreen_buffer gameOffscreenBuffer = {};
        gameOffscreenBuffer.memory = gBackBuffer.memory;
        gameOffscreenBuffer.width = gBackBuffer.width;
        gameOffscreenBuffer.height = gBackBuffer.height;
        gameOffscreenBuffer.pitch = gBackBuffer.pitch;


        // If sound is invalid, it means it is either the first run
        // or some strange sound buffer death
        real32 secondsElapsedSinceFrameFlip =
          Win32GetSecondsElapsed(lastCounter,
                                 Win32GetWallClock());
        real32 secondsExpectedToFrameFlip =
          targetSecondsPerFrame - secondsElapsedSinceFrameFlip;
        validSound = Win32SetupSoundBuffer(&gSoundOutput,
                                           secondsExpectedToFrameFlip,
                                           !validSound);

        // The sound ouput to be passed to the game
        game_sound_output_buffer gameSoundOutput = {};
        if(validSound)
        {
          gameSoundOutput.bufferMemory = gSoundOutput.bufferMemory;
          gameSoundOutput.samplesPerSecond = gSoundOutput.samplesPerSecond;
          gameSoundOutput.sampleCount = gSoundOutput.bytesToWrite /
                                        gSoundOutput.bytesPerBlock;
          gameSoundOutput.valid = true;
        }
        if (gameSoundOutput.valid)
        {
          currentGameCode.getSoundFunction(&gameSoundOutput,
                                           &gameMemory,
                                           newInput);
          Win32FillSoundBuffer(&gSoundOutput,
                               &gameSoundOutput);
        }

        if(win32State.snapshotRecording)
        {
          Win32RecordInput(&win32State, newInput);
        }
        else if(win32State.snapshotPlayback)
        {
          Win32PlaybackInput(&win32State, newInput);
        }

        // We call the game with the game memory, the graphics buffer,
        // This will generate all the content needed for the frame
        currentGameCode.updateAndRenderFunction(&gameOffscreenBuffer,
                                                &gameMemory,
                                                newInput);

        // We write to sound buffer inmediately because it has to sound even
        // when we are sleeping waiting for the page flip
        /**
         * We calculate the wall clock time that happened since the last counter.
         * After we wait the appropiate amount of time, we can show the frame and
         * fill the sound buffer
         */
        LARGE_INTEGER workCounter = Win32GetWallClock();
        real32 secondsElapsedForFrame = Win32GetSecondsElapsed(lastCounter,
                                                               workCounter);

        // We will verify all the amount of time we spent in the
        // game loop this iteration
        // TODO(Cristián): NOT TESTED YET! PROBABLY BUGGY!
        DWORD sleepMs = 0;
        if(secondsElapsedForFrame > targetSecondsPerFrame)
        {
          // We skipped a frame!
          // TODO(Cristián): Loggin'
        }
        else
        {
          // We are done for the frame, so we send the CPU to sleep
          if(sleepIsGranular)
          {
            // TODO(Cristián): We needed to remove a millisecond of sleep,
            //                 probably because the granularity is not that good
            sleepMs = (DWORD)(1000.0f * (targetSecondsPerFrame - secondsElapsedForFrame));
            if(sleepMs > 1)
            {
              // TODO(Cristián): See why the CPU is not sleeping by the amount we want!
              Sleep(sleepMs);
            }
            real32 testSecondsElapsedForFrame =
              Win32GetSecondsElapsed(lastCounter,
                                     Win32GetWallClock());
            if(testSecondsElapsedForFrame < targetSecondsPerFrame)
            {
              // TODO(Cristián): Log this missed sleep
            }
          }

          while(secondsElapsedForFrame < targetSecondsPerFrame)
          {
            secondsElapsedForFrame = Win32GetSecondsElapsed(lastCounter,
                                                            Win32GetWallClock());
          }
        }

        // We update the lastCounter so we can now how much
        // time has passed until the next check is issued
        lastCounter = Win32GetWallClock();


#if HANDMADE_INTERNAL
        int displayCurrentMarkerIndex = debugMarkerIndex - 1;
        if(displayCurrentMarkerIndex < 0)
        {
          displayCurrentMarkerIndex = ARRAY_COUNT(debugMarkerCursors);
        }

        // NOTE(Cristián): This is debug code
        Win32DebugSyncDisplay(&gBackBuffer,
                              &gSoundOutput,
                              debugMarkerCursors,
                              ARRAY_COUNT(debugMarkerCursors),
                              displayCurrentMarkerIndex);
#endif

        /**
         * FRAME FLIP
         */
        win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
        Win32TransferBufferToWindows(deviceContext,
                                     &gBackBuffer,
                                     dimension.width, dimension.height);





#if HANDMADE_INTERNAL
        {
          win32_debug_time_marker currentMarker = {};

          // We fill the currentMarker with the sound information
          currentMarker.fillPlayCursor = gSoundOutput.playCursor;
          currentMarker.fillWriteCursor = gSoundOutput.writeCursor;

          gSecondaryBuffer->GetCurrentPosition(&currentMarker.flipPlayCursor,
                                               &currentMarker.flipWriteCursor);
          currentMarker.runningBlockIndex =
            (gSoundOutput.runningBlockIndex * gSoundOutput.bytesPerBlock) %
            gSoundOutput.bufferSize;
          currentMarker.byteToLock = gSoundOutput.byteToLock;
          currentMarker.byteToWrite =
            ((gSoundOutput.byteToLock + gSoundOutput.bytesToWrite) %
            gSoundOutput.bufferSize);
          debugMarkerCursors[debugMarkerIndex++] = currentMarker;

          if (debugMarkerIndex >= ARRAY_COUNT(debugMarkerCursors))
          {
            debugMarkerIndex = 0;
          }
          ASSERT(debugMarkerIndex < ARRAY_COUNT(debugMarkerCursors));
        }



        real32 fps = 1.0f / secondsElapsedForFrame;
        // TODO(Cristián): Remove from production
        char buffer[256];
        //wsprintf(buffer, "ms / frame: %d ms\n", msPerFrame);
        sprintf_s(buffer,
                "%f ms\t| %f FPS\t| %d sleepMs\n",
                1000.0f * secondsElapsedForFrame,
                fps,
                sleepMs);
        OutputDebugStringA(buffer);
#endif

        //uint64 endCycleCount = __rdtsc();
        //uint64 cyclesElapsed = endCycleCount - lastCycleCount;
        //lastCycleCount = endCycleCount;

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

inline LARGE_INTEGER
Win32GetWallClock()
{
  LARGE_INTEGER result;
  QueryPerformanceCounter(&result);
  return result;
}


inline real32
Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
  uint64 counterElapsed = end.QuadPart - start.QuadPart;
  return (real32)counterElapsed / gPerformanceCounterFrequency;
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
    case WM_QUIT:
    {
      gMainLoopIsRunning = false;
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
        //TODO:(Cristián): Handle this with a message to the user
        gMainLoopIsRunning = false;
      } break;
    case WM_DESTROY:
      {
        //TODO:(Cristián): Handle this with an error - Recreate window?
        gMainLoopIsRunning = false;
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
