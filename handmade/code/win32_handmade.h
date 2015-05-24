/*  =====================================================================
    $File: win32_handmade.h
    $Creation Date: 2015-04-10
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _WIN32_HANDMADE_H

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

struct win32_debug_time_marker
{
  DWORD playCursor;
  DWORD writeCursor;
};

#define _WIN32_HANDMADE_H
#endif
