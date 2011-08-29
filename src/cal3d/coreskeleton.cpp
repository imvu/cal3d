//****************************************************************************//
// coreskeleton.cpp                                                           //
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

#include "cal3d/error.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h"

CalCoreSkeleton::CalCoreSkeleton()
    : coreBones(m_coreBones)
{}

size_t CalCoreSkeleton::addCoreBone(const CalCoreBonePtr& coreBone) {
    cal3d::verify(coreBone->parentId == -1 || coreBone->parentId < coreBones.size(), "bones must be added in topological order");

    size_t boneId = coreBones.size();
    m_coreBones.push_back(coreBone);

    if (coreBone->parentId == -1) {
        rootBoneIds.push_back(boneId);
    }

    return boneId;
}

void CalCoreSkeleton::scale(float factor) {
    for (
        std::vector<CalCoreBonePtr>::const_iterator i = coreBones.begin();
        i != coreBones.end();
        ++i
    ) {
        (*i)->scale(factor);
    }
}
