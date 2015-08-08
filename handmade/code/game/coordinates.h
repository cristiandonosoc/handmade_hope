#ifndef _HANDMADE_COORDINATES_H

struct tile_coordinates
{
  // NOTE(Cristian): Represent the global (obsolute) position of the tile
  // in the world
  MATH::vector3D<int32> tile;

  // Movement (in world) meters from the referenced tile ny (tileX, tileY)
  // If the coordinates are normalized, both pX and pY should be
  // 0 <= pX, pY < tileInMeters
  real32 pX;
  real32 pY;
};

#define _HANDMADE_COORDINATES_H
#endif
