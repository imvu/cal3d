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
