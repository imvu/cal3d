#pragma once

#include "cal3d/vector.h"

CAL3D_ALIGN_HEAD(16)
struct CalBase4 {
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
}
CAL3D_ALIGN_TAIL(16);

struct CalVector4 : CalBase4 {
    CalVector4() {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 0.0f;
    }

    CalVector4(float x, float y, float z, float w = 0.0f) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    CAL3D_FORCEINLINE void setAsVector(const CalVector& r) {
        x = r.x;
        y = r.y;
        z = r.z;
        w = 0.0f;
    }
};

struct CalPoint4 : CalBase4 {
    CalPoint4() {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 1.0f;
    }

    CalPoint4(float x, float y, float z, float w = 1.0f) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    CAL3D_FORCEINLINE void setAsPoint(const CalVector& r) {
        x = r.x;
        y = r.y;
        z = r.z;
        w = 1.0f;
    }
};

inline bool operator==(const CalBase4& lhs, const CalBase4& rhs) {
    return lhs.x == rhs.x
           && lhs.y == rhs.y
           && lhs.z == rhs.z
           && lhs.w == rhs.w;
}

// 3x3 transform matrix plus a translation 3-vector (stored in the w components
// of the rows.  This struct needs to be 16-byte aligned for SSE.
struct BoneTransform {
    CalVector4 rowx;
    CalVector4 rowy;
    CalVector4 rowz;
};

inline bool operator==(const BoneTransform& lhs, const BoneTransform& rhs) {
    return lhs.rowx == rhs.rowx
           && lhs.rowy == rhs.rowy
           && lhs.rowz == rhs.rowz;
}
