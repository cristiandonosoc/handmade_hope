/*  =====================================================================
    $File: handmade.h
    $Creation Date: 2015-01-19
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _HANDMADE_H_INCLUDED

#define ARRAY_COUNT(Array) (sizeof(Array) / sizeof((Array)[0]))

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

struct game_sound_output_buffer
{
  void *bufferMemory;
  int32 sampleCount;

  int32 toneVolume;
  int32 samplesPerSecond;
  int32 toneHz;
};

struct game_button_state
{
  int halfTransitionCount;
  bool32 endedDown;
};

struct game_controller_input
{
  bool32 isAnalog;

  real32 startX;
  real32 startY;

  real32 minX;
  real32 minY;

  real32 maxX;
  real32 maxY;

  real32 endX;
  real32 endY;

  /**
   * union allows for several ways to access the same memory.
   * In this case, the game_controller_input is always the same in memory,
   * but I can access it through the buttons array or directly into the struct.
   * As both are pointing to the *same* memory, any changes from one will be
   * reflected on the other.
   */
  union
  {
    game_button_state buttons[6];
    struct
    {
      game_button_state a;
      game_button_state b;
      game_button_state x;
      game_button_state y;
      game_button_state leftShoulder;
      game_button_state rightShoulder;
    };
  };
};


struct game_input
{
  game_controller_input controllers[4];
};


internal void GameUpdateAndRender(game_offscreen_buffer *offscreenBuffer,
                                  game_sound_output_buffer *soundBuffer,
                                  game_input *gameInput);

#define _HANDMADE_H_INCLUDED
#endif
