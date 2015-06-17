/*  =====================================================================
    $File: platform_layerwin32win32_debug.h
    $Creation Date: 2015-05-25
    $Last Modified: $
    $Revision: $
    $Creator: Cristi�n Donoso $
    $Notice: (c) Copyright 2015 Cristi�n Donoso $
    ===================================================================== */

#ifndef _WIN32_DEBUG_H

struct win32_debug_time_marker
{
  DWORD fillPlayCursor;
  DWORD fillWriteCursor;
  DWORD flipPlayCursor;
  DWORD flipWriteCursor;
  DWORD runningBlockIndex;
  DWORD byteToLock;
  DWORD byteToWrite;
};

#define _WIN32_DEBUG_H
#endif
