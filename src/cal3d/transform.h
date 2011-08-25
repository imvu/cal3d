#pragma once

#include "cal3d/vector.h"
#include "cal3d/quaternion.h"

namespace cal3d {
    struct Transform {
        CalVector translation;
        CalQuaternion rotation;
    };
}
