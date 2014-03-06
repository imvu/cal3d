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

TEST(can_convert_close_to_180_rotation_matrix_to_quaternion) {
    // An earlier version of our matrix-to-quaternion conversion routine was
    // numerically instable in this case (similar to the routine on Flipcode).
    CalVector x(0.993884f, 0, 0.110431f);
    CalVector y(0, -1, 0);
    CalVector z(0.110431f, 0, -0.993884f);
    CalMatrix m(x, y, z);

    CalQuaternion q(m);
    float mag = sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    CHECK_CLOSE(1.0f, mag, 0.001f);
}

TEST(applyZUpToYUp_transform) {
    CalQuaternion quat;
    CalVector axis(1,1,1);
    axis.normalize();
    quat.setAxisAngle(axis, DegreesToRadians(30));
    CalVector trans(1.0f, 2.0f, 3.0f);
    cal3d::RotateTranslate rotTrans(quat, trans);
    cal3d::applyZUpToYUp(rotTrans);

    cal3d::applyZUpToYUp(quat);
    cal3d::applyZUpToYUp(trans);
    CHECK_EQUAL(trans, rotTrans.translation);
    CHECK_EQUAL(quat, rotTrans.rotation);
}

//TEST(applyCoordinateTransform_transform) {
//    CalQuaternion ZUpToYUp(-0.70710678f, 0.0f, 0.0f, 0.70710678f);
//    cal3d::RotateTranslate rt(CalQuaternion(0.0f, 0.0f, 0.70710678f, 0.70710678f), CalVector(2.0f, 3.0f, 5.0f));
//
//    cal3d::applyZUpToYUp(rt);
//
//    CHECK_CALVECTOR_CLOSE(rt.translation, CalVector(2.0f, 5.0f, -3.0f), 0.000001f);
//    CHECK_CALQUATERNION_CLOSE(rt.rotation, CalQuaternion(0.0f, 0.70710678f, 0.0f, 0.70710678f), 0.000001f);
//}
