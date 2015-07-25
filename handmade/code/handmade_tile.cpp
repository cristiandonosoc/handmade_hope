#ifndef _HANDMADE_TILE_CPP

#include "handmade_coordinates.cpp"

internal tile_chunk*
GetTileChunk(tile_map* tileMap, tile_coordinates* coords)
{
  int32_point tileChunkCoords = GetTileChunkCoordinates(tileMap, coords);

  // TODO(Cristian): Find a good way of finding if the tileChunk actually exists!
  if((tileChunkCoords.x >= 0 && tileChunkCoords.x < tileMap->tileChunkCountX) &&
     (tileChunkCoords.y >= 0 && tileChunkCoords.y < tileMap->tileChunkCountY))
  {
    tile_chunk* result = tileMap->tileChunks +
                         (tileChunkCoords.y * tileMap->tileChunkCountX) +
                         tileChunkCoords.x;
    return result;
  }

  return nullptr;
}

internal uint32*
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
internal uint32
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

internal void
SetTileValue(tile_map* tileMap, tile_chunk* tileChunk, int32 tileX, int32 tileY, uint32 value)
{
  ASSERT(tileMap);
  ASSERT(tileChunk);
  ASSERT(tileX >= 0 && tileX < tileMap->tileMax);
  ASSERT(tileX >= 0 && tileY < tileMap->tileMax);

  tileChunk->tiles[(tileY * tileMap->tileMax) + tileX] = value;
}



internal void
SetTileValue(tile_map* tileMap, tile_coordinates* coords, uint32 value)
{
  tile_chunk* tileChunk = GetTileChunk(tileMap, coords);

  // TODO(Cristian): On-demand tile_chunk creation
  ASSERT(tileChunk != nullptr);

  int32_point tileCoords = GetTileCoordinates(tileMap, coords);
  SetTileValue(tileMap, tileChunk, tileCoords.x, tileCoords.y, value);
}

#define _HANDMADE_TILE_CPP
#endif
