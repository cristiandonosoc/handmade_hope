/*  =====================================================================
    $File: handmade.cpp
    $Creation Date: 2015-01-19
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _HANDMADE_CPP_INCLUDED
#include "handmade.h"

#include <math.h> // TODO(Cristián): Implement our own sine function

/**
 * Writes a 'weird' gradient into a memory buffer
 * @param *buffer A pointer to the buffer info struct. We can pass it by reference also
 *                because the struct does not contain the buffer itself, but a pointer, so
 *                this method does not ACTUALLY modify the struct, but the memory it points to.
 *                We pass a pointer in order to not copy a fairly big structure into memory via
 *                the stack.
 */
internal void
RenderWeirdGradient(game_offscreen_buffer *buffer, int blueOffset, int greenOffset)
{
  // TODO(Cristián): Let's see what the optimizer does if it is passed by reference
  // instead of a pointer.
  int bitmapWidth = buffer->width;
  int bitmapHeight = buffer->height;
  int pitch = buffer->pitch;
  void *memory = buffer->memory;

  uint8* row = (uint8 *)memory;
  for(int y = 0;
      y < bitmapHeight;
      y++)
  {
    uint32 *pixel = (uint32 *)row;
    for(int x = 0;
        x < bitmapWidth;
        x++)
    {
      /*
        Pixel in Memory: BB GG RR xx
        Little Endian Architecture
        Pixel in Register: 0xXXRRGGBB
       */
      uint8 blue = (x + blueOffset);
      uint8 green = (y + greenOffset);
      uint8 red = 0;
      uint8 pad = 0;

      *pixel++ = blue | (green << 8);
    }
    row += pitch;
  }
}

internal void
OutputGameSound(game_sound_output_buffer *soundOutput,
                int32 toneHz, int32 toneVolume)
{
  /**
   * We write into the buffer by writing and advancing the output pointer
   * We make two writes because we created 2 channels, which makes the buffer to look like this:
   * [int16 int16] [int16 int16] ...
   * [LEFT  RIGHT] [LEFT  RIGHT] ...
   * [  SAMPLE   ] [  SAMPLE   ] ...
   *
   */

  local_persist real32 tSine;
  int32 wavePeriod = soundOutput->samplesPerSecond / toneHz;

  // We cast the region pointer into int16 pointers (it is a DWORD) so we can
  // write into each channel of the sound buffer
  int16 *sampleOut = (int16 *)soundOutput->bufferMemory;
  //int32 region1SampleCount = region1Size / soundOutput->bytesPerBlock;
  // TODO(Cristián): Assert that region sizes are valid (sample multiple)
  for(int32 sampleIndex = 0;
      sampleIndex < soundOutput->sampleCount;
      sampleIndex++)
  {
    real32 sineValue = sinf(tSine);
    int16 sampleValue = (int16)(sineValue * toneVolume);

    *sampleOut++ = sampleValue;
    *sampleOut++ = sampleValue;

    tSine += 2 * PI32 / (real32)wavePeriod;
    while(tSine > 2 * PI32)
    {
      tSine -= 2 * PI32;
    }
  }
}

internal void
GameUpdateAndRender(game_offscreen_buffer *offscreenBuffer,
                    game_sound_output_buffer *soundBuffer,
                    game_input *gameInput)
{

  local_persist int32 xOffset = 0;
  local_persist int32 yOffset = 0;
  local_persist int32 toneHz = 440;
  local_persist int32 toneVolume = 7000;

  game_controller_input *input0 = &gameInput->controllers[0];

  if(input0->isAnalog)
  {
    // NOTE(Cristián): Use analog movement tuning
    toneHz = 256 + (int32)(120.0f * input0->endX);
  }
  else
  {
    // NOTE(Cristián): Use digital movement tuning
  }

  if(input0->a.endedDown)
  {
    xOffset += 1;
  }



  OutputGameSound(soundBuffer, toneHz, toneVolume);
  RenderWeirdGradient(offscreenBuffer, xOffset, yOffset);
}

#define _HANDMADE_CPP_INCLUDED
#endif

