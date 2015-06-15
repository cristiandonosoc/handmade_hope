/*  =====================================================================
    $File: win32_handmade.h
    $Creation Date: 2015-04-10
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _WIN32_HANDMADE_H

struct win32_game_code
{
  HMODULE gameCodeDll;
  bool32 valid;
  game_update_and_render *updateAndRenderFunction;
  game_get_sound *getSoundFunction;
  FILETIME lastWriteTime;
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
