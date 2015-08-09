/*  =====================================================================
    $File: utils.cpp
    $Creation Date: 2015-07-16
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _UTILS_FLOAT_CPP

#include "common_types.h"

namespace UTILS
{
namespace FLOAT
{

inline real32
AbsoluteReal32(real32 val)
{
  real32 result = fabs(val);
  return result;
}

inline uint32
RoundReal32ToUInt32(real32 val)
{
  // TODO(Cristian): Intrinsic?
  // TODO(Cristian): Fix negative rounding
  real32 offset = 0.5f;
  if(val < 0.0f) { offset = -0.5f; }
  return (uint32)(val + offset);
}

inline int32
CeilReal32ToInt32(real32 val)
{
  int32 result = (int32)ceilf(val);
  return result;
}

inline uint32
FloorReal32ToUInt32(real32 val)
{
  ASSERT(val >= 0.0f);
  return (uint32)val;
}

inline int32
FloorReal32ToInt32(real32 val)
{
   if(val < 0.0f) { val -= 1.0f; }
  return (int32)val;
}

inline uint32
RealRGBToUInt32(real32 R, real32 G, real32 B)
{
  // NOTE(Cristian): Color bit pattern 0xAARRGGBB
  // NOTE(Cristian): As each color byte is separate
  //                 we can use bitwise OR to join them
  return ((RoundReal32ToUInt32(R * 255.0f) << 16) |
          (RoundReal32ToUInt32(G * 255.0f) << 8) |
          (RoundReal32ToUInt32(B * 255.0f) << 0));
}


} // namespace FLOAT
} // namespace UTILS

#define _UTILS_FLOAT_CPP
#endif

