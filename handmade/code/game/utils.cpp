/*  =====================================================================
    $File: utils.cpp
    $Creation Date: 2015-07-16
    $Last Modified: $
    $Revision: $
    $Creator: Cristián Donoso $
    $Notice: (c) Copyright 2015 Cristián Donoso $
    ===================================================================== */

#ifndef _GAME_UTILS_CPP

namespace UTILS
{

inline uint32
RoundReal32ToUInt32(real32 val)
{
  // TODO(Cristian): Intrinsic?
  // TODO(Cristian): Fix negative rounding
  return (uint32)(val + 0.5f);
}

inline uint32
TruncateReal32ToUInt32(real32 val)
{
  return (uint32)val;
}

inline int32
TruncateReal32ToInt32(real32 val)
{
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


}


#define _GAME_UTILS_CPP
#endif

