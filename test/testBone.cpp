#include "TestPrologue.h"
#include <cal3d/bone.h>
#include <cal3d/corebone.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/mixer.h>
#include <cal3d/skeleton.h>

static cal3d::Transform makeTranslation(float x, float y, float z) {
    return cal3d::Transform(CalQuaternion(), CalVector(x, y, z));
}

FIXTURE(BoneFixture) {
    SETUP(BoneFixture)
        : coreBone(testBone())
        , bone(coreBone)
    {}

    static CalCoreBone testBone() {
        CalCoreBone cb("test");
        // bind pose = 1.0f, 1.0f, 1.0f
        cb.relativeTransform = makeTranslation(1.0f, 1.0f, 1.0f);
        return cb;
    }

    CalCoreBone coreBone;
    CalBone bone;
};

TEST_F(BoneFixture, bone_with_no_animation_returns_bind_position) {
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(1.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_one_full_animation_returns_animation) {
    bone.blendPose(makeTranslation(0, 0, 0), false, 1.0f);
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(0.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_scaled_animation_returns_blend_of_bind_position_and_animation) {
    bone.blendPose(makeTranslation(0, 0, 0), false, 0.5f);
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(0.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_two_replacements_uses_first_replacement) {
    bone.blendPose(makeTranslation(2, 2, 2), true, 1.0f);
    bone.blendPose(makeTranslation(0, 0, 0), true, 1.0f);
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(2.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_two_replacements_uses_first_replacement_partially_scaled) {
    bone.blendPose(makeTranslation(2, 2, 2), true, 0.8f);
    bone.blendPose(makeTranslation(10, 10, 10), true, 1.0f);
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(3.6f, bt.rowx.w);
}

TEST_F(BoneFixture, can_ramp_in_at_zero) {
    bone.blendPose(makeTranslation(2, 2, 2), true, 0.0f);
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(1.0f, bt.rowx.w);    
}

FIXTURE(MixerFixture) {
    static CalCoreSkeletonPtr fakeSkeleton() {
        CalCoreBonePtr coreRoot(new CalCoreBone("root"));
        coreRoot->relativeTransform.translation.x = 1;
        coreRoot->boneSpaceTransform.translation.x = 10;

        CalCoreBonePtr coreBone(new CalCoreBone("bone", 0));
        coreBone->relativeTransform.translation.x = 2;
        coreBone->boneSpaceTransform.translation.x = 20;

        CalCoreSkeletonPtr cs(new CalCoreSkeleton);
        cs->addCoreBone(coreRoot);
        cs->addCoreBone(coreBone);
        return cs;
    }

    SETUP(MixerFixture)
        : skeleton(fakeSkeleton())
    {}

    CalSkeleton skeleton;
    CalMixer mixer;
};

TEST_F(MixerFixture, boneTransform_accumulates_transforms) {
    mixer.updateSkeleton(&skeleton, std::vector<BoneTransformAdjustment>(), std::vector<BoneScaleAdjustment>(), IncludeRootTransform);

    CHECK_EQUAL(11, skeleton.boneTransforms[0].rowx.w);
    CHECK_EQUAL(23, skeleton.boneTransforms[1].rowx.w);
}

TEST_F(MixerFixture, can_optionally_disregard_root_transform) {
    mixer.updateSkeleton(&skeleton, std::vector<BoneTransformAdjustment>(), std::vector<BoneScaleAdjustment>(), IgnoreRootTransform);

    CHECK_EQUAL(10, skeleton.boneTransforms[0].rowx.w);
    CHECK_EQUAL(22, skeleton.boneTransforms[1].rowx.w);
}
