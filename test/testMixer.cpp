#include "TestPrologue.h"
#include <cal3d/mixer.h>
#include <cal3d/corebone.h>
#include <cal3d/coreanimation.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/skeleton.h>
#include <cal3d/streamops.h>

FIXTURE(MixerFixture) {
    SETUP(MixerFixture)
        : coreSkeleton(fakeMixerSkeleton())
        , skeleton(coreSkeleton)
    {}

    static CalCoreSkeletonPtr fakeMixerSkeleton() {
        CalCoreSkeletonPtr p(new CalCoreSkeleton());
        CalCoreBonePtr root(new CalCoreBone("root"));
        p->addCoreBone(root);
        return p;
    }

    void updateSkeleton() {
        mixer.updateSkeleton(
            &skeleton,
            std::vector<BoneTransformAdjustment>(),
            std::vector<BoneScaleAdjustment>(),
            IncludeRootTransform);
    }

    CalCoreSkeletonPtr coreSkeleton;
    CalSkeleton skeleton;
    CalMixer mixer;
};

TEST_F(MixerFixture, no_animation_leaves_bone_in_bind_pose) {
    updateSkeleton();
    CHECK_EQUAL(cal3d::Transform(), skeleton.bones[0].absoluteTransform);
}

static CalAnimationPtr makeAnimation(CalVector t) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, t, CalQuaternion()));
    CalCoreTrack track(0, keyframes);

    CalCoreAnimationPtr coreAnimation(new CalCoreAnimation());
    coreAnimation->tracks.push_back(track);

    CalAnimationPtr anim(new CalAnimation(coreAnimation));
    return anim;
}

TEST_F(MixerFixture, one_low_priority_animation_poses_bone) {
    CalAnimationPtr anim(makeAnimation(CalVector(1, 1, 1)));

    AnimationAttributes attr;
    attr.priority_ = 0;
    attr.time_ = 0.0f;
    attr.weight_ = 1.0f;
    attr.rampValue_ = 1.0f;

    mixer.addManualAnimation(anim);
    mixer.setManualAnimationAttributes(anim, attr);
    updateSkeleton();
    CHECK_EQUAL(CalVector(1, 1, 1), skeleton.bones[0].absoluteTransform.translation);
}

TEST_F(MixerFixture, high_priority_animation_trumps_low_priority_animation) {
    CalAnimationPtr low(makeAnimation(CalVector(1, 1, 1)));
    CalAnimationPtr high(makeAnimation(CalVector(-1, -1, -1)));

    AnimationAttributes attr;
    attr.priority_ = 0;
    attr.time_ = 0.0f;
    attr.weight_ = 1.0f;
    attr.rampValue_ = 1.0f;

    mixer.addManualAnimation(low);
    mixer.setManualAnimationAttributes(low, attr);

    attr.priority_ = 2;
    attr.time_ = 0.0f;
    attr.weight_ = 1.0f;
    attr.rampValue_ = 1.0f;

    mixer.addManualAnimation(high);
    mixer.setManualAnimationAttributes(high, attr);

    updateSkeleton();
    CHECK_EQUAL(CalVector(-1, -1, -1), skeleton.bones[0].absoluteTransform.translation);
}
