/*  =====================================================================
    $File: platform_layerwin32win32_debug.cpp
    $Creation Date: 2015-05-09
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _WIN32_DEBUG_CPP
#include "win32_debug.h"

internal void
Win32DebugDrawVerticalLine(win32_offscreen_buffer *backBuffer,
                           int x,
                           int top,
                           int bottom,
                           uint32 color)
{
  uint8 *pixel = (uint8 *)backBuffer->memory +
                 4 * x +
                 top * backBuffer->pitch;

  for (int y = top;
       y < bottom;
       y++)
  {
    *(uint32 *)pixel = color;
    pixel += backBuffer->pitch;
  }
}

internal void
Win32DebugDrawVerticalLineFromValue(
  win32_offscreen_buffer *backBuffer,
  win32_sound_output *soundOutput,
  int padX, int padY, int top, int bottom,
  real32 value, uint32 color)
{
  ASSERT(value < soundOutput->bufferSize);
  real32 c = (real32)(backBuffer->width - 2 * padX) /
             (real32)soundOutput->bufferSize;
  int x = padX + (int)(c * value);
  Win32DebugDrawVerticalLine(backBuffer, x, top, bottom, color);
}

internal void
Win32DebugSyncDisplay(win32_offscreen_buffer *backBuffer,
                      win32_sound_output *soundOutput,
                      win32_debug_time_marker* debugTimeMarkers,
                      int markerCount,
                      int currentMarkerIndex)
{
  DWORD playColor = 0xFFFFFFFF;
  DWORD writeColor = 0xFF0000FF;
  DWORD byteToLockColor = 0xFFFF0000;
  DWORD byteToWriteColor = 0xFFFFFF00;
  DWORD flipPlayCursorColor = 0xFFFFFFF;
  DWORD flipWriteCursorColor = 0xFF0000FF;

  for(int markerIndex = 0;
      markerIndex < markerCount;
      markerIndex++)
  {
    int padX = 16;
    int padY = 16;
    int top = padY;
    int bottom = backBuffer->height / 10;
    if(markerIndex == currentMarkerIndex)
    {

      top = backBuffer->height / 8;
      bottom = backBuffer->height / 6;

      Win32DebugDrawVerticalLineFromValue(
          backBuffer,
          soundOutput,
          padX, padY, top, bottom,
          (real32)debugTimeMarkers[markerIndex].byteToLock,
          byteToLockColor);

      Win32DebugDrawVerticalLineFromValue(
          backBuffer,
          soundOutput,
          padX, padY, top, bottom,
          (real32)debugTimeMarkers[markerIndex].byteToWrite,
          byteToWriteColor);

      Win32DebugDrawVerticalLineFromValue(
          backBuffer,
          soundOutput,
          padX, padY, top, bottom,
          (real32)debugTimeMarkers[markerIndex].flipPlayCursor,
          flipPlayCursorColor);

      Win32DebugDrawVerticalLineFromValue(
          backBuffer,
          soundOutput,
          padX, padY, top, bottom,
          (real32)debugTimeMarkers[markerIndex].flipWriteCursor,
          flipWriteCursorColor);


      top = padY;
      bottom = backBuffer->height / 8;
    }
    Win32DebugDrawVerticalLineFromValue(
        backBuffer,
        soundOutput,
        padX, padY, top, bottom,
        (real32)debugTimeMarkers[markerIndex].fillPlayCursor,
        playColor);

    Win32DebugDrawVerticalLineFromValue(
        backBuffer,
        soundOutput,
        padX, padY, top, bottom,
        (real32)debugTimeMarkers[markerIndex].fillWriteCursor,
        writeColor);

  }
}



#define _WIN32_DEBUG_CPP
#endif
