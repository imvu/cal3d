#include "TestPrologue.h"
#include <cal3d/mixer.h>
#include <cal3d/corebone.h>
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

    CalCoreSkeletonPtr coreSkeleton;
    CalSkeleton skeleton;
    CalMixer mixer;
};

TEST_F(MixerFixture, no_animation_leaves_bone_in_bind_pose) {
    mixer.updateSkeleton(
        &skeleton,
        std::vector<BoneTransformAdjustment>(),
        std::vector<BoneScaleAdjustment>(),
        IncludeRootTransform);
    CHECK_EQUAL(cal3d::Transform(), skeleton.bones[0].absoluteTransform);
}
