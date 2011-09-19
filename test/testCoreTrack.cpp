#include "TestPrologue.h"
#include <cal3d/coretrack.h>
#include <cal3d/streamops.h>

FIXTURE(TrackFixture) {
    cal3d::RotateTranslate t;
};

TEST_F(TrackFixture, getState_returns_empty_if_no_keyframes) {
    CalCoreTrack track(0, CalCoreTrack::KeyframeList());
    t = track.getState(-1);
    CHECK_EQUAL(CalVector(), t.translation);

    t = track.getState(1);
    CHECK_EQUAL(cal3d::RotateTranslate(), t);
}

TEST_F(TrackFixture, getState_blends_keyframes) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(2, 2, 2), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(1, CalVector(4, 4, 4), CalQuaternion()));
    CalCoreTrack track(0, keyframes);

    t = track.getState(-1);
    CHECK_EQUAL(CalVector(2, 2, 2), t.translation);

    t = track.getState(0);
    CHECK_EQUAL(CalVector(2, 2, 2), t.translation);

    t = track.getState(0.5);
    CHECK_EQUAL(CalVector(3, 3, 3), t.translation);

    t = track.getState(1);
    CHECK_EQUAL(CalVector(4, 4, 4), t.translation);

    t = track.getState(2);
    CHECK_EQUAL(CalVector(4, 4, 4), t.translation);
}

TEST_F(TrackFixture, getState_with_two_keyframes_at_same_time) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(2, 2, 2), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(1, CalVector(4, 4, 4), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(1, CalVector(6, 6, 6), CalQuaternion()));
    CalCoreTrack track(0, keyframes);

    t = track.getState(1.5);
    CHECK_EQUAL(CalQuaternion(), t.rotation);
    CHECK_EQUAL(CalVector(6, 6, 6), t.translation);
}
