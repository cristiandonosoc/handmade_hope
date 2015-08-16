#ifndef _MATH_VECTOR4D_H

template<typename T>
struct v4
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

template<typename T>
inline bool32
operator==(v4<T> a, v4<T> b)
{
  bool32 result = ((a.x == b.x) &&
                   (a.y == b.y) &&
                   (a.z == b.z) &&
                   (a.w == b.w));
  return result;
}

template<typename T>
inline bool32
operator!=(v4<T> a, v4<T> b)
{
  bool32 result = ((a.x != b.x) ||
                   (a.y != b.y) ||
                   (a.z != b.z) ||
                   (a.w != b.w));
  return result;
}



// + -
template<typename T>
inline v4<T>
operator+(v4<T> a, v4<T> b)
{
  v4<T> result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;
  result.w = a.w + b.w;

  return result;
}

template<typename T>
inline v4<T>&
operator+=(v4<T>& a, v4<T> b)
{
  a = a + b;
  return a;
}

template<typename T>
inline v4<T>
operator-(v4<T> a, v4<T> b)
{
  v4<T> result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;
  result.w = a.w - b.w;

  return result;
}

template<typename T>
inline v4<T>&
operator-=(v4<T>& a, v4<T> b)
{
  a = a - b;
  return a;
}

// Scalar
template<typename T>
inline v4<T>
operator*(v4<T> a, T b)
{
  v4<T> result;

  result.x = a.x * b;
  result.y = a.y * b;
  result.z = a.z * b;
  result.w = a.w * b;

  return result;
}

template<typename T>
inline v4<T>
operator*(T b, v4<T> a)
{
  v4<T> result = a * b;
  return result;
}

template<typename T>
inline v4<T>
operator*=(v4<T>& b, T a)
{
   b = a * b;
   return b;
}

template<typename T>
inline v4<T>
operator/(v4<T> a, T b)
{
  v4<T> result;

  result.x = a.x / b;
  result.y = a.y / b;
  result.z = a.z / b;
  result.w = a.w / b;

  return result;
}

template<typename T>
inline v4<T>
operator/(T b, v4<T> a)
{
  v4<T> result = a / b;
  return result;
}

template<typename T>
inline v4<T>
operator/=(v4<T>& b, T a)
{
   b = a / b;
   return b;
}


inline v4<real32>
NormalizeVector(v4<real32> v)
{
  real32 norm = sqrt(Square(v.x) +
                     Square(v.y) +
                     Square(v.z) +
                     Square(v.w));
  if(norm == 0.0f) { return v; }
  v4<real32> result = norm / v;
  return result;
}


template<typename T>
inline T
InnerProduct(v4<T> a, v4<T> b)
{
  T result = (a.x * b.x) + (a.y * b.y) +
             (a.z + b.z) + (a.w * b.w);
  return result;
}





#define _MATH_VECTOR4D_H
#endif
