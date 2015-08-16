#ifndef _MATH_VECTOR3D_H

template<typename T>
struct v3
{
  T x;
  T y;
  T z;

  T operator[](int index)
  {
    ASSERT(index >= 0);
    ASSERT(index < 3);
    return &(this->x)[index];
  }
};

template<typename T>
inline bool32
operator==(v3<T> a, v3<T> b)
{
  bool32 result = ((a.x == b.x) &&
                   (a.y == b.y) &&
                   (a.z == b.z));
  return result;
}

template<typename T>
inline bool32
operator!=(v3<T> a, v3<T> b)
{
  bool32 result = ((a.x != b.x) ||
                   (a.y != b.y) ||
                   (a.z != b.z));
  return result;
}



// + -
template<typename T>
inline v3<T>
operator+(v3<T> a, v3<T> b)
{
  v3<T> result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;

  return result;
}

template<typename T>
inline v3<T>&
operator+=(v3<T>& a, v3<T> b)
{
  a = a + b;
  return a;
}

template<typename T>
inline v3<T>
operator-(v3<T> a, v3<T> b)
{
  v3<T> result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;

  return result;
}

template<typename T>
inline v3<T>&
operator-=(v3<T>& a, v3<T> b)
{
  a = a - b;
  return a;
}

// Scalar
template<typename T>
inline v3<T>
operator*(v3<T> a, T b)
{
  v3<T> result;

  result.x = a.x * b;
  result.y = a.y * b;
  result.z = a.z * b;

  return result;
}

template<typename T>
inline v3<T>
operator*(T b, v3<T> a)
{
  v3<T> result = a * b;
  return result;
}

template<typename T>
inline v3<T>
operator*=(v3<T>& b, T a)
{
   b = a * b;
   return b;
}

template<typename T>
inline v3<T>
operator/(v3<T> a, T b)
{
  v3<T> result;

  result.x = a.x / b;
  result.y = a.y / b;
  result.z = a.z / b;

  return result;
}

template<typename T>
inline v3<T>
operator/(T b, v3<T> a)
{
  v3<T> result = a / b;
  return result;
}

template<typename T>
inline v3<T>
operator/=(v3<T>& b, T a)
{
   b = a / b;
   return b;
}


inline v3<real32>
NormalizeVector(v3<real32> v)
{
  real32 norm = sqrt(Square(v.x) +
                     Square(v.y) +
                     Square(v.z));
  if(norm == 0.0f) { return v; }
  v3<real32> result = norm / v;
  return result;
}

template<typename T>
inline T
InnerProduct(v3<T> a, v3<T> b)
{
  T result = (a.x * b.x) + (a.y * b.y) + (a.z + b.z);
  return result;
}





#define _MATH_VECTOR3D_H
#endif
