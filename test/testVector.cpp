#include "TestPrologue.h"
#include <cal3d/color.h>
#include <cal3d/vector.h>
#include <cal3d/vector4.h>
#include <cal3d/quaternion.h>
#include <cal3d/streamops.h>


TEST(Vector) {
    CalVector s, t;
    s = t = CalVector(0, 0, 0);
    CHECK_EQUAL(s, CalVector(0, 0, 0));
    CHECK_EQUAL(t, CalVector(0, 0, 0));
}


#define CHECK_COLOR(uint32, float3) \
    do { \
        CHECK_EQUAL(uint32, CalMakeColor(float3)); \
        CHECK_EQUAL(float3, CalVectorFromColor(uint32)); \
    } while (0)


TEST(ColorFromVector) {
    CHECK_COLOR(0xff000000, CalVector(0.0f, 0.0f, 0.0f));
    CHECK_COLOR(0xffff0000, CalVector(1.0f, 0.0f, 0.0f));
    CHECK_COLOR(0xff00ff00, CalVector(0.0f, 1.0f, 0.0f));
    CHECK_COLOR(0xff0000ff, CalVector(0.0f, 0.0f, 1.0f));
    CHECK_COLOR(0xffffffff, CalVector(1.0f, 1.0f, 1.0f));
}

TEST(Vector4_default) {
    CAL3D_ALIGN_HEAD(16)
        float f[4]
    CAL3D_ALIGN_TAIL(16);
    *(CalVector4*)f = CalVector4();
    CHECK_EQUAL(0.0, f[0]);
    CHECK_EQUAL(0.0, f[1]);
    CHECK_EQUAL(0.0, f[2]);
    CHECK_EQUAL(0.0, f[3]);

    CHECK_EQUAL(0.0, CalVector4().w);
}

TEST(Point4_default) {
    CAL3D_ALIGN_HEAD(16)
        float f[4]
    CAL3D_ALIGN_TAIL(16);
    *(CalPoint4*)f = CalPoint4();
    CHECK_EQUAL(0.0, f[0]);
    CHECK_EQUAL(0.0, f[1]);
    CHECK_EQUAL(0.0, f[2]);
    CHECK_EQUAL(1.0, f[3]);

    CHECK_EQUAL(1.0, CalPoint4().w);
}

TEST(applyZupToYup) {
    CalPoint4 point4(0.0f, 1.0f, 2.0f, 3.0f);
    CalVector4 vec4(0.0f, 1.0f, 2.0f, 3.0f);
    CalVector vec(0.0f, 1.0f, 2.0f);
    CalQuaternion quat(0.0f, 1.0f, 2.0f, 3.0f);
    CalPoint4 point4_x(0.0f, 2.0f, -1.0f, 3.0f);
    CalVector4 vec4_x(0.0f, 2.0f, -1.0f, 3.0f);
    CalQuaternion quat_x(0.0f, 2.0f, -1.0f, 3.0f);    
    CalVector vec_x(0.0f, 2.0f, -1.0f);
    cal3d::applyZupToYup(point4);
    cal3d::applyZupToYup(vec4);
    cal3d::applyZupToYup(vec);
    cal3d::applyZupToYup(quat);
    CHECK_EQUAL((point4_x.asCalVector()), (point4.asCalVector()));    
    CHECK_EQUAL((vec4_x.asCalVector()), (vec4.asCalVector()));
    CHECK_EQUAL((point4_x.asCalVector()), (point4.asCalVector()));
    CHECK_EQUAL(vec_x, vec);    
    CHECK_EQUAL(quat_x, quat);
}

TEST(quaternionMultiply) {
    CalQuaternion q1l(0.0f, 0.0f, 0.0f, 1.0f);
    CalQuaternion q1r(0.1980f, 0.2971f, 0.4952f, 0.7923f);
    CalQuaternion q1x(0.1980f, 0.2971f, 0.4952f, 0.7923f);

    CalQuaternion q2l(0.1980f, 0.2971f, 0.4952f, 0.7923f);
    CalQuaternion q2r(0.0f, 0.0f, 0.0f, 1.0f);
    CalQuaternion q2x(0.1980f, 0.2971f, 0.4952f, 0.7923f);

    CalQuaternion q3l(0.7071f, 0.0f, 0.0f, 0.7071f);
    CalQuaternion q3r(0.1980f, 0.2971f, 0.4952f, 0.7923f);
    CalQuaternion q3x(0.70024113f, -0.14007651f, 0.56023533f, 0.42022953f);

    CalQuaternion q4l(0.0f, 0.7071f, 0.0f, 0.7071f);
    CalQuaternion q4r(0.1980f, 0.2971f, 0.4952f, 0.7923f);
    CalQuaternion q4x(0.49016172f, 0.77031474f, 0.21015012f, 0.35015592f);

    CalQuaternion q5l(0.0f, 0.0, 0.7071f, 0.7071f);
    CalQuaternion q5r(0.1980f, 0.2971f, 0.4952f, 0.7923f);
    CalQuaternion q5x(-0.07007361f, 0.35008521f, 0.91039125f, 0.21007941f);

    CalQuaternion q1p = q1l * q1r;
    CalQuaternion q2p = q2l * q2r;
    CalQuaternion q3p = q3l * q3r;
    CalQuaternion q4p = q4l * q4r;
    CalQuaternion q5p = q5l * q5r;

    CHECK_CALQUATERNION_CLOSE(q1p, q1x, 0.000001f);
    CHECK_CALQUATERNION_CLOSE(q2p, q2x, 0.000001f);
    CHECK_CALQUATERNION_CLOSE(q3p, q3x, 0.000001f);
    CHECK_CALQUATERNION_CLOSE(q4p, q4x, 0.000001f);
    CHECK_CALQUATERNION_CLOSE(q5p, q5x, 0.000001f);
}

TEST(applyQuaternionToVector) {
    CalQuaternion q0(0.0f, 0.0f, 0.0f, 1.0f);
    CalQuaternion q1(0.70710678f, 0.0f, 0.0f, 0.70710678f);
    CalQuaternion q2(0.0f, 0.70710678f, 0.0f, 0.70710678f);
    CalQuaternion q3(0.0f, 0.0f, 0.70710678f, 0.70710678f);
    CalVector v(2.0f, 3.0f, 5.0f);

    CalVector p0 = q0 * v;
    CalVector p1 = q1 * v;
    CalVector p2 = q2 * v;
    CalVector p3 = q3 * v;

    CHECK_CALVECTOR_CLOSE(p0, CalVector(2.0f, 3.0f, 5.0f), 0.000001f);
    CHECK_CALVECTOR_CLOSE(p1, CalVector(2.0f, -5.0f, 3.0f), 0.000001f);
    CHECK_CALVECTOR_CLOSE(p2, CalVector(5.0f, 3.0f, -2.0f), 0.000001f);
    CHECK_CALVECTOR_CLOSE(p3, CalVector(-3.0f, 2.0f, 5.0f), 0.000001f);
}

TEST(applyCoordinateTransformToPoint4) {
    CalQuaternion zUpToYUp(-0.70710678f, 0.0f, 0.0f, 0.70710678f);
    CalPoint4 p1(1.0f, 0.0f, 0.0f);
    CalPoint4 p2(0.0f, 1.0f, 0.0f);
    CalPoint4 p3(0.0f, 0.0f, 1.0f);

    cal3d::applyCoordinateTransform(p1, zUpToYUp);
    cal3d::applyCoordinateTransform(p2, zUpToYUp);
    cal3d::applyCoordinateTransform(p3, zUpToYUp);

    CHECK_CALPOINT4_CLOSE(p1, CalPoint4(1.0f, 0.0f, 0.0f), 0.000001f);
    CHECK_CALPOINT4_CLOSE(p2, CalPoint4(0.0f, 0.0f, -1.0f), 0.000001f);
    CHECK_CALPOINT4_CLOSE(p3, CalPoint4(0.0f, 1.0f, 0.0f), 0.000001f);
}

TEST(applyCoordinateTransformToVector4) {
    CalQuaternion zUpToYUp(-0.70710678f, 0.0f, 0.0f, 0.70710678f);
    CalVector4 v1(1.0f, 0.0f, 0.0f);
    CalVector4 v2(0.0f, 1.0f, 0.0f);
    CalVector4 v3(0.0f, 0.0f, 1.0f);

    cal3d::applyCoordinateTransform(v1, zUpToYUp);
    cal3d::applyCoordinateTransform(v2, zUpToYUp);
    cal3d::applyCoordinateTransform(v3, zUpToYUp);

    CHECK_CALVECTOR4_CLOSE(v1, CalPoint4(1.0f, 0.0f, 0.0f), 0.000001f);
    CHECK_CALVECTOR4_CLOSE(v2, CalPoint4(0.0f, 0.0f, -1.0f), 0.000001f);
    CHECK_CALVECTOR4_CLOSE(v3, CalPoint4(0.0f, 1.0f, 0.0f), 0.000001f);
}
