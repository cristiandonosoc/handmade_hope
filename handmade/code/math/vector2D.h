#ifndef _MATH_VECTOR2D_H

template<typename T>
struct v2
{
  T x;
  T y;

  T operator[](int index)
  {
    ASSERT(index >= 0);
    ASSERT(index < 2);
    return &(this->x)[index];
  }
};

template<typename T>
inline bool32
operator==(v2<T> a, v2<T> b)
{
  bool32 result = ((a.x == b.x) &&
                   (a.y == by));
  return result;
}

template<typename T>
inline bool32
operator!=(v2<T> a, v2<T> b)
{
  bool32 result = ((a.x != b.x) ||
                   (a.y != by));
  return result;
}

// + -
template<typename T>
inline v2<T>
operator+(v2<T> a, v2<T> b)
{
  v2<T> result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;

  return result;
}

template<typename T>
inline v2<T>&
operator+=(v2<T>& a, v2<T> b)
{
  a = a + b;
  return a;
}

template<typename T>
inline v2<T>
operator-(v2<T> a, v2<T> b)
{
  v2<T> result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;

  return result;
}

template<typename T>
inline v2<T>&
operator-=(v2<T>& a, v2<T> b)
{
  a = a - b;
  return a;
}

// Scalar
template<typename T>
inline v2<T>
operator*(v2<T> a, T b)
{
  v2<T> result;

  result.x = a.x * b;
  result.y = a.y * b;

  return result;
}

template<typename T>
inline v2<T>
operator*(T b, v2<T> a)
{
  v2<T> result = a * b;
  return result;
}

template<typename T>
inline v2<T>
operator*=(v2<T>& b, T a)
{
   b = a * b;
   return b;
}

template<typename T>
inline v2<T>
operator/(v2<T> a, real32 b)
{
  v2<T> result;

  result.x = a.x / b;
  result.y = a.y / b;

  return result;
}

template<typename T>
inline v2<T>
operator/(real32 b, v2<T> a)
{
  v2<T> result = a / b;
  return result;
}

template<typename T>
inline v2<T>
operator/=(v2<T>& b, real32 a)
{
   b = a / b;
   return b;
}

inline v2<real32>
NormalizeVector(v2<real32> v)
{
  real32 norm = SquareRoot(Square(v.x) + Square(v.y));
  if(norm == 0.0f) { return v; }
  v2<real32> result = norm / v;
  return result;
}

template<typename T>
inline T
InnerProduct(v2<T> a, v2<T> b)
{
  T result = (a.x * b.x) + (a.y * b.y);
  return result;
}

template<typename T>
inline T
LengthSq(v2<T> v)
{
  T result = InnerProduct(v, v);
  return result;
}

inline real32
Length(v2<real32> v)
{
  real32 result = SquareRoot(LengthSq(v));
  return result;
}

#define _MATH_VECTOR2D_H
#endif
