#include "TestPrologue.h"
#include <cal3d/coreanimation.h>
#include <cal3d/corebone.h>
#include <cal3d/coremesh.h>
#include <cal3d/coresubmesh.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/matrix.h>
#include <cal3d/streamops.h>

TEST(coreskeleton_can_only_have_one_root) {
    CalCoreBonePtr root1(new CalCoreBone("root1"));
    CalCoreBonePtr root2(new CalCoreBone("root2"));
    
    CalCoreSkeleton cs;
    cs.addCoreBone(root1);
    cs.addCoreBone(root2);

    CHECK_EQUAL(-1, cs.coreBones[0]->parentId);
    CHECK_EQUAL(0, cs.coreBones[1]->parentId);

    CHECK_EQUAL(0, root2->parentId);
}

TEST(coreskeleton_second_roots_are_transformed_relative_to_first_root) {
    CalCoreBonePtr root1(new CalCoreBone("root1"));
    root1->relativeTransform.translation = CalVector(1, 1, 1);
    CalCoreBonePtr root2(new CalCoreBone("root2"));
    root2->relativeTransform.translation = CalVector(-1, -1, -1);
    
    CalCoreSkeleton cs;
    cs.addCoreBone(root1);
    cs.addCoreBone(root2);

    CHECK_EQUAL(root2, cs.coreBones[1]);
    CHECK_EQUAL(0, root2->parentId);
    CHECK_EQUAL(CalVector(-2, -2, -2), root2->relativeTransform.translation);
}

TEST(loader_topologically_sorts) {
    std::vector<CalCoreBonePtr> bones;
    bones.push_back(CalCoreBonePtr(new CalCoreBone("a", 1)));
    bones.push_back(CalCoreBonePtr(new CalCoreBone("b", -1)));
    CalCoreSkeleton cs(bones);

    CHECK_EQUAL(cs.coreBones[0].get(), bones[1].get());
    CHECK_EQUAL(cs.coreBones[1].get(), bones[0].get());

    CHECK_EQUAL(1u, cs.boneIdTranslation[0]);
    CHECK_EQUAL(0u, cs.boneIdTranslation[1]);
}

TEST(addCoreBone_adds_id_translation_to_table) {
    CalCoreSkeleton skeleton;
    CalCoreBonePtr bone(new CalCoreBone("a"));
    skeleton.addCoreBone(bone);
    CHECK_EQUAL(skeleton.coreBones.size(), skeleton.boneIdTranslation.size());
    CHECK_EQUAL(0u, skeleton.boneIdTranslation[0]);
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
    CHECK_EQUAL(0, cs.coreBones[1]->parentId);
}

TEST(loader_disregards_paths_to_out_of_range) {
    std::vector<CalCoreBonePtr> bones;
    bones.push_back(CalCoreBonePtr(new CalCoreBone("a", 2)));
    bones.push_back(CalCoreBonePtr(new CalCoreBone("b", 0)));
    CalCoreSkeleton cs(bones);

    CHECK_EQUAL(2u, cs.coreBones.size());
    CHECK_EQUAL(-1, cs.coreBones[0]->parentId);
    CHECK_EQUAL(0, cs.coreBones[1]->parentId);
}

TEST(topologically_sorted_skeletons_can_fixup_mesh_references) {
    std::vector<CalCoreBonePtr> bones;
    bones.push_back(CalCoreBonePtr(new CalCoreBone("a", 1)));
    bones.push_back(CalCoreBonePtr(new CalCoreBone("b", -1)));
    CalCoreSkeletonPtr cs(new CalCoreSkeleton(bones));

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> influences;
    influences.push_back(CalCoreSubmesh::Influence(0, 0, false));
    influences.push_back(CalCoreSubmesh::Influence(1, 1, true));

    CalCoreSubmeshPtr csm(new CalCoreSubmesh(1, 1, 1));
    csm->addVertex(v, 0, influences);

    CalCoreMesh cm;
    cm.submeshes.push_back(csm);
    cm.fixup(cs);

    influences = csm->getInfluences();
    CHECK_EQUAL(2u, influences.size());
    CHECK_EQUAL(0u, influences[0].boneId);
    CHECK_EQUAL(1u, influences[1].boneId);

    CalMatrix identity;
    BoneTransform bt[2] = {
        BoneTransform(identity, CalVector(0, 0, 0)),
        BoneTransform(identity, CalVector(1, 1, 1)),
    };

    CHECK(csm->isStatic());
    BoneTransform staticTransform = csm->getStaticTransform(bt);
    CHECK_EQUAL(0.0f, staticTransform.rowx.w);
}

TEST(fixup_out_of_range_influences_resets_to_zero) {
    std::vector<CalCoreBonePtr> bones;
    bones.push_back(CalCoreBonePtr(new CalCoreBone("a", 1)));
    bones.push_back(CalCoreBonePtr(new CalCoreBone("b", -1)));
    CalCoreSkeletonPtr cs(new CalCoreSkeleton(bones));

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> influences;
    influences.push_back(CalCoreSubmesh::Influence(2, 1, true));

    CalCoreSubmeshPtr csm(new CalCoreSubmesh(1, 1, 1));
    csm->addVertex(v, 0, influences);

    CalCoreMesh cm;
    cm.submeshes.push_back(csm);
    cm.fixup(cs);

    influences = csm->getInfluences();
    CHECK_EQUAL(1u, influences.size());
    CHECK_EQUAL(0u, influences[0].boneId);
}

TEST(topologically_sorted_skeletons_can_fixup_animations) {
    std::vector<CalCoreBonePtr> bones;
    bones.push_back(CalCoreBonePtr(new CalCoreBone("a", 1)));
    bones.push_back(CalCoreBonePtr(new CalCoreBone("b", -1)));
    CalCoreSkeletonPtr cs(new CalCoreSkeleton(bones));

    CalCoreTrack ct(0, std::vector<CalCoreKeyframe>());

    CalCoreAnimation ca;
    ca.tracks.push_back(ct);

    ca.fixup(cs);
    CHECK_EQUAL(1u, ca.tracks[0].coreBoneId);
}
