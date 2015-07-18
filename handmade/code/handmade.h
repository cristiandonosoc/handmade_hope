/*  =====================================================================
    $File: handmade.h
    $Creation Date: 2015-01-19
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

/**
 * NOTE(Cristián):
 *
 * HANDMADE_INTERNAL:
 *  0 - Build for public release
 *  1 - Build for developer only
 *
 * HANDMADE_SLOW:
 *  0 - No slow code allowed
 *  1 - Slow code allowed
 */
#ifndef _HANDMADE_H_INCLUDED

#include "common_types.h"
#include "handmade_platform.h"

/**
 * GAME FUNCTIONS
 */

#define GAME_UPDATE_AND_RENDER(name)\
  void name(thread_context *threadContext,\
            game_offscreen_buffer *offscreenBuffer,\
            game_memory *gameMemory,\
            game_input *gameInput)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub)
{
  /* STUB */
}

// NOTE(Cristián): At the moment, this has to be a very fast
// function (1ms or so)
// TODO(Cristián): Reduce the pressure on this function by
// measuring or querying.
#define GAME_GET_SOUND(name)\
  void name(thread_context *threadContext,\
            game_sound_output_buffer *soundBuffer,\
            game_memory *gameMemory,\
            game_input *gameInput)
typedef GAME_GET_SOUND(game_get_sound);
GAME_GET_SOUND(GameGetSoundStub)
{
  /* STUB */
}


/*** GAME OBJECTS ***/
#define TILE_ROWS 9
#define TILE_COLUMNS 17

struct world_map; // Forward-declare
struct tile_map
{
  uint32* tiles;
  world_map* world;
};

struct world_coordinates
{
  int32 tileMapX;
  int32 tileMapY;
  int32 tileX;
  int32 tileY;
  real32 pX;
  real32 pY;
};

struct world_map
{
  int32 tileCountX;
  int32 tileCountY;
  int32 tileWidth;
  int32 tileHeight;
  int offsetX;
  int offsetY;

  // TODO(Cristian): Sparseness
  int32 tileMapCountX;
  int32 tileMapCountY;

  tile_map* tileMaps;

  // Functions
  tile_map* getTileMap(world_coordinates* coords)
  {
    int32 x = coords->tileMapX;
    int32 y = coords->tileMapY;

    if((x >= 0 && x < this->tileMapCountX) &&
       (y >= 0 || y < this->tileMapCountY))
    {
      return tileMaps + (y * tileMapCountY) + x;
    }

    return nullptr;
  }

  int32 getTileMapWidth()
  {
    return this->tileCountX * this->tileWidth;
  }

  int32 getTileMapHeight()
  {
    return this->tileCountY * this->tileHeight;
  }

};

struct game_state
{
  int32 toneHz;
  int32 toneVolume;
  real32 tSine;

  world_coordinates coords;
};



#define _HANDMADE_H_INCLUDED
#endif
