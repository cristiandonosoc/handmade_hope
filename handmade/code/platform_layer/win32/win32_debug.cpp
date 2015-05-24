/*  =====================================================================
    $File: platform_layerwin32win32_debug.cpp
    $Creation Date: 2015-05-09
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _WIN32_DEBUG_CPP

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
Win32DebugSyncDisplay(win32_offscreen_buffer *backBuffer,
                      win32_sound_output *soundOutput,
                      win32_debug_time_marker* debugTimeMarkers,
                      int markerCount,
                      real32 targetSecondsPerFrame)
{
  int padX = 16;
  int padY = 16;
  int top = padY;
  int bottom = backBuffer->height - padY;

  real32 c = (real32)(backBuffer->width - 2 * padX) / (real32)soundOutput->bufferSize;

  for(int markerIndex = 0;
      markerIndex < markerCount;
      markerIndex++)
  {
    real32 playCursor = (real32)debugTimeMarkers[markerIndex].playCursor;
    real32 playX = padX + (int)(c * playCursor);
    Win32DebugDrawVerticalLine(backBuffer, playX, top, bottom, 0xFFFFFFFF);

    real32 writeCursor = (real32)debugTimeMarkers[markerIndex].writeCursor;
    real32 writeX = padX + (int)(c * writeCursor);
    Win32DebugDrawVerticalLine(backBuffer, writeX, top, bottom, 0xFF000000);
 
  }
}



#define _WIN32_DEBUG_CPP
#endif
