#ifndef _GAME_TILES_H

#include "math/vector.h"
#include "game/entity.h"

#define ENTITIES_PER_CHUNK 32
struct tile_chunk
{
  MATH::v3<int32> tileChunkCoords;
  bool32 initialized;
  uint32* tiles;

  entity_def* entities[ENTITIES_PER_CHUNK];
  tile_chunk* nextTileChunkInHash;
};

#define TILE_CHUNK_HASH_SIZE 4096
struct tile_map
{
  int tileShift;
  int tileMask;
  int tileSide;

  real32 tileInMeters;
  tile_chunk* tileChunkHash[TILE_CHUNK_HASH_SIZE] = {};
};


#define _GAME_TILES_H
#endif
