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

void CalCoreAnimation::fixup(const CalCoreSkeletonPtr& skeleton) {
    const auto& coreBones = skeleton->coreBones;

    TrackList output;
    output.reserve(coreBones.size());

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
        } else {
            i->fixup(
                coreBone,
                skeleton->getAdjustedRootTransform(i->coreBoneId));
        }
        output.push_back(*i);
    }

    swap(tracks, output);
}

void CalCoreAnimation::applyZupToYup() {
    for (auto i = tracks.begin(); i != tracks.end(); ++i) {
        i->applyZupToYup();
    }
}

void CalCoreAnimation::applyCoordinateTransform(CalQuaternion& xfm) {
    for (auto i = tracks.begin(); i != tracks.end(); ++i) {
        i->applyCoordinateTransform(xfm);
    }
}
