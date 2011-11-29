#include "cal3d/transform.h"

cal3d::Transform cal3d::operator*(const cal3d::Transform& outer, const cal3d::Transform& inner) {
    return cal3d::Transform(
        outer.basis * inner.basis,
        outer.translation + outer.basis * inner.translation);
}

inline CalMatrix operator*(CalMatrix m, const cal3d::Scale& s) {
    m.cx *= s.scale.x;
    m.cy *= s.scale.y;
    m.cz *= s.scale.z;
    return m;
}

inline CalMatrix operator*(const cal3d::Scale& s, CalMatrix m) {
    m.cx *= s.scale;
    m.cy *= s.scale;
    m.cz *= s.scale;
    return m;
}

inline CalVector operator*(const cal3d::Scale& s, const CalVector& translation) {
    return s.scale * translation;
}

cal3d::Transform cal3d::operator*(const cal3d::Transform& outer, const cal3d::Scale& inner) {
    return cal3d::Transform(
        outer.basis * inner,
        outer.translation);
}

cal3d::Transform cal3d::operator*(const cal3d::Scale& outer, const cal3d::Transform& inner) {
    return cal3d::Transform(
        outer * inner.basis,
        outer * inner.translation);
}
