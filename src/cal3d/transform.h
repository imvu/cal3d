#pragma once

#include "cal3d/vector.h"
#include "cal3d/quaternion.h"

namespace cal3d {
    struct Transform {
        CalQuaternion rotation;
        CalVector translation;
    };

    inline Transform operator*(const Transform& outer, const Transform& inner) {
        Transform out;
        out.rotation = inner.rotation * outer.rotation;
        out.translation = outer.rotation * inner.translation + outer.translation;
        return out;
    }

    inline CalVector operator*(const Transform& t, const CalVector& v) {
        return t.rotation * v + t.translation;
    }
}
