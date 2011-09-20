#pragma once

#include "cal3d/vector4.h"

class CalMatrix;
namespace cal3d {
    struct RotateTranslate;
    struct Transform;
}

// 3x3 transform matrix plus a translation 3-vector (stored in the w components
// of the rows.  This struct needs to be 16-byte aligned for SSE.
struct CAL3D_API BoneTransform {
    BoneTransform() {}
    BoneTransform(const CalVector4& rx, const CalVector4& ry, const CalVector4& rz);
    BoneTransform(const CalMatrix& matrix, const CalVector& translation);
    BoneTransform(const cal3d::RotateTranslate& transform);
    BoneTransform(const cal3d::Transform& transform);

    CalVector4 rowx;
    CalVector4 rowy;
    CalVector4 rowz;
};

inline bool operator==(const BoneTransform& lhs, const BoneTransform& rhs) {
    return lhs.rowx == rhs.rowx
           && lhs.rowy == rhs.rowy
           && lhs.rowz == rhs.rowz;
}
