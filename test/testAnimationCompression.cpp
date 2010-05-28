#include "TestPrologue.h"
#include <cal3d/coretrack.h>
#include <cal3d/corekeyframe.h>

TEST(empty_track_compresses_to_empty_track) {
    CalCoreTrack t(0, CalCoreTrack::KeyframeList());
    CalCoreTrackPtr p = t.compress(0.1, 0.1, 0);
    CHECK_EQUAL(0, p->keyframes.size());
}

TEST(one_frame_track_compresses_to_itself) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(), CalQuaternion()));
    CalCoreTrack t(0, keyframes);
    CalCoreTrackPtr p = t.compress(0.1, 0.1, 0);
    CHECK_EQUAL(1, p->keyframes.size());
}

TEST(four_frames_are_merged_if_identical) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(0, CalVector(), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(0, CalVector(), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(0, CalVector(), CalQuaternion()));

    CalCoreTrack t(0, keyframes);
    CalCoreTrackPtr p = t.compress(0.1, 0.1, 0);
    CHECK_EQUAL(2, p->keyframes.size());
}
