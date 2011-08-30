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

void CalCoreSkeleton::addCoreBone(const CalCoreBonePtr& coreBone) {
    cal3d::verify(coreBone->parentId == -1 || coreBone->parentId < coreBones.size(), "bones must be added in topological order");

    m_coreBones.push_back(coreBone);
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

std::vector<int> CalCoreSkeleton::getChildIds(const CalCoreBone* coreBone) const {
    int index = -1;
    if (coreBone) {
        for (size_t i = 0; i < coreBones.size(); ++i) {
            if (coreBones[i].get() == coreBone) {
                index = i;
                break;
            }
        }
        cal3d::verify(index != -1, "Cannot calculate children of bone not owned by this skeleton");
    } else {
        // find roots
    }

    std::vector<int> rv;
    for (size_t i = 0; i < coreBones.size(); ++i) {
        if (coreBones[i]->parentId == index) {
            rv.push_back(i);
        }
    }
    return rv;
}
