#include "TestPrologue.h"
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
