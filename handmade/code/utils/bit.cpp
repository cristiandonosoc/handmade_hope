#ifndef _UTILS_BIT_CPP

#include "bit.h"

namespace UTILS
{
namespace BIT
{
  inline bit_scan_result
  FindLeastSignificantSetBit(uint32 mask)
  {

    bit_scan_result result = {};
#if COMPILER_MSVC
    result.found = _BitScanForward((unsigned long*)&result.index, mask);
#else

    for(uint32 shiftTest = 0;
        shiftTest < 32;
        shiftTest++)
    {
      if(mask & (1 << shiftTest))
      {
        result.found = true;
        result.index = shiftTest;
        break;
      }
    }
#endif

    return result;
  }

  inline uint32
  RotateLeft(uint32 value, int32 amount)
  {
    uint32 result = _rotl(value, amount);
    return result;
  }

  inline uint32
  RotateRight(uint32 value, int32 amount)
  {
    uint32 result = _rotr(value, amount);
    return result;
  }



}
}

#define _UTILS_BIT_CPP
#endif
