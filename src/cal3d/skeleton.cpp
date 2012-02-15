//****************************************************************************//
// calskeleton.cpp                                                            //
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
#include <utility>
#include "cal3d/skeleton.h"
#include "cal3d/error.h"
#include "cal3d/bone.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h" // DEBUG

CalSkeleton::CalSkeleton(const CalCoreSkeletonPtr& pCoreSkeleton) {
    assert(pCoreSkeleton);

    // clone the skeleton structure of the core skeleton
    const std::vector<CalCoreBonePtr>& coreBones = pCoreSkeleton->coreBones;

    size_t boneCount = coreBones.size();
    bones.destructive_resize(boneCount);

    for (size_t boneId = 0; boneId < boneCount; ++boneId) {
        new(&bones[boneId]) CalBone(*coreBones[boneId]);
    }

    boneTransforms.destructive_resize(boneCount);
}

void CalSkeleton::resetPose() {
    std::for_each(bones.begin(), bones.end(), std::mem_fun_ref(&CalBone::resetPose));
}

void CalSkeleton::calculateAbsolutePose() {
    CalBone* bones_ptr = cal3d::pointerFromVector(bones);
    for (unsigned i = 0; i < bones.size(); ++i) {
        boneTransforms[i] = bones_ptr[i].calculateAbsolutePose(bones_ptr);
    }
}
