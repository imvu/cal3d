#include "TestPrologue.h"


TEST(CalSubmesh_getFaces_succeeds_if_face_list_is_empty) {
    CalCoreSubmesh csm;
    CHECK(csm.create());

    CalSubmesh sm;
    sm.create(&csm);

    CHECK_EQUAL(0, sm.getFaces(0));
}
