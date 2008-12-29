#pragma once

#include <ostream>
#include <cal3d/vector4.h>

std::ostream& operator<<(std::ostream& os, const CalVector4& v) {
  return os << v.x << "," << v.y << "," << v.z << "," << v.w;
}

std::ostream& operator<<(std::ostream& os, const BoneTransform& bt) {
  return os << "[rowx=" << bt.rowx << " rowy=" << bt.rowy << " rowz=" << bt.rowz << "]";
}
