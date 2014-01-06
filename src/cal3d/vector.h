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

struct CalVector4;

class CAL3D_API CalVector {
public:
    float x, y, z;

    CalVector asCalVector() const {
        return *this;
    }

    CalVector4 asCalVector4() const;

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
        float l = length();
        if (l >= 0.0001f) {
            *this /= l;
        } else {
            if (x > y && x > z) {
                clear();
                x = 1.0f;
            } else if (y > z) {
                clear();
                y = 1.0f;
            } else {
                clear();
                z = 1.0f;
            }
        }
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

namespace cal3d {    
    inline void applyZUpToYUp(CalVector &v) {
        float temp = v.y;
        v.y = v.z;
        v.z = temp;
        v.z = -v.z;
    }
}

#define CHECK_CALVECTOR_CLOSE(v1, v2, tolerance)   CHECK_CLOSE(0.0f, ((v1)-(v2)).length(), tolerance)
