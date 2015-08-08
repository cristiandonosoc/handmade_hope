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

internal tile_chunk*
GetTileChunk(tile_map* tileMap, int32 x, int32 y, int32 z)
{
  vector3D<int32> tileChunkCoords = GetTileChunkCoordinates(tileMap, x, y, z);

  // TODO(Cristian): Find a good way of finding if the tileChunk actually exists!
  if((tileChunkCoords.x >= 0 && tileChunkCoords.x < tileMap->tileChunkCountX) &&
     (tileChunkCoords.y >= 0 && tileChunkCoords.y < tileMap->tileChunkCountY) &&
     (tileChunkCoords.z >= 0 && tileChunkCoords.z < tileMap->tileChunkCountZ))

  {
    tile_chunk* result =
      tileMap->tileChunks +
      (tileChunkCoords.z * tileMap->tileChunkCountY * tileMap->tileChunkCountX) +
      (tileChunkCoords.y * tileMap->tileChunkCountX) +
      tileChunkCoords.x;
    return result;
  }

  return nullptr;
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
  // ASSERT(x >= 0 && x < tileMap->tileChunkCountX * tileMap->tileSide);
  // ASSERT(y >= 0 && y < tileMap->tileChunkCountY * tileMap->tileSide);
  // ASSERT(z >= 0 && z < tileMap->tileChunkCountZ * tileMap->tileSide);

  vector2D<int32> tileCoords = GetTileCoordinates(tileMap, x, y);

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
  ASSERT(tileChunk != nullptr);

  // If the tile_chunk is not initialized, we require the memory for it
  if(!tileChunk->initialized)
  {
    tileChunk->tiles = PushArray(memoryManager,
                                 GetTileChunkDim(tileMap),
                                 uint32);
    tileChunk->initialized = true;
  }

  vector2D<int32> tileCoords = GetTileCoordinates(tileMap, coords);
  SetTileValue(tileMap, tileChunk, tileCoords.x, tileCoords.y, value);
}

#define _GAME_TILES_CPP
#endif
