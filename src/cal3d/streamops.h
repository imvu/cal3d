#pragma once

#include <cal3d/coresubmesh.h>

#include <iosfwd>

class CalCoreAnimation;
class CalCoreKeyframe;
class CalCoreTrack;
class CalQuaternion;
class CalVector;
struct BoneTransform;
struct CalAABox;
struct CalPoint4;
struct CalVector4;

namespace cal3d {
    struct RotateTranslate;
    struct Transform;
};

CAL3D_API std::ostream& operator<<(std::ostream& os, const BoneTransform& bt);
CAL3D_API std::ostream& operator<<(std::ostream& os, const cal3d::RotateTranslate& t);
CAL3D_API std::ostream& operator<<(std::ostream& os, const cal3d::Transform& t);
CAL3D_API std::ostream& operator<<(std::ostream& os, const CalAABox& box);
CAL3D_API std::ostream& operator<<(std::ostream& os, const CalCoreAnimation& animation);
CAL3D_API std::ostream& operator<<(std::ostream& os, const CalCoreKeyframe& keyframe);
CAL3D_API std::ostream& operator<<(std::ostream& os, const CalCoreSubmesh::TextureCoordinate& tc);
CAL3D_API std::ostream& operator<<(std::ostream& os, const CalCoreSubmesh::Vertex& vertex);
CAL3D_API std::ostream& operator<<(std::ostream& os, const CalCoreTrack& track);
CAL3D_API std::ostream& operator<<(std::ostream& os, const CalMatrix& m);
CAL3D_API std::ostream& operator<<(std::ostream& os, const CalPoint4& v);
CAL3D_API std::ostream& operator<<(std::ostream& os, const CalQuaternion& quat);
CAL3D_API std::ostream& operator<<(std::ostream& os, const CalVector4& v);
CAL3D_API std::ostream& operator<<(std::ostream& os, const CalVector& v);
