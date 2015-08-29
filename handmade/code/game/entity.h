#ifndef _GAME_ENTITY_H

#include "game/coordinates.h"

enum entity_type
{
  player,
  wall
};

enum entity_residence
{
  cold,
  hot
};

struct tile_chunk; // Forward Declaration
struct entity_def
{
  bool32 exists;
  entity_type type;
  entity_residence residence;
  tile_coordinates pos;
  tile_chunk* tileChunk;

  MATH::v2<real32> dPos;
  MATH::v2<real32> hotPos;
  // LBRU
  uint32 facingDirection;

  real32 width;
  real32 height;
};


#define  _GAME_ENTITY_H
#endif
