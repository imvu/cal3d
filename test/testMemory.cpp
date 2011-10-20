#include <TestFramework/TestFramework.h>
#include "cal3d/memory.h"

TEST(can_grow_SSEArray) {
    cal3d::SSEArray<__m128> v;
    v.push_back(__m128());
}
