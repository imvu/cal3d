#pragma once

#include "cal3d/vector4.h"
#include "cal3d/quaternion.h"

namespace cal3d {
    struct Transform {
        Transform()
        {}

        Transform(const CalQuaternion& r, const CalVector& t)
            : rotation(r)
            , translation(t)
        {}

        CalQuaternion rotation;
        CalVector translation;
    };

    inline bool operator==(const Transform& lhs, const Transform& rhs) {
        return lhs.rotation == rhs.rotation && lhs.translation == rhs.translation;
    }

    inline Transform operator*(const Transform& outer, const Transform& inner) {
        return Transform(
            outer.rotation * inner.rotation,
            outer.rotation * inner.translation + outer.translation);
    }

    inline CalVector operator*(const Transform& t, const CalVector& v) {
        return t.rotation * v + t.translation;
    }

    inline Transform blend(float factor, const Transform& left, const Transform& right) {
        return Transform(
            slerp(factor, left.rotation, right.rotation),
            lerp(factor, left.translation, right.translation));
    }
}
