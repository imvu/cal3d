#include "cal3d/bonetransform.h"
#include "cal3d/matrix.h"
#include "cal3d/transform.h"

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

BoneTransform::BoneTransform(const cal3d::RotateTranslate& transform) {
    CalMatrix matrix(transform.rotation);
    const CalVector& translation = transform.translation;
    rowx.set(matrix.dxdx, matrix.dxdy, matrix.dxdz, translation.x);
    rowy.set(matrix.dydx, matrix.dydy, matrix.dydz, translation.y);
    rowz.set(matrix.dzdx, matrix.dzdy, matrix.dzdz, translation.z);
}

BoneTransform::BoneTransform(const cal3d::Transform& transform) {
    const CalMatrix& matrix = transform.basis;
    const CalVector& translation = transform.translation;
    rowx.set(matrix.dxdx, matrix.dxdy, matrix.dxdz, translation.x);
    rowy.set(matrix.dydx, matrix.dydy, matrix.dydz, translation.y);
    rowz.set(matrix.dzdx, matrix.dzdy, matrix.dzdz, translation.z);
}
