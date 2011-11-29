#include "TestPrologue.h"
#include <cal3d/bone.h>
#include <cal3d/corebone.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/mixer.h>
#include <cal3d/skeleton.h>
#include <cal3d/streamops.h>

static cal3d::RotateTranslate makeTranslation(float x, float y, float z) {
    return cal3d::RotateTranslate(CalQuaternion(), CalVector(x, y, z));
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

TEST_F(BoneFixture, bone_with_zero_weight_is_replaced) {
    bone.blendPose(0.0f, makeTranslation(0, 0, 0), 0.0f);
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(1.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_one_full_animation_returns_animation) {
    bone.blendPose(1.0f, makeTranslation(0, 0, 0), 0.0f);
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(0.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_scaled_animation_returns_blend_of_bind_position_and_animation) {
    bone.blendPose(0.5f, makeTranslation(0, 0, 0), 0.0f);
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(0.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_two_replacements_uses_first_replacement) {
    bone.blendPose(1.0f, makeTranslation(2, 2, 2), 1.0f);
    bone.blendPose(1.0f, makeTranslation(0, 0, 0), 1.0f);
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(2.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_two_replacements_uses_first_replacement_partially_scaled) {
    bone.blendPose(0.8f, makeTranslation(2, 2, 2), 0.8f);
    bone.blendPose(1.0f, makeTranslation(10, 10, 10), 1.0f);
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(3.6f, bt.rowx.w);
}

inline float lerp(float a, float s, float t) {
    return a * t + (1.0f - a) * s;
}

TEST_F(BoneFixture, bone_with_two_replacements_and_one_nonreplacement_blends_more_to_the_first_replacement) {
    bone.blendPose(0.5f, makeTranslation(2, 2, 2), 0.5f);
    bone.blendPose(0.5f, makeTranslation(10, 10, 10), 0.5f);
    bone.blendPose(0.5f, makeTranslation(100, 100, 100), 0.5f);
    BoneTransform bt = bone.calculateAbsolutePose(&bone, true);
    CHECK_EQUAL(lerp(1.0f / 7.0f, lerp(1.0f / 3.0f, 2, 10), 100), bt.rowx.w);
}

FIXTURE(MixerFixture) {
    static CalCoreSkeletonPtr fakeSkeleton() {
        CalCoreBonePtr coreRoot(new CalCoreBone("root"));
        coreRoot->relativeTransform.translation.x = 1;
        coreRoot->inverseBindPoseTransform.translation = CalVector(10, 10, 10);

        CalCoreBonePtr coreBone(new CalCoreBone("bone", 0));
        coreBone->relativeTransform.translation.x = 2;
        coreBone->inverseBindPoseTransform.translation = CalVector(20, 20, 20);

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

inline CalVector transformPoint(const BoneTransform& bt, const CalVector& point) {
    return CalVector(
        bt.rowx.x * point.x + bt.rowx.y * point.y + bt.rowx.z * point.z + bt.rowx.w,
        bt.rowy.x * point.x + bt.rowy.y * point.y + bt.rowy.z * point.z + bt.rowy.w,
        bt.rowz.x * point.x + bt.rowz.y * point.y + bt.rowz.z * point.z + bt.rowz.w);
}

TEST_F(MixerFixture, boneTransform_accumulates_transforms_and_scales) {
    std::vector<BoneScaleAdjustment> boneScaleAdjustments;
    boneScaleAdjustments.push_back(BoneScaleAdjustment(0, CalVector(2, 2, 2)));
    boneScaleAdjustments.push_back(BoneScaleAdjustment(1, CalVector(0.5f, 0.5f, 0.5f)));
    mixer.updateSkeleton(&skeleton, std::vector<BoneTransformAdjustment>(), boneScaleAdjustments, IncludeRootTransform);

    CHECK_EQUAL(CalVector(25, 28, 36), transformPoint(skeleton.boneTransforms[0], CalVector(2, 4, 8)));
    CHECK_EQUAL(CalVector(27, 24, 28), transformPoint(skeleton.boneTransforms[1], CalVector(2, 4, 8)));
}

TEST(scale_is_in_bone_space) {
    CalQuaternion aboutZ;
    aboutZ.setAxisAngle(CalVector(0, 0, 1), 3.1415927410125732421875f / 2.0f);

    CHECK_EQUAL(CalVector(-4, 2, 8), aboutZ * CalVector(2, 4, 8));

    CalCoreBonePtr root(new CalCoreBone("root"));
    root->inverseBindPoseTransform = cal3d::RotateTranslate(aboutZ, CalVector(2, 4, 8));

    CalCoreSkeletonPtr coreSkeleton(new CalCoreSkeleton);
    coreSkeleton->addCoreBone(root);

    CalSkeleton skeleton(coreSkeleton);

    std::vector<BoneScaleAdjustment> boneScaleAdjustments;
    boneScaleAdjustments.push_back(BoneScaleAdjustment(0, CalVector(0.5f, 1, 2)));

    CalMixer mixer;
    mixer.updateSkeleton(&skeleton, std::vector<BoneTransformAdjustment>(), boneScaleAdjustments, IncludeRootTransform);

    CHECK_EQUAL(CalVector(-1, 6, 32), transformPoint(skeleton.boneTransforms[0], CalVector(2, 4, 8)));
}

TEST_F(MixerFixture, can_optionally_disregard_root_transform) {
    mixer.updateSkeleton(&skeleton, std::vector<BoneTransformAdjustment>(), std::vector<BoneScaleAdjustment>(), IgnoreRootTransform);

    CHECK_EQUAL(10, skeleton.boneTransforms[0].rowx.w);
    CHECK_EQUAL(22, skeleton.boneTransforms[1].rowx.w);
}
