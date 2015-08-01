#ifndef _MATH_VECTOR2D_H

template<typename T>
struct vector2D
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

// + -
template<typename T>
inline vector2D<T>
operator+(vector2D<T> a, vector2D<T> b)
{
  vector2D<T> result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;

  return result;
}

template<typename T>
inline vector2D<T>&
operator+=(vector2D<T>& a, vector2D<T> b)
{
  a = a + b;
  return a;
}

template<typename T>
inline vector2D<T>
operator-(vector2D<T> a, vector2D<T> b)
{
  vector2D<T> result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;

  return result;
}

template<typename T>
inline vector2D<T>&
operator-=(vector2D<T>& a, vector2D<T> b)
{
  a = a - b;
  return a;
}

// Scalar
template<typename T>
inline vector2D<T>
operator*(vector2D<T> a, T b)
{
  vector2D<T> result;

  result.x = a.x * b;
  result.y = a.y * b;

  return result;
}

template<typename T>
inline vector2D<T>
operator*(T b, vector2D<T> a)
{
  vector2D<T> result = a * b;
  return result;
}

template<typename T>
inline vector2D<T>
operator*=(vector2D<T>& b, T a)
{
   b = a * b;
   return b;
}

template<typename T>
inline vector2D<T>
operator/(vector2D<T> a, real32 b)
{
  vector2D<T> result;

  result.x = a.x / b;
  result.y = a.y / b;

  return result;
}

template<typename T>
inline vector2D<T>
operator/(real32 b, vector2D<T> a)
{
  vector2D<T> result = a / b;
  return result;
}

template<typename T>
inline vector2D<T>
operator/=(vector2D<T>& b, real32 a)
{
   b = a / b;
   return b;
}

inline vector2D<real32>
NormalizeVector(vector2D<real32> v)
{
  real32 norm = sqrt(Square(v.x) + Square(v.y));
  if(norm == 0.0f) { return v; }
  vector2D<real32> result = norm / v;
  return result;
}



#define _MATH_VECTOR2D_H
#endif
