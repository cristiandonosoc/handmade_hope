#ifndef _HANDMADE_COORDINATES_CPP

#include "tiles.h"
#include "utils/float.cpp"
#include "math/vector.h"
#include "utils/float.cpp"

using namespace MATH;
/**
 * Modifies the coordinates so that pX and pY, which represent the offset from the tile,
 * are within the tile bounds. If they're not, we must move the tile into the correct offset.
 */
internal void
NormalizeCoordinates(const tile_map& tileMap, tile_coordinates* coords)
{
  // We normalize the real coordinates of the point within the tile
  v3<int32> div = {UTILS::FLOAT::FloorReal32ToInt32(coords->pX / tileMap.tileInMeters),
                   UTILS::FLOAT::FloorReal32ToInt32(coords->pY / tileMap.tileInMeters)};
  // We move the tile offset
  coords->tile += div;

  // We correct the correct offset
  coords->pX -= div.x * tileMap.tileInMeters;
  coords->pY -= div.y * tileMap.tileInMeters;
}

/**
 * Gets the coordinates of a tile relative to it's a tile_chunk
 */
internal v2<int32>
GetTileCoordinates(const tile_map& tileMap, int32 x, int32 y)
{
  v2<int32> point = {};
  point.x = x & tileMap.tileMask;
  point.y = y & tileMap.tileMask;

  return point;
}

internal v2<int32>
GetTileCoordinates(const tile_map& tileMap, tile_coordinates* coords)
{
  auto result = GetTileCoordinates(tileMap, coords->tile.x, coords->tile.y);
  return result;
}

internal v3<int32>
GetTileChunkCoordinates(const tile_map& tileMap, int32 x, int32 y, int32 z)
{
  v3<int32> point = {};
  point.x = x >> tileMap.tileShift;
  point.y = y >> tileMap.tileShift;
  point.z = z;

  return point;
}

internal v3<int32>
GetTileChunkCoordinates(const tile_map& tileMap, const tile_coordinates& coords)
{
  auto result = GetTileChunkCoordinates(tileMap, coords.tile.x,
                                                 coords.tile.y,
                                                 coords.tile.z);
  return result;
}

internal tile_coordinates
ModifyCoordinates(const tile_map& tileMap, tile_coordinates coords, real32 dX, real32 dY)
{
  coords.pX += dX;
  coords.pY += dY;

  NormalizeCoordinates(tileMap, &coords);

  return coords;
}

inline v2<real32>
Distance(const tile_map& tileMap, tile_coordinates a, tile_coordinates b)
{
  ASSERT(a.tile.z == b.tile.z);

  v2<real32> tileV = {(b.tile.x - a.tile.x) * tileMap.tileInMeters,
                      (b.tile.y - a.tile.y) * tileMap.tileInMeters};
  v2<real32> tileR = {(b.pX - a.pX),
                      (b.pY - a.pY)};
  auto result = tileV + tileR;
  return result;
}

inline tile_coordinates
GenerateCoords(int32 x, int32 y, int32 z)
{
  tile_coordinates result = {x, y, z};
  return result;
}

#define _HANDMADE_COORDINATES_CPP
#endif
