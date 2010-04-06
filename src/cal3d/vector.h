//****************************************************************************//
// vector.h                                                                   //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <assert.h>
#include <math.h>
#include "cal3d/global.h"

class CalQuaternion;

class CAL3D_API CalVector : public Cal::Object
{
public:
  float x ,y ,z;

  CalVector asCalVector() const {
    return *this;
  }

  void setAsPoint(const CalVector& v) {
    *this = v;
  }

  void setAsVector(const CalVector& v) {
    *this = v;
  }

  inline CalVector(): x(0.0f), y(0.0f), z(0.0f) {};
  inline CalVector(const CalVector& v) : x(v.x), y(v.y), z(v.z) {};
  inline CalVector(float vx, float vy, float vz): x(vx), y(vy), z(vz) {};
  inline ~CalVector() {};

  inline float& operator[](unsigned int i) 
  {
	  return (&x)[i];
  }

  inline const float& operator[](unsigned int i) const
  {
	  return (&x)[i];
  }

  CAL3D_FORCEINLINE void operator+=(const CalVector& v)
  {
	  x += v.x;
	  y += v.y;
	  z += v.z;
  }
  
  
  inline void operator-=(const CalVector& v)
  {
	  x -= v.x;
	  y -= v.y;
	  z -= v.z;
  }

  inline void operator*=(float d)
  {
	  x *= d;
	  y *= d;
	  z *= d;
  }

  void operator*=(const CalQuaternion& q);

  inline void operator/=(float d)
  {
    assert(d != 0);
    float m = 1.0f / d;
    x *= m;
    y *= m;
    z *= m;
  }

  inline bool operator==(const CalVector& v) const
  {
	  return ((x == v.x) && (y == v.y) && (z == v.z));
  }

  inline void blend(float d, const CalVector& v)
  {
	  x += d * (v.x - x);
	  y += d * (v.y - y);
	  z += d * (v.z - z);
  }

  inline void clear() 
  {
	  x=0.0f;
	  y=0.0f;
	  z=0.0f;		  
  }

  inline float lengthSquared() const
  {
      return (float)(x * x + y * y + z * z);
  }
  inline float length() const
  {
      return (float)sqrt(lengthSquared());
  }
  inline float normalize()
  {
    // calculate the length of the vector
    float length = (float) sqrt(x * x + y * y + z * z);
    *this /= length;
    return length;
  }
  
  void set(float vx, float vy, float vz)
  {
	  x = vx;
	  y = vy;
	  z = vz;
  }

};

static inline CalVector operator+(const CalVector& v, const CalVector& u)
{
  return CalVector(v.x + u.x, v.y + u.y, v.z + u.z);
}

static inline CalVector operator-(const CalVector& v, const CalVector& u)
{
	return CalVector(v.x - u.x, v.y - u.y, v.z - u.z);
}

static inline CalVector operator*(const CalVector& v, const float d)
{
	return CalVector(v.x * d, v.y * d, v.z * d);
}

static inline CalVector operator*(const float d, const CalVector& v)
{
	return CalVector(v.x * d, v.y * d, v.z * d);
}

static inline CalVector operator/(const CalVector& v, const float d)
{
	return CalVector(v.x / d, v.y / d, v.z / d);
}

static inline float operator*(const CalVector& v, const CalVector& u)
{
	return v.x * u.x + v.y * u.y + v.z * u.z;
}  

static inline CalVector operator%(const CalVector& v, const CalVector& u)
{
	return CalVector(v.y * u.z - v.z * u.y, v.z * u.x - v.x * u.z, v.x * u.y - v.y * u.x);
}
