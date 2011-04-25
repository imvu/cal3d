#pragma once

#include "cal3d/vector.h"

#ifndef IMVU_NO_INTRINSICS

struct CalBase4 {
    union {
        __m128 v;
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };

    explicit CalBase4(__m128 m)
        : v(m)
    {}

    explicit CalBase4(float f) {
        v = _mm_load1_ps(&f);
    }

    struct _ZERO {};
    explicit CalBase4(_ZERO) {
        v = _mm_setzero_ps();
    }

    struct _POINT {};
    explicit CalBase4(_POINT) {
        v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    }

    CalBase4(float _x, float _y, float _z, float _w) {
        float a[4] = { _x, _y, _z, _w };
        v = _mm_loadu_ps(a);
    }

    void set(float _x, float _y, float _z, float _w) {
        float a[4] = { _x, _y, _z, _w };
        v = _mm_loadu_ps(a);
    }

    CalVector asCalVector() const {
        return CalVector(x, y, z);
    }

    void operator*=(float f) {
        __m128 fv = _mm_load1_ps(&f);
        v = _mm_mul_ps(v, fv);
    }

    void operator+=(const CalBase4& rhs) {
        v = _mm_add_ps(v, rhs.v);
    }
};

inline CalBase4 operator*(float f, const CalBase4& v) {
    return CalBase4(_mm_mul_ps(_mm_load1_ps(&f), v.v));
}

inline CalBase4 operator*(const CalBase4& lhs, const CalBase4& rhs) {
    return CalBase4(_mm_mul_ps(lhs.v, rhs.v));
}

inline bool operator==(const CalBase4& lhs, const CalBase4& rhs) {
    return lhs.x == rhs.x
           && lhs.y == rhs.y
           && lhs.z == rhs.z
           && lhs.w == rhs.w;
}

#else

CAL3D_ALIGN_HEAD(16)
struct CalBase4 {
    float x, y, z, w;

    CalBase4(float _x, float _y, float _z, float _w) {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    struct _ZERO {};
    explicit CalBase4(_ZERO) {
        set(0.0f, 0.0f, 0.0f, 0.0f);
    }

    struct _POINT {};
    explicit CalBase4(_POINT) {
        set(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void set(float _x, float _y, float _z, float _w) {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    void operator+=(const CalBase4& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        //w += v.w;
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

inline CalBase4 operator*(float f, const CalBase4& v) {
    return CalBase4(
        f * v.x,
        f * v.y,
        f * v.z,
        f * v.w);
}

inline bool operator==(const CalBase4& lhs, const CalBase4& rhs) {
    return lhs.x == rhs.x
           && lhs.y == rhs.y
           && lhs.z == rhs.z
           && lhs.w == rhs.w;
}

#endif

struct CalVector4 : CalBase4 {
    CalVector4() : CalBase4(CalBase4::_ZERO())
    {}

    CalVector4(float f) : CalBase4(f)
    {}

    CalVector4(float _x, float _y, float _z, float _w = 0.0f) 
        : CalBase4(_x, _y, _z, _w)
    {}

    CAL3D_FORCEINLINE void setAsVector(const CalVector& r) {
        *this = CalVector4(r.x, r.y, r.z, 0.0f);
    }
};

struct CalPoint4 : CalBase4 {
    CalPoint4() : CalBase4(CalBase4::_POINT())
    {}

    CalPoint4(float _x, float _y, float _z, float _w = 1.0f)
    : CalBase4(_x, _y, _z, _w)
    {}

    CAL3D_FORCEINLINE void setAsPoint(const CalVector& r) {
        *this = CalPoint4(r.x, r.y, r.z, 1.0f);
    }
};

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
