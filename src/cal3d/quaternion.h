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
#include "cal3d/vector.h"

class CAL3D_API CalQuaternion {
public:
    float x;
    float y;
    float z;
    float w;

    inline CalQuaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {};
    inline CalQuaternion(const CalQuaternion& q): x(q.x), y(q.y), z(q.z), w(q.w) {};
    inline CalQuaternion(float qx, float qy, float qz, float qw): x(qx), y(qy), z(qz), w(qw) {};

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

    inline void set(float qx, float qy, float qz, float qw) {
        x = qx;
        y = qy;
        z = qz;
        w = qw;
    }

    // right-handed API
    void setAxisAngle(const CalVector& axis, float angle) {
        float h2 = angle * 0.5f;
        float s = sin(h2);
        w = -cos(h2); // left-handed
        x = s * axis.x;
        y = s * axis.y;
        z = s * axis.z;
    }
};

// left-multiply, rotate v by quaternion and produce rotated vector
CalVector CAL3D_API operator*(const CalQuaternion& q, const CalVector& v);

// Implicitly convert vector to quaternion (w=0), multiply times q
inline CalQuaternion operator*(const CalVector& v, const CalQuaternion& q) {
    float qx = q.x;
    float qy = q.y;
    float qz = q.z;
    float qw = q.w;

    return CalQuaternion(
        qw * v.x            + qy * v.z - qz * v.y,
        qw * v.y - qx * v.z            + qz * v.x,
        qw * v.z + qx * v.y - qy * v.x,
                 - qx * v.x - qy * v.y - qz * v.z);
}



inline CalQuaternion operator*(const CalQuaternion& outer, const CalQuaternion& inner) {
    float qx = inner.x;
    float qy = inner.y;
    float qz = inner.z;
    float qw = inner.w;

    return CalQuaternion(
        qw * outer.x + qx * outer.w + qy * outer.z - qz * outer.y,
        qw * outer.y - qx * outer.z + qy * outer.w + qz * outer.x,
        qw * outer.z + qx * outer.y - qy * outer.x + qz * outer.w,
        qw * outer.w - qx * outer.x - qy * outer.y - qz * outer.z);
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
