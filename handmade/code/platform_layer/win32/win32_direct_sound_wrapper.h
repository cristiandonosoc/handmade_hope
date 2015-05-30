/*  =====================================================================
    $File: platform_layerwin32win32_direct_sound_wrapper.h
    $Creation Date: 2015-01-28
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _WIN32_DIRECT_SOUND_WRAPPER_H_INCLUDED

struct win32_sound_output
{
  // Buffer Definition
  int32 samplesPerSecond;
  int32 nChannels;
  int32 bytesPerBlock;
  int32 bufferSize;
  int32 latency;
  uint32 expectedSoundBytesPerFrame;
  uint32 safetyBytes;

  // Buffer
  void *bufferMemory;

  // VARIABLE DATA
  int32 runningBlockIndex;
  uint32 byteToLock;
  uint32 bytesToWrite;
  DWORD playCursor;
  DWORD writeCursor;
  real32 latencySeconds;
};

#define _WIN32_DIRECT_SOUND_WRAPPER_H_INCLUDED
#endif
