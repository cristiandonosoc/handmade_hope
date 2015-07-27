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

struct world_map; // Forward-declare

struct tile_coordinates
{
  // NOTE(Cristian): Represent the global (obsolute) position of the tile
  // in the world
  int32 tileX;
  int32 tileY;
  int32 tileZ;

  // Movement (in world) meters from the referenced tile ny (tileX, tileY)
  // If the coordinates are normalized, both pX and pY should be
  // 0 <= pX, pY < tileInMeters
  real32 pX;
  real32 pY;
};


struct tile_chunk
{
  bool32 initialized;
  uint32* tiles;
};

struct tile_map
{
  int tileShift;
  int tileMask;
  int tileSide;

  real32 tileInMeters;

  // TODO(Cristian): Sparseness
  int32 tileChunkCountX;
  int32 tileChunkCountY;
  int32 tileChunkCountZ;

  tile_chunk* tileChunks;
};

struct world_definition
{
  tile_map* tileMap;
};

struct memory_manager
{
  size_t size;
  size_t used;

  uint8* base;
};

struct game_state
{
  int32 toneHz;
  int32 toneVolume;

  bool32 zChangePress;

  memory_manager memoryManager;
  world_definition* world;
  tile_coordinates coords;
};



#define _HANDMADE_H_INCLUDED
#endif
