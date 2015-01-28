/*  =====================================================================
    $File: handmade.h
    $Creation Date: 2015-01-19
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _HANDMADE_H_INCLUDED

/**
 * TODO(Cristián): Services that the platform layer provides to the game
 */

/**
 * TODO(Cristián): Services that the game provides to the platform layer
 * (this may expand in the future (sound on a separate thread))
 */
// TODO(Cristián): In the future, rendering *specifically* will become a
// three-tier abstraction
struct game_offscreen_buffer
{
  void *memory;
  int width;
  int height;
  int pitch;
};

struct game_sound_ouput_buffer
{
  void *bufferMemory;
  int32 sampleCount;

  int32 toneVolume;
  int32 samplesPerSecond;
  int32 toneHz;
};


internal void GameUpdateAndRender(game_offscreen_buffer *buffer);

#define _HANDMADE_H_INCLUDED
#endif
