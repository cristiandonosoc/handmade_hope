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

internal void
ClearScreenBuffer(game_offscreen_buffer *buffer)
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

      *pixel++ = 0;
    }
    row += pitch;
  }

}

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
      uint8 blue = (uint8)(x + blueOffset);
      uint8 green = (uint8)(y + greenOffset);

      *pixel++ = blue | (green << 8);
    }
    row += pitch;
  }
}

internal void
OutputGameSound(game_sound_output_buffer *soundOutput,
                int32 toneHz, int32 toneVolume)
{
  if(!soundOutput->valid) { return; }

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
                    game_memory *gameMemory,
                    game_input *gameInput)
{
  ASSERT(((&gameInput->controllers[0].terminator) -
          (&gameInput->controllers[0].buttons[0])) ==
         ARRAY_COUNT(gameInput->controllers[0].buttons));
  ASSERT(sizeof(game_state) <= gameMemory->permanentStorageSize);

  game_state *gameState = (game_state *)gameMemory->permanentStorage;
  if(!gameMemory->graphicsInitialized)
  {
    // We try to read from files
    // TODO(Cristián): Stop allocating new memory for the file
    //                 Instead reserve memory from the already allocated
    //                 gameMemory
    char *fileName = __FILE__;
    game_file gameFile = DEBUG_PlatformReadEntireFile(fileName);
    if (gameFile.content)
    {
      DEBUG_PlatformWriteEntireFile("file_out.test",
                                    gameFile.contentSize,
                                    gameFile.content);
      DEBUG_PlatformFreeGameFile(&gameFile);
    }

    // TODO(Cristián): This may be more appropiate to do in the platform layer
    gameMemory->graphicsInitialized = true;
  }

  for(int controllerIndex = 0;
      controllerIndex < ARRAY_COUNT(gameInput->controllers);
      controllerIndex++)
  {
    game_controller_input *input =
      GetController(gameInput, controllerIndex);

    if(input->isAnalog)
    {
      // NOTE(Cristián): Use analog movement tuninY
      gameState->xOffset += (int32)(10 * input->leftStickAverageX);
      gameState->yOffset += (int32)(10 * input->leftStickAverageY);
    }

    if(input->actionDown.endedDown)
    {
      gameState->xOffset += 1;
    }

  }
  bool32 debug = false;
  if(debug && !debug)
  {
    RenderWeirdGradient(offscreenBuffer,
                        gameState->xOffset,
                        gameState->yOffset);
  }
  else
  {
    ClearScreenBuffer(offscreenBuffer);
  }
}

internal void
GameGetSound(game_sound_output_buffer *soundBuffer,
             game_memory *gameMemory,
             game_input *gameInput)
{

  ASSERT(((&gameInput->controllers[0].terminator) -
          (&gameInput->controllers[0].buttons[0])) ==
         ARRAY_COUNT(gameInput->controllers[0].buttons));
  ASSERT(sizeof(game_state) <= gameMemory->permanentStorageSize);

  game_state *gameState = (game_state *)gameMemory->permanentStorage;
  if(!gameMemory->soundInitialized)
  {
    gameState->toneHz = 440;
    gameState->toneVolume = 7000;

    // TODO(Cristián): This may be more appropiate to do in the platform layer
    gameMemory->soundInitialized = true;
  }

  gameState->toneHz = 256;

  for(int controllerIndex = 0;
      controllerIndex < ARRAY_COUNT(gameInput->controllers);
      controllerIndex++)
  {
    game_controller_input *input =
      GetController(gameInput, controllerIndex);

    if(input->isAnalog)
    {
      // NOTE(Cristián): Use analog movement tuninY
    }
    else
    {
      if (input->moveRight.endedDown)
      {
        gameState->toneHz = 256 + (int32)(120.0f * input->moveRight.endedDown);
      }
    }
  }
  OutputGameSound(soundBuffer,
                  gameState->toneHz,
                  gameState->toneVolume);
}

#define _HANDMADE_CPP_INCLUDED
#endif

