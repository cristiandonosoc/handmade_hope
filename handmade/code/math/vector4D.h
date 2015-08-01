#ifndef _MATH_VECTOR4D_H

template<typename T>
struct vector4D
{
  T x;
  T y;
  T z;
  T w;

  T operator[](int index)
  {
    ASSERT(index >= 0);
    ASSERT(index < 2);
    return &(this->x)[index];
  }
};

// + -
template<typename T>
inline vector4D<T>
operator+(vector4D<T> a, vector4D<T> b)
{
  vector4D<T> result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;
  result.w = a.w + b.w;

  return result;
}

template<typename T>
inline vector4D<T>&
operator+=(vector4D<T>& a, vector4D<T> b)
{
  a = a + b;
  return a;
}

template<typename T>
inline vector4D<T>
operator-(vector4D<T> a, vector4D<T> b)
{
  vector4D<T> result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;
  result.w = a.w - b.w;

  return result;
}

template<typename T>
inline vector4D<T>&
operator-=(vector4D<T>& a, vector4D<T> b)
{
  a = a - b;
  return a;
}

// Scalar
template<typename T>
inline vector4D<T>
operator*(vector4D<T> a, T b)
{
  vector4D<T> result;

  result.x = a.x * b;
  result.y = a.y * b;
  result.z = a.z * b;
  result.w = a.w * b;

  return result;
}

template<typename T>
inline vector4D<T>
operator*(T b, vector4D<T> a)
{
  vector4D<T> result = a * b;
  return result;
}

template<typename T>
inline vector4D<T>
operator*=(vector4D<T>& b, T a)
{
   b = a * b;
   return b;
}

template<typename T>
inline vector4D<T>
operator/(vector4D<T> a, T b)
{
  vector4D<T> result;

  result.x = a.x / b;
  result.y = a.y / b;
  result.z = a.z / b;
  result.w = a.w / b;

  return result;
}

template<typename T>
inline vector4D<T>
operator/(T b, vector4D<T> a)
{
  vector4D<T> result = a / b;
  return result;
}

template<typename T>
inline vector4D<T>
operator/=(vector4D<T>& b, T a)
{
   b = a / b;
   return b;
}

#define _MATH_VECTOR4D_H
#endif
