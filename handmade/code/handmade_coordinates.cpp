#ifndef _HANDMADE_COORDINATES_CPP

#include "game\utils.cpp"

/**
 * Modifies the coordinates so that pX and pY, which represent the offset from the tile,
 * are within the tile bounds. If they're not, we must move the tile into the correct offset.
 */
internal void
NormalizeCoordinates(tile_map* tileMap, tile_coordinates* coords)
{
  // We normalize the real coordinates of the point within the tile
  real32 divX = UTILS::FloorReal32ToInt32(coords->pX / tileMap->tileInMeters);
  real32 divY = UTILS::FloorReal32ToInt32(coords->pY / tileMap->tileInMeters);
  // We move the tile offset
  coords->tileX += divX;
  coords->tileY += divY;

  // We correct the correct offset
  coords->pX -= divX * tileMap->tileInMeters;
  coords->pY -= divY * tileMap->tileInMeters;
}

internal point2D<int32>
GetTileCoordinates(tile_map* tileMap, tile_coordinates* coords)
{
  point2D<int32> point = {};
  point.x = coords->tileX & tileMap->tileMask;
  point.y = coords->tileY & tileMap->tileMask;

  return point;
}

internal point2D<int32>
GetTileChunkCoordinates(tile_map* tileMap, tile_coordinates* coords)
{
  point2D<int32> point = {};
  point.x = coords->tileX >> tileMap->tileShift;
  point.y = coords->tileY >> tileMap->tileShift;

  return point;
}

internal tile_coordinates
ModifyCoordinates(tile_map* tileMap, tile_coordinates coords, real32 dX, real32 dY)
{
  coords.pX += dX;
  coords.pY += dY;

  NormalizeCoordinates(tileMap, &coords);

  return coords;
}

#define _HANDMADE_COORDINATES_CPP
#endif
