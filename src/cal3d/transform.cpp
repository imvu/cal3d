#include "cal3d/transform.h"

cal3d::Transform cal3d::operator*(const cal3d::Transform& outer, const cal3d::Transform& inner) {
    return cal3d::Transform(
        outer.basis * inner.basis,
        outer.translation + outer.basis * inner.translation);
}

inline CalMatrix operator*(CalMatrix m, const cal3d::Scale& s) {
    m.dxdx *= s.scale.x;
    m.dydx *= s.scale.x;
    m.dzdx *= s.scale.x;
    m.dxdy *= s.scale.y;
    m.dydy *= s.scale.y;
    m.dzdy *= s.scale.y;
    m.dxdz *= s.scale.z;
    m.dydz *= s.scale.z;
    m.dzdz *= s.scale.z;
    return m;
}

inline CalMatrix operator*(const cal3d::Scale& s, CalMatrix m) {
    m.dxdx *= s.scale.x;
    m.dydx *= s.scale.y;
    m.dzdx *= s.scale.z;
    m.dxdy *= s.scale.x;
    m.dydy *= s.scale.y;
    m.dzdy *= s.scale.z;
    m.dxdz *= s.scale.x;
    m.dydz *= s.scale.y;
    m.dzdz *= s.scale.z;
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
