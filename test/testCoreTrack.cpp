#include "TestPrologue.h"
#include <cal3d/coretrack.h>
#include <cal3d/streamops.h>

FIXTURE(TrackFixture) {
    CalVector t;
    CalQuaternion q;
};

TEST_F(TrackFixture, getState_returns_empty_if_no_keyframes) {
    CalCoreTrack track(0, CalCoreTrack::KeyframeList());
    track.getState(-1, t, q);
    CHECK_EQUAL(t, CalVector());

    t = CalVector(1, 2, 3);
    track.getState(1, t, q);
    CHECK_EQUAL(t, CalVector());
}

TEST_F(TrackFixture, getState_blends_keyframes) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(2, 2, 2), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(1, CalVector(4, 4, 4), CalQuaternion()));
    CalCoreTrack track(0, keyframes);

    // this behavior makes no sense.  why would it extrapolate off the end of the keyframes?
    track.getState(-1, t, q);
    CHECK_EQUAL(CalVector(0, 0, 0), t);

    track.getState(0.5, t, q);
    CHECK_EQUAL(CalVector(3, 3, 3), t);

    // this behavior makes no sense.  why would it extrapolate off the end of the keyframes?
    track.getState(2, t, q);
    CHECK_EQUAL(CalVector(6, 6, 6), t);
}
