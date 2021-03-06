#ifndef _MATH_COMMON_H

#define MIN(a, b) ((a) < (b)) ? (a) : (b)
#define MAX(a, b) ((a) > (b)) ? (a) : (b)

template<typename T>
inline T
Square(T x)
{
  T result = x * x;
  return result;
}

inline real32
SquareRoot(real32 x)
{
  real32 result = sqrtf(x);
  return result;
}

#define _MATH_COMMON_H
#endif
