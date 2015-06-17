/*  =====================================================================
    $File: win32_handmade.h
    $Creation Date: 2015-04-10
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _WIN32_HANDMADE_H

struct win32_frame_snapshot
{
  game_input gameInput;
};

struct win32_game_code
{
  HMODULE gameCodeDll;
  bool32 valid;
  game_update_and_render *updateAndRenderFunction;
  game_get_sound *getSoundFunction;
  FILETIME lastWriteTime;
};

#define WIN32_MAX_PATH_LENGTH MAX_PATH
struct win32_state
{
  // Permanent Store Snapshot
  uint64 permanentSnapshotSize;
  void* permanentStorage; // for snapshot copy
  void* permanentSnapshot;

  // Frame Snapshot Recording
  bool snapshotRecording;
  bool snapshotPlayback;
  int snapshotMax;
  int snapshotRecordingIndex;
  int snapshotPlaybackIndex;
  uint64 snapshotsMemorySize;
  win32_frame_snapshot* snapshots;
  game_input* gameInput;

  // File paths
  char exeDirPath[WIN32_MAX_PATH_LENGTH];
  char* filename;
};

int CALLBACK
WinMain(HINSTANCE hInstance,
        HINSTANCE prevHInstance,
        LPSTR commandLine,
        int showCode);

LRESULT CALLBACK
Win32MainWindowCallback(HWND windowHandle,
                        UINT message,
                        WPARAM wParam,
                        LPARAM lParam);

inline LARGE_INTEGER
Win32GetWallClock();

inline real32
Win32GetSecondsElapsed(LARGE_INTEGER start,
                       LARGE_INTEGER end);

#define _WIN32_HANDMADE_H
#endif
