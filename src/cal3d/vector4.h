#pragma once

#include "cal3d/vector.h"

__declspec(align(16)) struct CalBase4
{
  float x, y, z, w;

  void set(float _x, float _y, float _z, float _w) {
    x = _x;
    y = _y;
    z = _z;
    w = _w;
  }

  void operator*=(float f) {
    x *= f;
    y *= f;
    z *= f;
  }

  CalVector asCalVector() const {
    return CalVector(x, y, z);
  }
};

struct CalVector4 : CalBase4 {
  CalVector4() {
    w = 0.0f;
  }

  __forceinline void setAsVector(const CalVector& r) {
    x = r.x;
    y = r.y;
    z = r.z;
    w = 0.0f;
  }
};

struct CalPoint4 : CalBase4 {
  CalPoint4() {
    w = 1.0f;
  }

  __forceinline void setAsPoint(const CalVector& r) {
    x = r.x;
    y = r.y;
    z = r.z;
    w = 1.0f;
  }
};
