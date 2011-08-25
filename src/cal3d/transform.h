#pragma once

#include "cal3d/vector.h"
#include "cal3d/quaternion.h"

namespace cal3d {
    struct Transform {
        CalQuaternion rotation;
        CalVector translation;
    };

    // cal3d right-multiplies, so preserve that here :/
    inline Transform operator*(const Transform& inner, const Transform& outer) {
        Transform out;
        out.rotation = inner.rotation * outer.rotation;
        out.translation = inner.translation * outer.rotation + outer.translation;
        return out;
    }
}

