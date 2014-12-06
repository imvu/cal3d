#include "TestPrologue.h"

#include <cal3d/trisort.h>

TEST(triangle_sorting_works) {
    unsigned short indices[12] = {
        9,10,11,
        6,7,8,
        0,1,2,
        3,4,5,
    };

    float positions[36] = {
        0,0,0, 1,0,0, 0,1,0,
        0,0,1, 1,0,1, 0,1,1,
        0,0,2, 1,0,2, 0,1,2,
        0,0,3, 1,0,3, 0,1,3,
    };

    sortTrianglesBackToFront(4, indices, positions);

    CHECK_EQUAL(0, indices[0]);
    CHECK_EQUAL(1, indices[1]);
    CHECK_EQUAL(2, indices[2]);

    CHECK_EQUAL(3, indices[3]);
    CHECK_EQUAL(4, indices[4]);
    CHECK_EQUAL(5, indices[5]);

    CHECK_EQUAL(6, indices[6]);
    CHECK_EQUAL(7, indices[7]);
    CHECK_EQUAL(8, indices[8]);

    CHECK_EQUAL(9, indices[9]);
    CHECK_EQUAL(10, indices[10]);
    CHECK_EQUAL(11, indices[11]);
}
