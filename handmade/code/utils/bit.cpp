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

    return result;
  }

}
}

#define _UTILS_BIT_CPP
#endif
