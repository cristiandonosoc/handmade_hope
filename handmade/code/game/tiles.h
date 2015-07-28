#ifndef _GAME_TILES_H

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


#define _GAME_TILES_H
#endif
