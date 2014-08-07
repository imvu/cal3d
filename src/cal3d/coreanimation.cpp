//****************************************************************************//
// coreanimation.cpp                                                          //
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

#include "cal3d/coreanimation.h"
#include "cal3d/coretrack.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h"
#include "cal3d/memory.h"

#include "cal3d/error.h"
#include "cal3d/bone.h"
#include "cal3d/coremesh.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/matrix.h"
#include "cal3d/skeleton.h"

size_t sizeInBytes(const CalCoreTrack& t) {
    return t.sizeInBytes();
}

size_t CalCoreAnimation::sizeInBytes() const {
    return sizeof(*this) + ::sizeInBytes(tracks);
}

const CalCoreTrack* CalCoreAnimation::getCoreTrack(unsigned coreBoneId) const {
    for (
        TrackList::const_iterator iteratorCoreTrack = tracks.begin();
        iteratorCoreTrack != tracks.end();
        ++iteratorCoreTrack
    ) {
        if (iteratorCoreTrack->coreBoneId == coreBoneId) {
            return &*iteratorCoreTrack;
        }
    }

    // no match found
    return 0;
}

void CalCoreAnimation::scale(float factor) {
    std::for_each(tracks.begin(), tracks.end(), std::bind2nd(std::mem_fun_ref(&CalCoreTrack::scale), factor));
}

void CalCoreAnimation::fixup(const CalCoreSkeletonPtr& skeleton, cal3d::RotateTranslate rt, bool doFingerFix) {
    const auto& coreBones = skeleton->coreBones;

    TrackList output;
    output.reserve(coreBones.size());

    //  finger fix indices
    int trkIndexRing02 = -1;
    int trkIndexRing03 = -1;
    int trkIndexPnky02 = -1;
    int trkIndexPnky03 = -1;

    for (auto i = tracks.begin(); i != tracks.end(); ++i) {
        if (i->coreBoneId >= coreBones.size()) {
            continue;
        }

        cal3d::verify(skeleton->coreBones.size() == skeleton->boneIdTranslation.size(), "coreBones and boneIdTranslation must be the same size");

        cal3d::verify(i->coreBoneId < coreBones.size(), "untranslated track ID out of the range of bones...");
        i->coreBoneId = skeleton->boneIdTranslation[i->coreBoneId];
        cal3d::verify(i->coreBoneId < coreBones.size(), "translated track ID out of the range of bones...");

        const auto& coreBone = *coreBones[i->coreBoneId];

        if (coreBone.parentId == -1) {
            i->zeroTransforms();
            i->rotateTranslate(rt);
        } else {
            i->fixup(
                coreBone,
                skeleton->getAdjustedRootTransform(i->coreBoneId));
        }

        //  check for the bones required to fix lfFingerRing
        if (doFingerFix && coreBone.name.substr(0,8)==std::string("lfFinger")) {
            if (coreBone.name == "lfFingerRing02") {
                trkIndexRing02 = output.size();
            }
            if (coreBone.name == "lfFingerRing03") {
                trkIndexRing03 = output.size();
            }
            if (coreBone.name == "lfFingerPinky02") {
                trkIndexPnky02 = output.size();
            }
            if (coreBone.name == "lfFingerPinky03") {
                trkIndexPnky03 = output.size();
            }
        }

        output.push_back(*i);
    }

    //  doFingerFix addresses the problem caused by animations that were exported during the time that the
    //  Pid80 Female was missing the 'lfFingerRing03' bone. If any animation was performed on that bone then
    //  the resulting anim file will have a bogus animation track that animates from lfFingerRing02 out
    //  to the 'phantom' location, and another bogus track to animate from lfFingerRing03 back to the
    //  known good location of 'xTipBone90'. We can patch this (more or less) by using keyframe data from
    //  the adjacent finger (lfFingerPinky).
    if (doFingerFix) {

        //  need to have all four finger bones to perform the fingerectomy
        doFingerFix = doFingerFix && (trkIndexRing02>=0) && (trkIndexRing03>=0) && (trkIndexPnky02>=0) && (trkIndexPnky03>=0);

        //  phantom Ring finger anim tracks usually have 100-200 keyframes, but I'm just checking for >2 to be conservative
        doFingerFix = doFingerFix &&
            (output[trkIndexRing02].keyframes.size() > 2) &&
            (output[trkIndexRing03].keyframes.size() > 2) &&
            (output[trkIndexPnky02].keyframes.size() > 0) &&
            (output[trkIndexPnky03].keyframes.size() > 0);

        // further restrict the scope by checking for suspicious offets within the animation track
        if (doFingerFix) {
            bool normal = true;
            const float transThreshold = 0.25f;
            for( size_t k=0; normal && k<output[trkIndexRing02].keyframes.size(); k++) {
                CalCoreKeyframe key = output[trkIndexRing02].keyframes[k];
                normal= (fabs(key.transform.translation.x)<transThreshold) &&
                        (fabs(key.transform.translation.y)<transThreshold) &&
                        (fabs(key.transform.translation.z)<transThreshold);
            }
            for( size_t k=0; normal && k<output[trkIndexRing03].keyframes.size(); k++) {
                CalCoreKeyframe key = output[trkIndexRing03].keyframes[k];
                normal= (fabs(key.transform.translation.x)<transThreshold) &&
                        (fabs(key.transform.translation.y)<transThreshold) &&
                        (fabs(key.transform.translation.z)<transThreshold);
            }

            //  ok - problem confirmed. apply fix.
            if (!normal) {
                CalQuaternion rot180;
                rot180.setAxisAngle(CalVector(1, 0, 0), 3.1415927410125732421875f);

                output[trkIndexRing02].keyframes.clear();
                for( size_t k=0; k<output[trkIndexPnky02].keyframes.size(); k++) {
                    CalCoreKeyframe key = output[trkIndexPnky02].keyframes[k];
                    //  ring finger should be just a tad bit longer than the pinkey
                    key.transform.translation *= 1.1f;
                    output[trkIndexRing02].keyframes.push_back(key);
                }

                output[trkIndexRing03].keyframes.clear();
                for( size_t k=0; k<output[trkIndexPnky03].keyframes.size(); k++) {
                    CalCoreKeyframe key = output[trkIndexPnky03].keyframes[k];
                    //  lfFingerPinky03 has an annoying 180 rotation that needs to be removed
                    key.transform.rotation = key.transform.rotation * rot180;
                    //  ring finger should be just a tad bit longer than the pinkey
                    key.transform.translation *= 1.1f;
                    output[trkIndexRing03].keyframes.push_back(key);
                }
            }
        }
    }

    swap(tracks, output);
}
