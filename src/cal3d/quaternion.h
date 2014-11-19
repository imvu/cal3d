//****************************************************************************//
// quaternion.h                                                               //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"
#include "cal3d/vector4.h"
#include "cal3d/matrix.h"


class CAL3D_API CalQuaternion {
public:
    float x;
    float y;
    float z;
    float w;

    inline CalQuaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {};
    inline CalQuaternion(const CalQuaternion& q): x(q.x), y(q.y), z(q.z), w(q.w) {};
    inline CalQuaternion(float qx, float qy, float qz, float qw): x(qx), y(qy), z(qz), w(qw) {};
    CalQuaternion(CalMatrix m);

    CalVector4 asCalVector4() const {
        return CalVector4(x, y, z, w);
    }

    inline void operator=(const CalQuaternion& q) {
        x = q.x;
        y = q.y;
        z = q.z;
        w = q.w;
    }

    inline void clear() {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 1.0f;
    }
    inline void conjugate() {
        x = -x;
        y = -y;
        z = -z;
    }

    inline void invert() {
        conjugate();
        const float norm = (x * x) + (y * y) + (z * z) + (w * w);

        if (norm == 0.0f) {
            return;
        }

        const float inv_norm = 1 / norm;
        x *= inv_norm;
        y *= inv_norm;
        z *= inv_norm;
        w *= inv_norm;
    }

    inline CalQuaternion operator-() const {
        CalQuaternion rv = *this;
        rv.invert();
        return rv;
    }

    void setAxisAngle(const CalVector& axis, float angle) {
        float h2 = angle * 0.5f;
        float s = sin(h2);
        w = cos(h2); 
        x = s * axis.x;
        y = s * axis.y;
        z = s * axis.z;
    }
};

inline CalQuaternion operator*(const CalQuaternion& outer, const CalQuaternion& inner) {
    float qx = outer.x;
    float qy = outer.y;
    float qz = outer.z;
    float qw = outer.w;

    return CalQuaternion(
        qw * inner.x + qx * inner.w + qy * inner.z - qz * inner.y,
        qw * inner.y - qx * inner.z + qy * inner.w + qz * inner.x,
        qw * inner.z + qx * inner.y - qy * inner.x + qz * inner.w,
        qw * inner.w - qx * inner.x - qy * inner.y - qz * inner.z);
}

// Implicitly convert vector to quaternion (w=0), multiply times q
inline CalQuaternion operator*(const CalVector& v, const CalQuaternion& q) {
    CalQuaternion quatV(v.x, v.y, v.z, 0.0f);
    return quatV * q;
}

// Implicitly convert vector to quaternion (w=0), multiply times q
inline CalQuaternion operator*(const CalVector4& v, const CalQuaternion& q) {
    CalQuaternion quatV(v.x, v.y, v.z, v.w);
    return quatV * q;
}

// Implicitly convert point to quaternion (w=0), multiply times q
inline CalQuaternion operator*(const CalPoint4& p, const CalQuaternion& q) {
    CalQuaternion quatP(p.x, p.y, p.z, p.w);
    return quatP * q;
}

// left-multiply, rotate v by quaternion and produce rotated vector
inline CalVector operator*(const CalQuaternion& q, const CalVector& v) {
    CalQuaternion temp = v * CalQuaternion(-q.x, -q.y, -q.z, q.w);
    temp = q * temp;

    return CalVector(
        temp.x,
        temp.y,
        temp.z);
}

// left-multiply, rotate v by quaternion and produce rotated vector
inline CalVector4 operator*(const CalQuaternion& q, const CalVector4& v) {
    CalQuaternion temp = v * CalQuaternion(-q.x, -q.y, -q.z, q.w);
    temp = q * temp;

    return CalVector4(
        temp.x,
        temp.y,
        temp.z,
        temp.w);
}

inline CalPoint4 operator*(const CalQuaternion& q, const CalPoint4& p) {
    CalQuaternion temp = p * CalQuaternion(-q.x, -q.y, -q.z, q.w);
    temp = q * temp;

    return CalPoint4(
        temp.x,
        temp.y,
        temp.z,
        temp.w);
}

inline bool operator==(const CalQuaternion& lhs, const CalQuaternion& rhs) {
    return cal3d::close(lhs.x, rhs.x)
           && cal3d::close(lhs.y, rhs.y)
           && cal3d::close(lhs.z, rhs.z)
           && cal3d::close(lhs.w, rhs.w);
}

inline float dot(const CalQuaternion& left, const CalQuaternion& right) {
    return left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w;
}

inline CalQuaternion slerp(float d, const CalQuaternion& left, const CalQuaternion& right) {
    float norm = dot(left, right);

    bool bFlip = false;

    if (norm < 0.0f) {
        norm = -norm;
        bFlip = true;
    }

    float inv_d;
    if (1.0f - norm < 0.000001f) {
        inv_d = 1.0f - d;
    } else {
        float theta = acos(norm);
        float s = 1.0f / sin(theta);

        inv_d = sin((1.0f - d) * theta) * s;
        d = sin(d * theta) * s;
    }

    if (bFlip) {
        d = -d;
    }

    return CalQuaternion(
        inv_d * left.x + d * right.x,
        inv_d * left.y + d * right.y,
        inv_d * left.z + d * right.z,
        inv_d * left.w + d * right.w);
}
