#ifndef _GAME_TILES_CPP

#include "tiles.h"
#include "coordinates.cpp"
#include "game/memory.cpp"
#include "math/vector.h"

using namespace MATH;

inline uint32
GetTileChunkDim(tile_map* tileMap)
{
  uint32 result = tileMap->tileSide * tileMap->tileSide;
  return result;
}

inline uint32
TileChunkHashSlot(int32 x, int32 y, int32 z)
{
  // TODO(Cristian): BETTER HASH FUNCTION :)
  uint32 result = 19*z + 5*y + 11*x;
  result %= TILE_CHUNK_HASH_SIZE;
  return result;
}

internal tile_chunk*
GetTileChunk(tile_map* tileMap, int32 x, int32 y, int32 z)
{
  v3<int32> tileChunkCoords = GetTileChunkCoordinates(tileMap, x, y, z);

  uint32 hashSlot = TileChunkHashSlot(x, y, z);
  tile_chunk** tileChunk = tileMap->tileChunkHash + hashSlot;
  while(*tileChunk)
  {
    if((*tileChunk)->tileChunkCoords == tileChunkCoords) { break; }
    tileChunk = &((*tileChunk)->nextTileChunkInHash);
  }

  tile_chunk* result = *tileChunk;
  return result;
}

internal tile_chunk*
GetTileChunk(tile_map* tileMap, tile_coordinates* coords)
{
  auto result = GetTileChunk(tileMap, coords->tile.x,
                                      coords->tile.y,
                                      coords->tile.z);
  return result;
}

internal uint32*
GetTile(tile_map* tileMap, int32 x, int32 y, int32 z)
{
  v2<int32> tileCoords = GetTileCoordinates(tileMap, x, y);

  ASSERT(tileCoords.x >= 0 && tileCoords.x < tileMap->tileSide);
  ASSERT(tileCoords.y >= 0 && tileCoords.y < tileMap->tileSide);

  uint32* result = nullptr;

  tile_chunk* tileChunk = GetTileChunk(tileMap, x, y, z);
  if(tileChunk != nullptr && tileChunk->initialized)
  {
    result = tileChunk->tiles + (tileCoords.y * tileMap->tileSide) + tileCoords.x;
  }
  return result;

}

internal uint32*
GetTile(tile_map* tileMap, tile_coordinates* coords)
{
  auto result = GetTile(tileMap, coords->tile.x, coords->tile.y, coords->tile.z);
  return result;
}


#define TILE_INVALID 0xFFFFFFFF
internal uint32
GetTileValue(tile_map* tileMap, int32 x, int32 y, int32 z)
{
  uint32 result = TILE_INVALID;

  uint32* tile = GetTile(tileMap, x, y, z);
  if(tile != nullptr) { result = *tile; }

  return result;
}

internal uint32
GetTileValue(tile_map* tileMap, tile_coordinates* coords)
{
  auto result = GetTileValue(tileMap, coords->tile.x, coords->tile.y, coords->tile.z);
  return result;
}

internal void
SetTileValue(tile_map* tileMap, tile_chunk* tileChunk, int32 tileX, int32 tileY, uint32 value)
{
  ASSERT(tileMap);
  ASSERT(tileChunk);
  ASSERT(tileX >= 0 && tileX < tileMap->tileSide);
  ASSERT(tileX >= 0 && tileY < tileMap->tileSide);

  tileChunk->tiles[(tileY * tileMap->tileSide) + tileX] = value;
}



internal void
SetTileValue(memory_manager* memoryManager, tile_map* tileMap, tile_coordinates* coords, uint32 value)
{
  tile_chunk* tileChunk = GetTileChunk(tileMap, coords);

  // TODO(Cristian): On-demand tile_chunk creation
  if(!tileChunk)
  {
    uint32 hashSlot = TileChunkHashSlot(coords->tile.x, coords->tile.y, coords->tile.z);

    tile_chunk** tileChunkPtr = tileMap->tileChunkHash + hashSlot;
    while(*tileChunkPtr)
    {
      tileChunk = &(*tileChunk->nextTileChunkInHash);
    }

    *tileChunkPtr = PushStruct(memoryManager, tile_chunk);
    tileChunk = *tileChunkPtr;
    tileChunk->tiles = PushArray(memoryManager, GetTileChunkDim(tileMap), uint32);
    tileChunk->tileChunkCoords = GetTileChunkCoordinates(tileMap, coords);
    tileChunk->initialized = true;

  }

  v2<int32> tileCoords = GetTileCoordinates(tileMap, coords);
  SetTileValue(tileMap, tileChunk, tileCoords.x, tileCoords.y, value);
}

#define _GAME_TILES_CPP
#endif
