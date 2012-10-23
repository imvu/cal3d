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

TEST_F(TrackFixture, applyZupToYup) {
    CalCoreTrack::KeyframeList keyframes;
    CalQuaternion quat;
    quat.setAxisAngle(CalVector(0,1,0), DegreesToRadians(45.0f));    
    cal3d::RotateTranslate keyframeTransformOriginal( quat, CalVector(1.0f,2.0f,3.0f));
    keyframes.push_back(CalCoreKeyframe(0, keyframeTransformOriginal.translation, keyframeTransformOriginal.rotation));
    CalCoreTrack track(0, keyframes);
    track.applyZupToYup();
    CalCoreTrack::KeyframeList kfl = track.keyframes;
    const CalCoreKeyframe &kf = kfl[0];
    cal3d::applyZupToYup(keyframeTransformOriginal);
    CHECK_EQUAL(keyframeTransformOriginal.translation, kf.transform.translation);
    CHECK_EQUAL(keyframeTransformOriginal.rotation, kf.transform.rotation);
}

TEST_F(TrackFixture, applyCoordinateTransform) {
    // Note that cal3d quaternions are "left-handed", so that the following positive rotation is, in fact, the clockwise
    // rotation needed to convert z-up coordinates to y-up.
    CalQuaternion zUpToYUp(0.70710678f, 0.0f, 0.0f, 0.70710678f);
    CalCoreAnimation animation;
    CalCoreTrack::KeyframeList keyframes;
    CalQuaternion quat;
    quat.setAxisAngle(CalVector(0,1,0), DegreesToRadians(45.0f));
    cal3d::RotateTranslate keyframeTransformOriginal( quat, CalVector(1.0f,2.0f,3.0f));
    keyframes.push_back(CalCoreKeyframe(0, keyframeTransformOriginal.translation, keyframeTransformOriginal.rotation));
    animation.tracks.push_back(CalCoreTrack(0, keyframes));
    animation.applyCoordinateTransform(zUpToYUp);
    CalCoreTrack ktr = animation.tracks[0];
    CalCoreTrack::KeyframeList kfl = ktr.keyframes;
    const CalCoreKeyframe &kf = kfl[0];
    cal3d::applyCoordinateTransform(keyframeTransformOriginal, zUpToYUp);
    CHECK_CALVECTOR_CLOSE(keyframeTransformOriginal.translation, kf.transform.translation, 0.000001);
    CHECK_CALQUATERNION_CLOSE(keyframeTransformOriginal.rotation, kf.transform.rotation, 0.000001);
//    CHECK_CALVECTOR4_CLOSE(keyframeTransformOriginal.rotation.asCalVector4(), kf.transform.rotation.asCalVector4(), 0.000001)
//    CHECK_CLOSE(0.0f, ((keyframeTransformOriginal.rotation.asCalVector4())-(kf.transform.rotation.asCalVector4())).length(), 0.000001)
}

