//****************************************************************************//
// coretrack.cpp                                                              //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cal3d/coretrack.h"
#include "cal3d/corebone.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/error.h"
#include "cal3d/corekeyframe.h"
#include "cal3d/loader.h"

bool sortByTime(const CalCoreKeyframe& lhs, const CalCoreKeyframe& rhs) {
    return lhs.time < rhs.time;
}

CalCoreTrack::KeyframeList sorted(CalCoreTrack::KeyframeList ls) {
    std::sort(ls.begin(), ls.end(), sortByTime);
    return ls;
}

CalCoreTrack::CalCoreTrack(int coreBone, const KeyframeList& kf)
    : coreBoneId(coreBone)
    , keyframes(sorted(kf)) {
    translationRequired = true;
    translationIsDynamic = true;
}

size_t sizeInBytes(const CalCoreKeyframe&) {
    return sizeof(CalCoreKeyframe);
}

size_t CalCoreTrack::sizeInBytes() const {
    return sizeof(CalCoreTrack) + ::sizeInBytes(keyframes);
}

inline float DistanceSquared(CalVector const& v1, CalVector const& v2) {
    float dx = (v1.x - v2.x);
    float dy = (v1.y - v2.y);
    float dz = (v1.z - v2.z);
    return dx * dx + dy * dy + dz * dz;
}

inline float Distance(CalVector const& p1, CalVector const& p2) {
    return sqrtf(DistanceSquared(p1, p2));
}

float DistanceDegrees(CalQuaternion const& p1, CalQuaternion const& p2) {

    // To determine the angular distance between the oris, multiply one by the inverse
    // of the other, which should leave us with an identity ori if they are equal.  If
    // they are not equal, then the angular magnitude of the rotation in degrees is the
    // difference between the oris.
    CalQuaternion odist = p1;
    odist.invert();
    odist *= p2;
    float w = odist.w;
    if (w > 1) {
        w = 1;
    }
    if (w < -1) {
        w = -1;
    }
    float distrads = 2 * acos(w);   // Non-negative.
    float distdegrees = distrads * 180.0f / 3.141592654f; // Non-negative.
    if (distdegrees > 180.0) {
        distdegrees -= 360.0;
    }
    return fabsf(distdegrees);
}

static bool Near(
    CalVector const& p1, CalQuaternion const& q1, CalVector const& p2, CalQuaternion const& q2,
    double transTolerance,
    double rotTolerance
) {
    float distdegrees = DistanceDegrees(q1, q2);
    if (distdegrees > rotTolerance) {
        return false;
    }
    float dist = Distance(p1, p2);
    if (dist > transTolerance) {
        return false;
    }
    return true;
}


static bool keyframeEliminatable(
    const CalCoreKeyframe* prev,
    const CalCoreKeyframe* p,
    const CalCoreKeyframe* next,
    double transTolerance,
    double rotTolerance
) {
    assert(prev && p && next);
    float blendFactor = (p->time - prev->time) / (next->time - prev->time);

    // blend between the two keyframes
    CalVector translation = prev->translation;
    translation.blend(blendFactor, next->translation);
    CalQuaternion rotation = prev->rotation;
    rotation.blend(blendFactor, next->rotation);
    return Near(translation, rotation, p->translation, p->rotation, transTolerance, rotTolerance);
}



struct KeyLink {
    bool eliminated_;
    const CalCoreKeyframe* keyframe_;
    KeyLink* next_;
};


unsigned int
KeyFrameSequenceLength(KeyLink* p, double transTolerance, double rotTolerance) {
    CalVector translation = p->keyframe_->translation;
    CalQuaternion rotation = p->keyframe_->rotation;
    p = p->next_;
    unsigned int len = 1;
    while (p) {
        CalVector const ppos = p->keyframe_->translation;
        CalQuaternion const pori = p->keyframe_->rotation;
        if (Near(translation, rotation, ppos, pori, transTolerance, rotTolerance)) {
            len++;
            p = p->next_;
        } else {
            break;
        }
    }
    return len;
}


CalCoreTrackPtr CalCoreTrack::compress(
    double translationTolerance,
    double rotationToleranceDegrees,
    CalCoreSkeleton* skelOrNull
) const {
    size_t numFrames = keyframes.size();
    if (!numFrames) {
        return CalCoreTrackPtr(new CalCoreTrack(coreBoneId, KeyframeList()));
    }

    // I want to iterate through the vector as a list, and remove elements easily.
    std::vector<KeyLink> keyLinkArray(numFrames);
    for (size_t i = 0; i < numFrames; i++) {
        KeyLink* kl = & keyLinkArray[ i ];
        kl->keyframe_ = &keyframes[ i ];
        kl->next_ = (i == numFrames - 1) ? NULL : & keyLinkArray[ i + 1 ];
        kl->eliminated_ = false;
    }

    // Iterate until quiescence.
    bool removedFrame = true;
    while (removedFrame) {
        removedFrame = false;

        // Loop through the frames, starting with the second, ending with the second to last.
        // If the frame is approximately the same as the interpolated frame between its prev
        // and next frame, then eliminate it.
        KeyLink* prev = & keyLinkArray[ 0 ];
        while (true) {
            KeyLink* p = prev->next_;
            if (!p || !p->next_) {
                break;
            }
            KeyLink* next = p->next_;
            if (keyframeEliminatable(prev->keyframe_,
                                     p->keyframe_,
                                     next->keyframe_,
                                     translationTolerance, rotationToleranceDegrees)) {
                p->eliminated_ = true;

                // Splice this eliminated keyframe out of the list.
                prev->next_ = next;

                // Pass over next frame, making it prev instead of making it p, since I don't
                // want to consider eliminating two key frames in a row because that can
                // reduce a slow moving curve to a line no matter the arc
                // of the curve.
                prev = next;
                removedFrame = true;
            } else {
                prev = p;
            }
        }
    }

    KeyframeList output;

    // Rebuild the vector, freeing any of the eliminated keyframes.
    for (unsigned i = 0; i < numFrames; i++) {
        KeyLink* kl = & keyLinkArray[ i ];
        if (!kl->eliminated_) {
            output.push_back(CalCoreKeyframe(*kl->keyframe_));
        }
    }

    CalCoreTrackPtr result(new CalCoreTrack(coreBoneId, output));

    // Update the flag saying whether the translation, which I have loaded, is actually required.
    // If translation is not required, I can't do any better than that so I leave it alone.
    if (skelOrNull && translationRequired) {
        result->translationCompressibility(
            &result->translationRequired,
            &result->translationIsDynamic,
            translationTolerance, CalLoader::keyframePosRangeSmall, skelOrNull);
    }

    return result;
}



void CalCoreTrack::translationCompressibility(
    bool* transRequiredResult,
    bool* transDynamicResult,
    float threshold,
    float highRangeThreshold,
    CalCoreSkeleton* skel
) const {
    * transRequiredResult = false;
    * transDynamicResult = false;
    int numFrames = keyframes.size();
    CalCoreBone* cb = skel->coreBones[coreBoneId].get();
    const CalVector& cbtrans = cb->getTranslation();
    CalVector trans0;
    float t2 = threshold * threshold;
    unsigned int i;
    for (i = 0; i < numFrames; i++) {
        const CalCoreKeyframe* keyframe = &keyframes[i];
        const CalVector& kftrans = keyframe->translation;
        if (i == 0) {
            trans0 = keyframe->translation;
        } else {
            float d2 = DistanceSquared(trans0, kftrans);
            if (d2 > t2) {
                * transDynamicResult = true;
            }
        }
        float d2 = DistanceSquared(cbtrans, kftrans);
        if (d2 > t2) {
            * transRequiredResult = true;
        }
    }
}

void CalCoreTrack::getState(float time, CalVector& translation, CalQuaternion& rotation) const {
    if (keyframes.empty()) {
        translation.clear();
        rotation.clear();
        return;
    }

    KeyframeList::const_iterator iteratorCoreKeyframeAfter = getUpperBound(time);

    if (iteratorCoreKeyframeAfter == keyframes.end()) {
        --iteratorCoreKeyframeAfter;
        rotation = iteratorCoreKeyframeAfter->rotation;
        translation = iteratorCoreKeyframeAfter->translation;
        return;
    }

    if (iteratorCoreKeyframeAfter == keyframes.begin()) {
        rotation = iteratorCoreKeyframeAfter->rotation;
        translation = iteratorCoreKeyframeAfter->translation;
        return;
    }

    KeyframeList::const_iterator iteratorCoreKeyframeBefore = iteratorCoreKeyframeAfter;
    --iteratorCoreKeyframeBefore;

    const CalCoreKeyframe& pCoreKeyframeBefore = *iteratorCoreKeyframeBefore;
    const CalCoreKeyframe& pCoreKeyframeAfter  = *iteratorCoreKeyframeAfter;

    float blendFactor = 0.0;
    if (pCoreKeyframeAfter.time != pCoreKeyframeBefore.time) {
        blendFactor = (time - pCoreKeyframeBefore.time) / (pCoreKeyframeAfter.time - pCoreKeyframeBefore.time);
    }

    translation = pCoreKeyframeBefore.translation;
    translation.blend(blendFactor, pCoreKeyframeAfter.translation);

    rotation = pCoreKeyframeBefore.rotation;
    rotation.blend(blendFactor, pCoreKeyframeAfter.rotation);
}

CalCoreTrack::KeyframeList::const_iterator CalCoreTrack::getUpperBound(float time) const {
    int lowerBound = 0;
    int upperBound = keyframes.size() - 1;

    while (lowerBound < upperBound - 1) {
        int middle = (lowerBound + upperBound) / 2;

        if (time >= keyframes[middle].time) {
            lowerBound = middle;
        } else {
            upperBound = middle;
        }
    }

    return keyframes.begin() + upperBound;

}
