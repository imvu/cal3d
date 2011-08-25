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

    inline float& operator[](unsigned int index) {
        return (&x)[index];
    }

    inline const float& operator[](unsigned int index) const {
        return (&x)[index];
    }

    inline void operator=(const CalQuaternion& q) {
        x = q.x;
        y = q.y;
        z = q.z;
        w = q.w;
    }

    inline void operator*=(const CalQuaternion& q) {
        float qx, qy, qz, qw;
        qx = x;
        qy = y;
        qz = z;
        qw = w;

        x = qw * q.x + qx * q.w + qy * q.z - qz * q.y;
        y = qw * q.y - qx * q.z + qy * q.w + qz * q.x;
        z = qw * q.z + qx * q.y - qy * q.x + qz * q.w;
        w = qw * q.w - qx * q.x - qy * q.y - qz * q.z;
    }

    inline void operator*=(const CalVector& v) {
        float qx, qy, qz, qw;
        qx = x;
        qy = y;
        qz = z;
        qw = w;

        x = qw * v.x            + qy * v.z - qz * v.y;
        y = qw * v.y - qx * v.z            + qz * v.x;
        z = qw * v.z + qx * v.y - qy * v.x;
        w =          - qx * v.x - qy * v.y - qz * v.z;
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
};

inline CalQuaternion operator*(CalQuaternion q, const CalQuaternion& u) {
    q *= u;
    return q;
}

inline bool operator==(const CalQuaternion& lhs, const CalQuaternion& rhs) {
    return close(lhs.x, rhs.x)
           && close(lhs.y, rhs.y)
           && close(lhs.z, rhs.z)
           && close(lhs.w, rhs.w);
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
