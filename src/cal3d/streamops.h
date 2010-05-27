#pragma once

#include <iosfwd>
#include <cal3d/aabox.h>
#include <cal3d/vector4.h>
#include <cal3d/quaternion.h>
#include <cal3d/corekeyframe.h>

inline std::ostream& operator<<(std::ostream& os, const CalVector4& v) {
  return os << "CalVector4(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
}

inline std::ostream& operator<<(std::ostream& os, const BoneTransform& bt) {
  return os << "[rowx=" << bt.rowx << " rowy=" << bt.rowy << " rowz=" << bt.rowz << "]";
}

inline std::ostream& operator<<(std::ostream& os, const CalVector& v) {
    return os << "CalVector(" << v.x << "," << v.y << "," << v.z << ")";
}

inline std::ostream& operator<<(std::ostream& os, const CalAABox& box) {
    return os << "[min=" << box.min << " max=" << box.max << "]";
}

inline std::ostream& operator<<(std::ostream& os, const CalQuaternion& quat) {
    return os << "CalQuaternion(" << quat.x << "," << quat.y << "," << quat.z << "," << quat.w << ")";
}

inline std::ostream& operator<<(std::ostream& os, const CalCoreKeyframe& keyframe) {
    return os << "CalCoreKeyframe(" << keyframe.time << ", " << keyframe.translation << ", " << keyframe.rotation << ")";
}
