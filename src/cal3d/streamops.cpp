#include "cal3d/streamops.h"
#include "cal3d/aabox.h"
#include "cal3d/bonetransform.h"
#include "cal3d/coreanimation.h"
#include "cal3d/corekeyframe.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/coretrack.h"
#include "cal3d/quaternion.h"
#include "cal3d/transform.h"
#include "cal3d/vector4.h"

#include <iostream>

namespace {
    template<typename T>
    void writeIterable(std::ostream& os, const T& items) {
        os << "[";
        bool first = true;
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << *it;
        }
        os << "]";
    }
}

std::ostream& operator<<(std::ostream& os, const BoneTransform& bt) {
    return os << "[rowx=" << bt.rowx << " rowy=" << bt.rowy << " rowz=" << bt.rowz << "]";
}

std::ostream& operator<<(std::ostream& os, const cal3d::RotateTranslate& t) {
    return os << "cal3d::RotateTranslate(" << t.rotation << ", " << t.translation << ")";
}

std::ostream& operator<<(std::ostream& os, const cal3d::Transform& t) {
    return os << "cal3d::Transform(" << t.basis << ", " << t.translation << ")";
}

std::ostream& operator<<(std::ostream& os, const CalAABox& box) {
    return os << "[min=" << box.min << " max=" << box.max << "]";
}

std::ostream& operator<<(std::ostream& os, const CalCoreAnimation& animation) {
    os << "CalCoreAnimation(" << animation.duration << ", ";
    writeIterable(os, animation.tracks);
    return os << ")";
}

std::ostream& operator<<(std::ostream& os, const CalCoreKeyframe& keyframe) {
    return os << "CalCoreKeyframe(" << keyframe.time << ", " << keyframe.transform << ")";
}

std::ostream& operator<<(std::ostream& os, const CalCoreSubmesh::TextureCoordinate& tc) {
    return os << "TextureCoordinate(" << tc.u << ", " << tc.v << ")";
}

std::ostream& operator<<(std::ostream& os, const CalCoreSubmesh::Vertex& vertex) {
    return os << "Vertex(" << vertex.position << ", " << vertex.normal << ")";
}

std::ostream& operator<<(std::ostream& os, const CalCoreTrack& track) {
    os << "CalCoreTrack(" << track.coreBoneId << ", ";
    writeIterable(os, track.keyframes);
    return os << ")";
}

std::ostream& operator<<(std::ostream& os, const CalMatrix& m) {
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

std::ostream& operator<<(std::ostream& os, const CalPoint4& v) {
    return os << "CalPoint4(" << v.x << ", " << v.y << ", " << v.z << ")";
}

std::ostream& operator<<(std::ostream& os, const CalQuaternion& quat) {
    return os << "CalQuaternion(" << quat.x << "," << quat.y << "," << quat.z << "," << quat.w << ")";
}

std::ostream& operator<<(std::ostream& os, const CalVector4& v) {
    return os << "CalVector4(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
}

std::ostream& operator<<(std::ostream& os, const CalVector& v) {
    return os << "CalVector(" << v.x << "," << v.y << "," << v.z << ")";
}
