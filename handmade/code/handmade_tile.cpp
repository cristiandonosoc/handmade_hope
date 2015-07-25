#ifndef _HANDMADE_TILE_CPP

#include "handmade_coordinates.cpp"

inline tile_chunk*
GetTileChunk(tile_map* tileMap, tile_coordinates* coords)
{
  int32_point tileChunkCoords = GetTileChunkCoordinates(tileMap, coords);

  // TODO(Cristian): Find a good way of finding if the tileChunk actually exists!
  if((tileChunkCoords.x >= 0 && tileChunkCoords.x < tileMap->tileChunkCountX) &&
     (tileChunkCoords.y >= 0 && tileChunkCoords.y < tileMap->tileChunkCountY))
  {
    tile_chunk* result = tileMap->tileChunks +
                         (tileChunkCoords.y * tileMap->tileChunkCountY) +
                         tileChunkCoords.x;
    return result;
  }

  return nullptr;
}

inline uint32*
GetTile(tile_map* tileMap, tile_coordinates* coords)
{
  int32_point tileCoords = GetTileCoordinates(tileMap, coords);

  ASSERT(tileCoords.x >= 0 && tileCoords.x < tileMap->tileMax);
  ASSERT(tileCoords.y >= 0 && tileCoords.y < tileMap->tileMax);

  int32_point tileChunkCoords = GetTileChunkCoordinates(tileMap, coords);
  tile_chunk* tileChunk = GetTileChunk(tileMap, coords);

  uint32* result = nullptr;
  if(tileChunk != nullptr)
  {
    result = tileChunk->tiles + (tileCoords.y * tileMap->tileMax) + tileCoords.x;
  }
  return result;
}

#define TILE_INVALID 0xFFFFFFFF
inline uint32
GetTileValue(tile_map* tileMap, tile_coordinates* coords)
{
  uint32 result = TILE_INVALID;

  uint32* tile = GetTile(tileMap, coords);
  if(tile != nullptr)
  {
    result = *tile;
  }

  return result;
}

#define _HANDMADE_TILE_CPP
#endif
