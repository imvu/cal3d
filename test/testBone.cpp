#include "TestPrologue.h"
#include <cal3d/bone.h>
#include <cal3d/corebone.h>

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
    BoneTransform bt = bone.calculateState(&bone);
    CHECK_EQUAL(1.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_one_full_animation_returns_animation) {
    bone.blendState(1.0f, makeTranslation(0, 0, 0), 1.0f, false, 1.0f);
    BoneTransform bt = bone.calculateState(&bone);
    CHECK_EQUAL(0.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_scaled_animation_returns_blend_of_bind_position_and_animation) {
    bone.blendState(1.0f, makeTranslation(0, 0, 0), 0.5f, false, 1.0f);
    BoneTransform bt = bone.calculateState(&bone);
    CHECK_EQUAL(0.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_two_replacements_uses_first_replacement) {
    bone.blendState(1.0f, makeTranslation(2, 2, 2), 1.0f, true, 1.0f);
    bone.blendState(1.0f, makeTranslation(0, 0, 0), 1.0f, true, 1.0f);
    BoneTransform bt = bone.calculateState(&bone);
    CHECK_EQUAL(2.0f, bt.rowx.w);
}

TEST_F(BoneFixture, bone_with_two_replacements_uses_first_replacement_partially_scaled) {
    bone.blendState(1.0f, makeTranslation(2, 2, 2), 1.0f, true, 0.8f);
    bone.blendState(1.0f, makeTranslation(10, 10, 10), 1.0f, true, 1.0f);
    BoneTransform bt = bone.calculateState(&bone);
    CHECK_EQUAL(3.6f, bt.rowx.w);
}
