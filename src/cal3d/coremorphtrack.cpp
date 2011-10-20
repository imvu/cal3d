//****************************************************************************//
// coremorphtrack.cpp                                                              //
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

#include <algorithm>
#include "cal3d/coremorphtrack.h"
#include "cal3d/error.h"
#include "cal3d/coremorphkeyframe.h"
#include "cal3d/memory.h"

size_t sizeInBytes(CalCoreMorphKeyframe const&) {
    return sizeof(CalCoreMorphKeyframe);
}

size_t CalCoreMorphTrack::size() const {
    return sizeof(*this) + morphName.capacity() + sizeInBytes(keyframes);
}

void CalCoreMorphTrack::addCoreMorphKeyframe(CalCoreMorphKeyframe keyframe) {
    keyframes.push_back(keyframe);
    std::sort(keyframes.begin(), keyframes.end());
}

float CalCoreMorphTrack::getState(float time) {
    std::vector<CalCoreMorphKeyframe>::iterator iteratorCoreMorphKeyframeBefore;
    std::vector<CalCoreMorphKeyframe>::iterator iteratorCoreMorphKeyframeAfter;

    // get the keyframe after the requested time
    iteratorCoreMorphKeyframeAfter = getUpperBound(time);

    // check if the time is after the last keyframe
    if (iteratorCoreMorphKeyframeAfter == keyframes.end()) {
        // return the last keyframe state
        --iteratorCoreMorphKeyframeAfter;
        return (*iteratorCoreMorphKeyframeAfter).weight;
    }

    // check if the time is before the first keyframe
    if (iteratorCoreMorphKeyframeAfter == keyframes.begin()) {
        // return the first keyframe state
        return iteratorCoreMorphKeyframeAfter->weight;
    }

    // get the keyframe before the requested one
    iteratorCoreMorphKeyframeBefore = iteratorCoreMorphKeyframeAfter;
    --iteratorCoreMorphKeyframeBefore;

    // get the two keyframe pointers
    CalCoreMorphKeyframe* pCoreMorphKeyframeBefore = &(*iteratorCoreMorphKeyframeBefore);
    CalCoreMorphKeyframe* pCoreMorphKeyframeAfter = &(*iteratorCoreMorphKeyframeAfter);

    // calculate the blending factor between the two keyframe states
    float blendFactor = (time - pCoreMorphKeyframeBefore->time) / (pCoreMorphKeyframeAfter->time - pCoreMorphKeyframeBefore->time);

    // blend between the two keyframes
    float weight = pCoreMorphKeyframeBefore->weight;
    float otherWeight = pCoreMorphKeyframeAfter->weight;
    return weight + blendFactor * (otherWeight - weight);
}

void CalCoreMorphTrack::scale(float factor) {
    std::for_each(keyframes.begin(), keyframes.end(), std::bind2nd(std::mem_fun_ref(&CalCoreMorphKeyframe::scale), factor));
}

std::vector<CalCoreMorphKeyframe>::iterator CalCoreMorphTrack::getUpperBound(float time) {
    return std::upper_bound(
        keyframes.begin(),
        keyframes.end(),
        CalCoreMorphKeyframe(time, 0.0f));
}
