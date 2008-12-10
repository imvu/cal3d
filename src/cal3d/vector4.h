#pragma once

#include "cal3d/vector.h"

// for SSE
__declspec(align(16)) struct CalVector4
{
  float x, y, z, w;

  __forceinline void setAsVector(const CalVector& r) {
    x = r.x;
    y = r.y;
    z = r.z;
    w = 0.0f;
  }

  __forceinline void setAsPoint(const CalVector& r) {
    x = r.x;
    y = r.y;
    z = r.z;
    w = 1.0f;
  }

  void set(float _x, float _y, float _z, float _w) {
    x = _x;
    y = _y;
    z = _z;
    w = _w;
  }

  CalVector asCalVector() const {
    return CalVector(x, y, z);
  }

  void operator*=(float f) {
    x *= f;
    y *= f;
    z *= f;
  }
};
