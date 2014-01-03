#pragma once

#include "cal3d/vector.h"

#ifdef IMVU_NO_INTRINSICS
#else
#include <xmmintrin.h>
#endif

CAL3D_ALIGN_HEAD(16) struct CalBase4 {
#ifdef IMVU_NO_INTRINSICS
    float x, y, z, w;
#else
    union {
        __m128 v;
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };
#endif

    CalBase4(float _x, float _y, float _z, float _w) {
#ifdef IMVU_NO_INTRINSICS
        x = _x;
        y = _y;
        z = _z;
        w = _w;
#else
        float a[4] = { _x, _y, _z, _w };
        v = _mm_loadu_ps(a);
#endif
    }

    struct _ZERO {};
    explicit CalBase4(_ZERO) {
#ifdef IMVU_NO_INTRINSICS
        set(0.0f, 0.0f, 0.0f, 0.0f);
#else
        v = _mm_setzero_ps();
#endif
    }

    struct _POINT {};
    explicit CalBase4(_POINT) {
#ifdef IMVU_NO_INTRINSICS
        set(0.0f, 0.0f, 0.0f, 1.0f);
#else
        v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
#endif
    }

#ifdef IMVU_NO_INTRINSICS
#else
    CalBase4(const CalBase4& v)
        : v(v.v)
    {}

    explicit CalBase4(__m128 m)
        : v(m)
    {}
#endif

    explicit CalBase4(float f) {
#ifdef IMVU_NO_INTRINSICS
        x = f;
        y = f;
        z = f;
        w = f;
#else
        v = _mm_load1_ps(&f);
#endif
    }

    void set(float _x, float _y, float _z, float _w) {
#ifdef IMVU_NO_INTRINSICS
        x = _x;
        y = _y;
        z = _z;
        w = _w;
#else
        float a[4] = { _x, _y, _z, _w };
        v = _mm_loadu_ps(a);
#endif
    }

//    CalBase4 operator-() const {
//        return CalBase4(-x, -y, -z, -w);
//    }
//
    void operator=(const CalBase4& rhs) {
#ifdef IMVU_NO_INTRINSICS
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
        w = rhs.w;
#else
        v = rhs.v;
#endif
    }

    void operator+=(const CalBase4& rhs) {
#ifdef IMVU_NO_INTRINSICS
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
#else
        v = _mm_add_ps(v, rhs.v);
#endif
    }

    void operator-=(const CalBase4& rhs) {
#ifdef IMVU_NO_INTRINSICS
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
#else
        v = _mm_sub_ps(v, rhs.v);
#endif
    }

    void operator*=(const CalBase4& rhs) {
#ifdef IMVU_NO_INTRINSICS
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
#else
        v = _mm_mul_ps(v, rhs.v);
#endif
    }

    void operator*=(float f) {
#ifdef IMVU_NO_INTRINSICS
        x *= f;
        y *= f;
        z *= f;
        w *= f;
#else
        __m128 fv = _mm_load1_ps(&f);
        v = _mm_mul_ps(v, fv);
#endif
    }

    CalVector asCalVector() const {
        return CalVector(x, y, z);
    }

    float lengthSquared() const {
        return (float)(x * x + y * y + z * z);
    }
    float length() const {
        return sqrtf(lengthSquared());
    }
} CAL3D_ALIGN_TAIL(16);


inline CalBase4 operator+(const CalBase4& lhs, const CalBase4& rhs) {
#ifdef IMVU_NO_INTRINSICS
    return CalBase4(
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
        lhs.w + rhs.w);
#else
    return CalBase4(_mm_add_ps(lhs.v, rhs.v));
#endif
}

inline CalBase4 operator-(const CalBase4& lhs, const CalBase4& rhs) {
#ifdef IMVU_NO_INTRINSICS
    return CalBase4(
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
        lhs.w - rhs.w);
#else
    return CalBase4(_mm_sub_ps(lhs.v, rhs.v));
#endif
}

inline CalBase4 operator*(float f, const CalBase4& v) {
#ifdef IMVU_NO_INTRINSICS
    return CalBase4(
        f * v.x,
        f * v.y,
        f * v.z,
        f * v.w);
#else
    return CalBase4(_mm_mul_ps(_mm_load1_ps(&f), v.v));
#endif
}

inline CalBase4 operator*(const CalBase4& lhs, const CalBase4& rhs) {
#ifdef IMVU_NO_INTRINSICS
    return CalBase4(
        lhs.x * rhs.x,
        lhs.y * rhs.y,
        lhs.z * rhs.z,
        lhs.w * rhs.w);
#else
    return CalBase4(_mm_mul_ps(lhs.v, rhs.v));
#endif
}

inline bool operator==(const CalBase4& lhs, const CalBase4& rhs) {
    return lhs.x == rhs.x
           && lhs.y == rhs.y
           && lhs.z == rhs.z
           && lhs.w == rhs.w;
}

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
}