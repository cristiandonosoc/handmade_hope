#ifndef _MATH_RECTANGLE_H

#include "math/vector.h"

template<typename T>
struct rectangle2D
{
  v2<T> min;
  v2<T> max;
};

template<typename T>
inline bool32
IsWithinRectangle2D(T minX, T minY, T maxX, T maxY, T pX, T pY)
{
  bool32 result = true;
  if(pX < minX || pX > maxX ||
     pY < minY || pY > maxY)
  {
    result = false;
  }

  return result;
}

template<typename T>
inline bool32
IsWithinRectangle2D(rectangle2D<T> rect, v2<T> point)
{
  bool32 result = IsWithinRectangle(rect.min.x, rect.min.y,
                                    rect.max.x, rect.max.y,
                                    point.x, point.y);
  return result;
}

#define _MATH_RECTANGLE_H
#endif
