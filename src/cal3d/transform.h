#pragma once

#include "cal3d/matrix.h"
#include "cal3d/vector4.h"
#include "cal3d/quaternion.h"

namespace cal3d {
    struct Scale {
        Scale(const CalVector& s)
        : scale(s)
        {}
        CalVector scale;
    };

    struct RotateTranslate {
        RotateTranslate()
        {}

        RotateTranslate(const CalQuaternion& r, const CalVector& t)
            : rotation(r)
            , translation(t)
        {}

        CalQuaternion rotation;
        CalVector translation;
    };

    inline bool operator==(const RotateTranslate& lhs, const RotateTranslate& rhs) {
        return lhs.rotation == rhs.rotation && lhs.translation == rhs.translation;
    }

    // Given that Transform is equivalent to rotate-then-translate,
    //   i.e. v' = M * v = Mt * Mr * v
    // we are looking for a matrix M' such that:
    //   v = M' * M * v
    //   v = M' * (Mt * Mr) * v
    //   v = (Mr' * Mt') * (Mt * Mr) * v
    // thus,
    //   M' = (Mr' * Mt')
    inline RotateTranslate invert(const RotateTranslate& t) {
        return RotateTranslate(
            -t.rotation,
            (-t.rotation) * (-t.translation));
    }

    inline RotateTranslate operator*(const RotateTranslate& outer, const RotateTranslate& inner) {
        return RotateTranslate(
            outer.rotation * inner.rotation,
            outer.rotation * inner.translation + outer.translation);
    }

    inline CalVector operator*(const RotateTranslate& t, const CalVector& v) {
        return t.rotation * v + t.translation;
    }

    inline RotateTranslate blend(float factor, const RotateTranslate& left, const RotateTranslate& right) {
        return RotateTranslate(
            slerp(factor, left.rotation, right.rotation),
            lerp(factor, left.translation, right.translation));
    }

    struct Transform {
        Transform()
        {}

        Transform(const RotateTranslate& t)
            : basis(t.rotation)
            , translation(t.translation)
        {}

        Transform(const CalMatrix& m, const CalVector& t)
            : basis(m)
            , translation(t)
        {}

        Transform(const CalQuaternion& r, const CalVector& t)
            : basis(r)
            , translation(t)
        {}

        CalMatrix basis;
        CalVector translation;
    };

    inline bool operator==(const Transform& lhs, const Transform& rhs) {
        return lhs.basis == rhs.basis && lhs.translation == rhs.translation;
    }

    CAL3D_API Transform operator*(const Transform& outer, const Transform& inner);
    CAL3D_API Transform operator*(const Transform& outer, const Scale& inner);
    CAL3D_API Transform operator*(const Scale& outer, const Transform& inner);

    inline CalVector operator*(const Transform& t, const CalVector& v) {
        return t.translation + t.basis * v;
    }
}
