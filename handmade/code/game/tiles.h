#ifndef _GAME_TILES_H

#include "math/vector.h"

struct tile_chunk
{
  MATH::v3<int32> tileChunkCoords;
  bool32 initialized;
  uint32* tiles;

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
