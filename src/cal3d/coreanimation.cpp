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

size_t sizeInBytes(const CalCoreTrack& t) {
    return t.sizeInBytes();
}

size_t CalCoreAnimation::sizeInBytes() const {
    return sizeof(*this) + ::sizeInBytes(tracks);
}

const CalCoreTrack* CalCoreAnimation::getCoreTrack(int coreBoneId) const {
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
    TrackList output;

    for (
        TrackList::iterator i = tracks.begin();
        i != tracks.end();
        ++i
    ) {
        if (i->coreBoneId >= skeleton->coreBones.size()) {
            continue;
        }

        i->coreBoneId = skeleton->boneIdTranslation[i->coreBoneId];

        i->fixup(skeleton->coreBones[i->coreBoneId]);
        output.push_back(*i);
    }

    swap(tracks, output);
}
