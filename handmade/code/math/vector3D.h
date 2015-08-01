#ifndef _MATH_VECTOR3D_H

template<typename T>
struct vector3D
{
  T x;
  T y;
  T z;

  T operator[](int index)
  {
    ASSERT(index >= 0);
    ASSERT(index < 2);
    return &(this->x)[index];
  }
};

// + -
template<typename T>
inline vector3D<T>
operator+(vector3D<T> a, vector3D<T> b)
{
  vector3D<T> result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;

  return result;
}

template<typename T>
inline vector3D<T>&
operator+=(vector3D<T>& a, vector3D<T> b)
{
  a = a + b;
  return a;
}

template<typename T>
inline vector3D<T>
operator-(vector3D<T> a, vector3D<T> b)
{
  vector3D<T> result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;

  return result;
}

template<typename T>
inline vector3D<T>&
operator-=(vector3D<T>& a, vector3D<T> b)
{
  a = a - b;
  return a;
}

// Scalar
template<typename T>
inline vector3D<T>
operator*(vector3D<T> a, T b)
{
  vector3D<T> result;

  result.x = a.x * b;
  result.y = a.y * b;
  result.z = a.z * b;

  return result;
}

template<typename T>
inline vector3D<T>
operator*(T b, vector3D<T> a)
{
  vector3D<T> result = a * b;
  return result;
}

template<typename T>
inline vector3D<T>
operator*=(vector3D<T>& b, T a)
{
   b = a * b;
   return b;
}

template<typename T>
inline vector3D<T>
operator/(vector3D<T> a, T b)
{
  vector3D<T> result;

  result.x = a.x / b;
  result.y = a.y / b;
  result.z = a.z / b;

  return result;
}

template<typename T>
inline vector3D<T>
operator/(T b, vector3D<T> a)
{
  vector3D<T> result = a / b;
  return result;
}

template<typename T>
inline vector3D<T>
operator/=(vector3D<T>& b, T a)
{
   b = a / b;
   return b;
}

#define _MATH_VECTOR3D_H
#endif
