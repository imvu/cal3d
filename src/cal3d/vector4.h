#pragma once

#include "cal3d/vector.h"

#ifndef IMVU_NO_INTRINSICS

#include <xmmintrin.h>

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

    CalBase4(const CalBase4& v)
        : v(v.v)
    {}

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

    void operator=(const CalBase4& rhs) {
        v = rhs.v;
    }

    void operator*=(const CalBase4& rhs) {
        v = _mm_mul_ps(v, rhs.v);
    }

    void operator*=(float f) {
        __m128 fv = _mm_load1_ps(&f);
        v = _mm_mul_ps(v, fv);
    }

    void operator+=(const CalBase4& rhs) {
        v = _mm_add_ps(v, rhs.v);
    }

    void operator-=(const CalBase4& rhs) {
        v = _mm_sub_ps(v, rhs.v);
    }

    float lengthSquared() const {
        return (float)(x * x + y * y + z * z);
    }
    float length() const {
        return sqrtf(lengthSquared());
    }
};

inline CalBase4 operator*(float f, const CalBase4& v) {
    return CalBase4(_mm_mul_ps(_mm_load1_ps(&f), v.v));
}

inline CalBase4 operator+(const CalBase4& lhs, const CalBase4& rhs) {
    return CalBase4(_mm_add_ps(lhs.v, rhs.v));
}

inline CalBase4 operator-(const CalBase4& lhs, const CalBase4& rhs) {
    return CalBase4(_mm_sub_ps(lhs.v, rhs.v));
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

    explicit CalBase4(float f) {
        x = f;
        y = f;
        z = f;
        w = f;
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
        w += v.w;
    }

    void operator-=(const CalBase4& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
    }

    void operator*=(const CalBase4& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
    }

    void operator*=(float f) {
        x *= f;
        y *= f;
        z *= f;
        w *= f;
    }

    CalVector asCalVector() const {
        return CalVector(x, y, z);
    }

    CalVector4 asCalVector4() const {
        return CalVector4(x, y, z, w);
    }
}
CAL3D_ALIGN_TAIL(16);

inline CalBase4 operator+(const CalBase4& lhs, const CalBase4& rhs) {
    return CalBase4(
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
        lhs.w + rhs.w);
}

inline CalBase4 operator*(float f, const CalBase4& v) {
    return CalBase4(
        f * v.x,
        f * v.y,
        f * v.z,
        f * v.w);
}

inline CalBase4 operator*(const CalBase4& lhs, const CalBase4& rhs) {
    return CalBase4(
        lhs.x * rhs.x,
        lhs.y * rhs.y,
        lhs.z * rhs.z,
        lhs.w * rhs.w);
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

    CalVector4(const CalBase4& v) : CalBase4(v)
    {}

    CalVector4(float f) : CalBase4(f)
    {}

    CalVector4(float _x, float _y, float _z, float _w = 0.0f) 
        : CalBase4(_x, _y, _z, _w)
    {}

    explicit CalVector4(const CalVector& v)
        : CalBase4(v.x, v.y, v.z, 0.0f)
    {}

    CalVector4 asCalVector4() const {
        return *this;
    }
};

struct CalPoint4 : CalBase4 {
    CalPoint4() : CalBase4(CalBase4::_POINT())
    {}

    CalPoint4(const CalBase4& v) : CalBase4(v)
    {}

    CalPoint4(float _x, float _y, float _z, float _w = 1.0f)
    : CalBase4(_x, _y, _z, _w)
    {}

    explicit CalPoint4(const CalVector& v)
        : CalBase4(v.x, v.y, v.z, 1.0f)
    {}

    CalVector4 asCalVector4() const {
        return CalVector4(x, y, z, w);
    }
};

#define CHECK_CALVECTOR4_CLOSE(v1, v2, tolerance)   CHECK_CLOSE(0.0f, ((v1)-(v2)).length(), tolerance)
#define CHECK_CALPOINT4_CLOSE(p1, p2, tolerance)    CHECK_CALVECTOR4_CLOSE(p1.asCalVector4(), p2.asCalVector4(), tolerance)

class CalQuaternion;
namespace cal3d {
    void CAL3D_API applyZupToYup(CalVector4 &vec4);
    void CAL3D_API applyZupToYup(CalPoint4 &point4);
    void CAL3D_API applyCoordinateTransform(CalVector4 &vec4, CalQuaternion &xfm);
    void CAL3D_API applyCoordinateTransform(CalPoint4 &point4, CalQuaternion &xfm);
}