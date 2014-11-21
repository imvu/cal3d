#include "TestPrologue.h"
#include <cal3d/coretrack.h>
#include <cal3d/corekeyframe.h>

FIXTURE(AnimationCompressionFixture) {
};

TEST_F(AnimationCompressionFixture, empty_track_compresses_to_empty_track) {
    CalCoreTrack t(0, CalCoreTrack::KeyframeList());
    CalCoreTrackPtr p = t.compress(0.1, 0.1, 0);
    CHECK_EQUAL(0u, p->keyframes.size());
}

TEST_F(AnimationCompressionFixture, one_frame_track_compresses_to_itself) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(), CalQuaternion()));
    CalCoreTrack t(0, keyframes);
    CalCoreTrackPtr p = t.compress(0.1, 0.1, 0);
    CHECK_EQUAL(1u, p->keyframes.size());
}

TEST_F(AnimationCompressionFixture, four_frames_are_merged_if_identical) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(0, CalVector(), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(0, CalVector(), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(0, CalVector(), CalQuaternion()));

    CalCoreTrack t(0, keyframes);
    CalCoreTrackPtr p = t.compress(0.1, 0.1, 0);
    CHECK_EQUAL(2u, p->keyframes.size());
}
