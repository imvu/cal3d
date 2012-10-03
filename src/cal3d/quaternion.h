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

// This routine is wrong (it returns inner * outer instead of outer * inner as it should). There are other places
// in the code where math is jimmied to reconcile differences in coordinate systems (see the comment "left-handed"
// above, e.g.). It would be a big effort to get it all right, so I am re-instating the incorrect routine. The right
// one is commented out below.

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

//inline CalQuaternion operator*(const CalQuaternion& outer, const CalQuaternion& inner) {
//    float x1 = outer.x;
//    float y1 = outer.y;
//    float z1 = outer.z;
//    float w1 = outer.w;
//    float x2 = inner.x;
//    float y2 = inner.y;
//    float z2 = inner.z;
//    float w2 = inner.w;
//
//    return CalQuaternion(
//        w1*x2 + x1*w2 + y1*z2 - z1*y2,
//        w1*y2 + y1*w2 + z1*x2 - x1*z2,
//        w1*z2 + z1*w2 + x1*y2 - y1*x2,
//        w1*w2 - x1*x2 - y1*y2 - z1*z2);
//}
//
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

#define CHECK_CALQUATERNION_CLOSE(q1, q2, tolerance)    CHECK_CALVECTOR4_CLOSE(q1.asCalVector4(), q2.asCalVector4(), tolerance)

namespace cal3d {
    inline void applyZupToYup(CalQuaternion &q) {
        float temp = q.y;
        q.y = q.z;
        q.z = temp;
        q.z = -q.z;
    }

    inline void applyCoordinateTransform(CalQuaternion &q, CalQuaternion &xfm) {
        CalVector qVector(q.x, q.y, q.z);
        CalVector result = xfm * qVector;
        q.x = result.x;
        q.y = result.y;
        q.z = result.z;
    }

    inline void applyCoordinateTransform(CalVector &v, CalQuaternion &xfm) {
        v = xfm * v;
    }
}