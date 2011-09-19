#include "cal3d/transform.h"

cal3d::Transform cal3d::operator*(const cal3d::Transform& outer, const cal3d::Transform& inner) {
    return cal3d::Transform(
        outer.basis * inner.basis,
        outer.translation + outer.basis * inner.translation);
}
