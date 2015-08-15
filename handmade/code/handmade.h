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
#include "game/coordinates.h"
#include "game/tiles.h"
#include "math/vector.h"

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



// pragma pack(push, 1) indicates that we push the packing mode 1 into a pragma stack...
// This means that from that moment on, the compiler packs the structs without padding.
// When we want to go back to the mode before, we simple do pack(pop)
#pragma pack(push, 1)
struct bitmap_header
{
  // FILE HEADER
  uint16 fileType;
  uint32 fileSize;
  uint16 reserved1;
  uint16 reserved2;
  uint32 bitmapStart;       // Where the actual bitmap data is relative to the first byte
  // INFO HEADER
  uint32 headerSize;
  int32 width;
  int32 height;
  uint16 planes;            // Number of color planes
  uint16 bitsPerPixel;
  uint32 compressionType;
  uint32 imageSize;         // Size of the bitmap data
  int32 resolutionX;
  int32 resolutionY;
  uint32 colorCount;
  uint32 importantColors;

  // NOTE(Cristian): This masks are used to indicate where in the int32
  // the R, G, B values actually are. This is used when the compression
  // value in the header is set to BI_BITFIELS (3 in the last check).
  uint32 redMask;
  uint32 greenMask;
  uint32 blueMask;
};
#pragma pack(pop)

struct bitmap_definition
{
  bitmap_header header;
  uint32* pixels;
};

struct hero_bitmap
{
  bitmap_definition torso;
  bitmap_definition cape;
  bitmap_definition head;
  int32 offsetX;
  int32 offsetY;
};

enum entity_type
{
  player,
  wall
};

enum entity_residence
{
  cold,
  hot
};

struct entity_def
{
  bool32 exists;
  entity_type type;
  entity_residence residence;
  tile_coordinates pos;
  MATH::v2<real32> dPos;
  MATH::v2<real32> hotPos;
  // LBRU
  uint32 facingDirection;

  real32 width;
  real32 height;
};

#define ENTITY_COUNT 256
struct game_state
{
  memory_manager memoryManager;
  world_definition* world;

  int32 entityCount;
  entity_def entities[ENTITY_COUNT];
  // Each index maps an controller to a particular entity index
  int32 entityIndexForController[ARRAY_COUNT(((game_input*)0)->controllers)];

  entity_def* hotEntities[ENTITY_COUNT];
  entity_def* coldEntities[ENTITY_COUNT];

  tile_coordinates cameraPos;
  uint32 cameraFollowingEntityIndex;

  bitmap_definition background;
  int32 heroBitmapIndex;
  hero_bitmap heroBitmaps[4];
};


#define _HANDMADE_H_INCLUDED
#endif
