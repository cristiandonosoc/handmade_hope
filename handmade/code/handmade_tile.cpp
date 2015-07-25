#ifndef _HANDMADE_TILE_CPP

#include "handmade_coordinates.cpp"

inline tile_chunk*
GetTileChunk(tile_map* tileMap, tile_coordinates* coords)
{
  int32 x = coords->tileX >> tileMap->tileShift;
  int32 y = coords->tileY >> tileMap->tileShift;

  if((x >= 0 && x < tileMap->tileChunkCountX) &&
     (y >= 0 || y < tileMap->tileChunkCountY))
  {
    tile_chunk* result = tileMap->tileChunks + (y * tileMap->tileChunkCountY) + x;
    return result;
  }

  return nullptr;
}

inline uint32*
GetTile(tile_map* tileMap, tile_coordinates* coords)
{
  // TODO(Cristian): REMOVE THIS FIX CODE!
  if(coords->tileX < 0 || coords->tileY < 0) { return 0; }

  int32 tileChunkX = coords->tileX >> tileMap->tileShift;
  int32 tileChunkY = coords->tileY >> tileMap->tileShift;

  ASSERT(tileChunkX >= 0 || tileChunkX < tileMap->tileChunkCountX);
  ASSERT(tileChunkY >= 0 || tileChunkY < tileMap->tileChunkCountY);

  int32 tileX = coords->tileX & tileMap->tileMask;
  int32 tileY = coords->tileY & tileMap->tileMask;

  ASSERT(tileX >= 0 || tileX < tileMap->tileMax);
  ASSERT(tileY >= 0 || tileY < tileMap->tileMax);

  tile_chunk* tileChunk = GetTileChunk(tileMap, coords);
  uint32* res = tileChunk->tiles + (tileY * tileMap->tileMax) + tileX;
  uint32 a = *res;
  return res;
}

internal bool32
PointValid(tile_map* tileMap, tile_coordinates* coords)
{
  uint32 tileValue = *GetTile(tileMap, coords);
  return tileValue == 0;
}

#define _HANDMADE_TILE_CPP
#endif
