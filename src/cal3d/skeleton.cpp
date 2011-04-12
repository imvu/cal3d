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

CalSkeleton::CalSkeleton(const boost::shared_ptr<CalCoreSkeleton>& pCoreSkeleton)
    : m_pCoreSkeleton(pCoreSkeleton) {
    assert(pCoreSkeleton);

    // clone the skeleton structure of the core skeleton
    std::vector<boost::shared_ptr<CalCoreBone> >& vectorCoreBone = pCoreSkeleton->coreBones;

    size_t boneCount = vectorCoreBone.size();
    bones.reserve(boneCount);

    for (size_t boneId = 0; boneId < boneCount; ++boneId) {
        bones.push_back(CalBone(vectorCoreBone[boneId].get()));
    }

    boneTransforms.resize(boneCount);
}

void CalSkeleton::calculateState() {
    // calculate all bone states of the skeleton
    const std::vector<int>& listRootCoreBoneId = m_pCoreSkeleton->rootBoneIds;

    std::vector<int>::const_iterator iteratorRootBoneId;
    for (iteratorRootBoneId = listRootCoreBoneId.begin(); iteratorRootBoneId != listRootCoreBoneId.end(); ++iteratorRootBoneId) {
        bones[*iteratorRootBoneId].calculateState(this, *iteratorRootBoneId);
    }
}

void CalSkeleton::clearState() {
    std::for_each(bones.begin(), bones.end(), std::mem_fun_ref(&CalBone::clearState));
}

void CalSkeleton::lockState() {
    std::for_each(bones.begin(), bones.end(), std::mem_fun_ref(&CalBone::lockState));
}

