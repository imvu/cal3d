#include <cal3d/color.h>
#include "TestPrologue.h"


inline std::ostream& operator<<(std::ostream& os, const CalVector& v) {
  return os << '(' << v.x << ', ' << v.y << ', ' << v.z << ')';
}


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
