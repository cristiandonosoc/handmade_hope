/*  =====================================================================
    $File: platform_layerwin32win32_debug.h
    $Creation Date: 2015-05-25
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _WIN32_DEBUG_H

struct win32_debug_time_marker
{
  DWORD playCursor;
  DWORD writeCursor;
  int32 runningBlockIndex;
};

#define _WIN32_DEBUG_H
#endif
