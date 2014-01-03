#include "TestPrologue.h"
#include <libzero/Math.h>
#include <cal3d/streamops.h>
#include <cal3d/transform.h>

TEST(scale_defaults_to_identity) {
    cal3d::Scale s;
    CHECK_EQUAL(CalVector(1, 1, 1), s.scale);
}

TEST(can_assign_vector_to_scale) {
    cal3d::Scale s;
    s = CalVector(1, 2, 3);
    CHECK_EQUAL(CalVector(1, 2, 3), s.scale);
}

TEST(matrix_is_column_major) {
    CalMatrix matrix(CalVector(0, 1, 0), CalVector(-1, 0, 0), CalVector(0, 0, 1));
    CHECK_EQUAL(CalVector(0, 1, 0), matrix * CalVector(1, 0, 0));
}

TEST(non_uniform_scale_before_and_after_translation) {
    cal3d::Scale scale(CalVector(1, 2, 3));
    cal3d::Transform transform(CalMatrix(), CalVector(1, 2, 3));

    CHECK_EQUAL(CalVector(2, 6, 12), (transform * scale) * CalVector(1, 2, 3));
    CHECK_EQUAL(CalVector(2, 8, 18), (scale * transform) * CalVector(1, 2, 3));
}

TEST(non_uniform_scale_before_and_after_transform) {
    cal3d::Scale scale(CalVector(1, 2, 3));
    CalMatrix rotation(CalVector(0, 1, 0), CalVector(-1, 0, 0), CalVector(0, 0, 1));

    cal3d::Transform transform(rotation, CalVector(1, 2, 3));

    CHECK_EQUAL(CalVector(-3, 3, 12), (transform * scale) * CalVector(1, 2, 3));
    CHECK_EQUAL(CalVector(-1, 6, 18), (scale * transform) * CalVector(1, 2, 3));
}

TEST(applyZupToYup_transform) {
    CalQuaternion quat;
    CalVector axis(1,1,1);
    axis.normalize();
    quat.setAxisAngle(axis, DegreesToRadians(30));
    CalVector trans(1.0f, 2.0f, 3.0f);
    cal3d::RotateTranslate rotTrans(quat, trans);
    cal3d::applyZupToYup(rotTrans);

    cal3d::applyZupToYup(quat);
    cal3d::applyZupToYup(trans);
    CHECK_EQUAL(trans, rotTrans.translation);
    CHECK_EQUAL(quat, rotTrans.rotation);
}

//TEST(applyCoordinateTransform_transform) {
//    CalQuaternion ZUpToYUp(-0.70710678f, 0.0f, 0.0f, 0.70710678f);
//    cal3d::RotateTranslate rt(CalQuaternion(0.0f, 0.0f, 0.70710678f, 0.70710678f), CalVector(2.0f, 3.0f, 5.0f));
//
//    cal3d::applyZupToYup(rt);
//
//    CHECK_CALVECTOR_CLOSE(rt.translation, CalVector(2.0f, 5.0f, -3.0f), 0.000001f);
//    CHECK_CALQUATERNION_CLOSE(rt.rotation, CalQuaternion(0.0f, 0.70710678f, 0.0f, 0.70710678f), 0.000001f);
//}