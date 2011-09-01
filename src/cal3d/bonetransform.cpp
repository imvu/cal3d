#include "cal3d/bonetransform.h"
#include "cal3d/matrix.h"

BoneTransform::BoneTransform(const CalVector4& rx, const CalVector4& ry, const CalVector4& rz)
    : rowx(rx)
    , rowy(ry)
    , rowz(rz)
{}

BoneTransform::BoneTransform(const CalMatrix& matrix, const CalVector& translation) {
    rowx.set(matrix.dxdx, matrix.dxdy, matrix.dxdz, translation.x);
    rowy.set(matrix.dydx, matrix.dydy, matrix.dydz, translation.y);
    rowz.set(matrix.dzdx, matrix.dzdy, matrix.dzdz, translation.z);
}
