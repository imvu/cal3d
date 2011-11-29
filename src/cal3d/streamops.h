#pragma once

#include <iosfwd>
#include <cal3d/aabox.h>
#include <cal3d/vector4.h>
#include <cal3d/quaternion.h>
#include <cal3d/corekeyframe.h>
#include <cal3d/coreanimation.h>
#include <cal3d/coretrack.h>
#include <cal3d/bonetransform.h>
#include <cal3d/transform.h>

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

inline std::ostream& operator<<(std::ostream& os, const CalMatrix& m) {
    os << "CalMatrix(";
    const float* p = &m.cx.x;
    for (size_t i = 0; i < 9; ++i) {
        if (i) {
            os << ", ";
        }
        os << p[i];
    }
    return os << ")";
}

namespace cal3d {
    inline std::ostream& operator<<(std::ostream& os, const RotateTranslate& t) {
        return os << "cal3d::RotateTranslate(" << t.rotation << ", " << t.translation << ")";
    }

    inline std::ostream& operator<<(std::ostream& os, const Transform& t) {
        return os << "cal3d::Transform(" << t.basis << ", " << t.translation << ")";
    }
}

inline std::ostream& operator<<(std::ostream& os, const CalCoreKeyframe& keyframe) {
    return os << "CalCoreKeyframe(" << keyframe.time << ", " << keyframe.transform << ")";
}

inline std::ostream& operator<<(std::ostream& os, const CalCoreAnimation& animation) {
    return os << "CalCoreAnimation(" << animation.duration << ", " << animation.tracks << ")";
}

inline std::ostream& operator<<(std::ostream& os, const CalCoreTrack& track) {
    return os << "CalCoreTrack(" << track.coreBoneId << ", " << track.keyframes << ")";
}
