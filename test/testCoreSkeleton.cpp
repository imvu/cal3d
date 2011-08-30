#include "TestPrologue.h"
#include <cal3d/corebone.h>
#include <cal3d/coreskeleton.h>

TEST(loader_topologically_sorts) {
    std::vector<CalCoreBonePtr> bones;
    bones.push_back(CalCoreBonePtr(new CalCoreBone("a", 1)));
    bones.push_back(CalCoreBonePtr(new CalCoreBone("b", -1)));
    CalCoreSkeleton cs(bones);

    CHECK_EQUAL(cs.coreBones[0].get(), bones[1].get());
    CHECK_EQUAL(cs.coreBones[1].get(), bones[0].get());
}

TEST(loader_disregards_self_parents) {
    std::vector<CalCoreBonePtr> bones;
    bones.push_back(CalCoreBonePtr(new CalCoreBone("a", 0)));
    CalCoreSkeleton cs(bones);

    CHECK_EQUAL(1u, cs.coreBones.size());
    CHECK_EQUAL(-1, cs.coreBones[0]->parentId);
}

TEST(loader_disregards_out_of_range_parents) {
    std::vector<CalCoreBonePtr> bones;
    bones.push_back(CalCoreBonePtr(new CalCoreBone("a", 1)));
    CalCoreSkeleton cs(bones);

    CHECK_EQUAL(1u, cs.coreBones.size());
    CHECK_EQUAL(-1, cs.coreBones[0]->parentId);
}

TEST(loader_disregards_cyclic_parents) {
    std::vector<CalCoreBonePtr> bones;
    bones.push_back(CalCoreBonePtr(new CalCoreBone("a", 1)));
    bones.push_back(CalCoreBonePtr(new CalCoreBone("b", 0)));
    CalCoreSkeleton cs(bones);

    CHECK_EQUAL(2u, cs.coreBones.size());
    CHECK_EQUAL(-1, cs.coreBones[0]->parentId);
    CHECK_EQUAL(-1, cs.coreBones[1]->parentId);
}

TEST(loader_disregards_paths_to_out_of_range) {
    std::vector<CalCoreBonePtr> bones;
    bones.push_back(CalCoreBonePtr(new CalCoreBone("a", 2)));
    bones.push_back(CalCoreBonePtr(new CalCoreBone("b", 0)));
    CalCoreSkeleton cs(bones);

    CHECK_EQUAL(2u, cs.coreBones.size());
    CHECK_EQUAL(-1, cs.coreBones[0]->parentId);
    CHECK_EQUAL(-1, cs.coreBones[1]->parentId);
}


#if 0
TEST(can_fixup_meshes) {
}

TEST(can_fixup_animations) {
}
#endif
