#include "TestPrologue.h"
#include <libzero/Math.h>
#include <cal3d/coretrack.h>
#include <cal3d/transform.h>
#include <cal3d/streamops.h>

FIXTURE(TrackFixture) {
    cal3d::RotateTranslate t;
};

TEST_F(TrackFixture, getState_returns_empty_if_no_keyframes) {
    CalCoreTrack track(0, CalCoreTrack::KeyframeList());
    t = track.getCurrentTransform(-1);
    CHECK_EQUAL(CalVector(), t.translation);

    t = track.getCurrentTransform(1);
    CHECK_EQUAL(cal3d::RotateTranslate(), t);
}

TEST_F(TrackFixture, getState_blends_keyframes) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(2, 2, 2), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(1, CalVector(4, 4, 4), CalQuaternion()));
    CalCoreTrack track(0, keyframes);

    t = track.getCurrentTransform(-1);
    CHECK_EQUAL(CalVector(2, 2, 2), t.translation);

    t = track.getCurrentTransform(0);
    CHECK_EQUAL(CalVector(2, 2, 2), t.translation);

    t = track.getCurrentTransform(0.5);
    CHECK_EQUAL(CalVector(3, 3, 3), t.translation);

    t = track.getCurrentTransform(1);
    CHECK_EQUAL(CalVector(4, 4, 4), t.translation);

    t = track.getCurrentTransform(2);
    CHECK_EQUAL(CalVector(4, 4, 4), t.translation);
}

TEST_F(TrackFixture, optimize_removes_identical_frames) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(2, 2, 2), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(1, CalVector(2, 2, 2), CalQuaternion()));
    CalCoreTrack track(0, keyframes);
    track.optimize();

    CHECK_EQUAL(1u, track.keyframes.size());
    CHECK_EQUAL(CalVector(2, 2, 2), track.keyframes[0].transform.translation);
}

TEST_F(TrackFixture, optimize_leaves_different_frames) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(2, 2, 2), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(1, CalVector(4, 4, 4), CalQuaternion()));
    CalCoreTrack track(0, keyframes);
    track.optimize();
    CHECK_EQUAL(2u, track.keyframes.size());
}

TEST_F(TrackFixture, getState_with_one_keyframe_always_returns_that_keyframe) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(2, 2, 2), CalQuaternion()));
    CalCoreTrack track(0, keyframes);

    t = track.getCurrentTransform(-1);
    CHECK_EQUAL(CalVector(2, 2, 2), t.translation);

    t = track.getCurrentTransform(0);
    CHECK_EQUAL(CalVector(2, 2, 2), t.translation);

    t = track.getCurrentTransform(1);
    CHECK_EQUAL(CalVector(2, 2, 2), t.translation);
}

TEST_F(TrackFixture, getState_with_two_keyframes_at_same_time) {
    CalCoreTrack::KeyframeList keyframes;
    keyframes.push_back(CalCoreKeyframe(0, CalVector(2, 2, 2), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(1, CalVector(4, 4, 4), CalQuaternion()));
    keyframes.push_back(CalCoreKeyframe(1, CalVector(6, 6, 6), CalQuaternion()));
    CalCoreTrack track(0, keyframes);

    t = track.getCurrentTransform(1.5);
    CHECK_EQUAL(CalQuaternion(), t.rotation);
    CHECK_EQUAL(CalVector(6, 6, 6), t.translation);
}
