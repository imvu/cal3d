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

class CAL3D_API CalVector {
public:
    float x, y, z;

    CalVector asCalVector() const {
        return *this;
    }

    void setAsPoint(const CalVector& v) {
        *this = v;
    }

    void setAsVector(const CalVector& v) {
        *this = v;
    }

    CalVector(): x(0.0f), y(0.0f), z(0.0f) {};
    CalVector(const CalVector& v) : x(v.x), y(v.y), z(v.z) {};
    CalVector(float vx, float vy, float vz): x(vx), y(vy), z(vz) {};

    CalVector operator-() const {
        return CalVector(-x, -y, -z);
    }
    
    CAL3D_FORCEINLINE void operator+=(const CalVector& v) {
        x += v.x;
        y += v.y;
        z += v.z;
    }


    void operator-=(const CalVector& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }

    void operator*=(float d) {
        x *= d;
        y *= d;
        z *= d;
    }

    void operator*=(const CalVector& v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
    }

    void operator/=(float d) {
        assert(d != 0);
        float m = 1.0f / d;
        x *= m;
        y *= m;
        z *= m;
    }

    bool operator==(const CalVector& v) const {
        return (cal3d::close(x, v.x) && cal3d::close(y, v.y) && cal3d::close(z, v.z));
    }
    bool operator!=(const CalVector& v) const {
        return !(*this == v);
    }

    void clear() {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    float lengthSquared() const {
        return (float)(x * x + y * y + z * z);
    }
    float length() const {
        return sqrtf(lengthSquared());
    }
    void normalize() {
        *this /= length();
    }

    void set(float vx, float vy, float vz) {
        x = vx;
        y = vy;
        z = vz;
    }
};

inline bool exactlyEqual(const CalVector& lhs, const CalVector& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

inline CalVector operator+(const CalVector& v, const CalVector& u) {
    return CalVector(v.x + u.x, v.y + u.y, v.z + u.z);
}

inline CalVector operator-(const CalVector& v, const CalVector& u) {
    return CalVector(v.x - u.x, v.y - u.y, v.z - u.z);
}

inline CalVector operator*(const CalVector& v, const CalVector& u) {
    return CalVector(v.x * u.x, v.y * u.y, v.z * u.z);
}

inline CalVector operator*(const CalVector& v, float d) {
    return CalVector(v.x * d, v.y * d, v.z * d);
}

inline CalVector operator*(float d, const CalVector& v) {
    return CalVector(v.x * d, v.y * d, v.z * d);
}

inline CalVector operator/(const CalVector& v, float d) {
    return CalVector(v.x / d, v.y / d, v.z / d);
}

inline float dot(const CalVector& v, const CalVector& u) {
    return v.x * u.x + v.y * u.y + v.z * u.z;
}

inline CalVector cross(const CalVector& a, const CalVector& u) {
    return CalVector(a.y * u.z - a.z * u.y, a.z * u.x - a.x * u.z, a.x * u.y - a.y * u.x);
}

inline CalVector lerp(float f, CalVector left, CalVector right) {
    return (1 - f) * left + f * right;
}
