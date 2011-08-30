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

#include <queue>
#include "cal3d/error.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h"

std::vector<CalCoreBonePtr> findInvalidParents(
    const std::vector<CalCoreBonePtr>& bones,
    const std::map<unsigned, CalCoreBonePtr>& originalIndices
) {
    std::set<CalCoreBonePtr> unknown(bones.begin(), bones.end());

    std::vector<CalCoreBonePtr> knownInvalid;
    while (!unknown.empty()) {
        CalCoreBonePtr p = *unknown.begin();

        std::set<CalCoreBonePtr> path;
        
        path.insert(p);
        while (p->parentId != -1) {
            if (p->parentId >= bones.size()) {
                knownInvalid.insert(knownInvalid.end(), path.begin(), path.end());
                break;
            }

            CalCoreBonePtr next = originalIndices.at(p->parentId);
            if (path.count(next)) {
                knownInvalid.insert(knownInvalid.end(), path.begin(), path.end());
                break;
            }
            p = next;
            path.insert(p);
        }

        // clear path
        for (std::set<CalCoreBonePtr>::iterator i = path.begin(); i != path.end(); ++i) {
            unknown.erase(*i);
        }
    }
    return knownInvalid;
}

CalCoreSkeleton::CalCoreSkeleton(const std::vector<CalCoreBonePtr>& bones)
    : coreBones(m_coreBones)
{
    m_coreBones.reserve(bones.size());

    std::map<unsigned, CalCoreBonePtr> originalIndices;
    for (size_t i = 0; i < bones.size(); ++i) {
        originalIndices[i] = bones[i];
    }

    std::vector<CalCoreBonePtr> invalidParents = findInvalidParents(bones, originalIndices);
    for (size_t i = 0; i < invalidParents.size(); ++i) {
        invalidParents[i]->parentId = -1;
    }

    std::queue<CalCoreBonePtr> q;
    for (size_t i = 0; i < bones.size(); ++i) {
        q.push(bones[i]);
    }

    std::set<CalCoreBonePtr> added;

    std::map<CalCoreBonePtr, unsigned> newIndices;

    while (!q.empty()) {
        CalCoreBonePtr p = q.front();
        q.pop();

        if (p->parentId == -1 || added.count(originalIndices[p->parentId])) {
            if (p->parentId != -1) {
                p->parentId = newIndices[originalIndices[p->parentId]];
            }
            newIndices[p] = addCoreBone(p);
            added.insert(p);
        } else {
            q.push(p);
        }
    }

    cal3d::verify(added.size() == bones.size(), "should have added all bones");
    cal3d::verify(newIndices.size() == originalIndices.size(), "new and original index lists should have identical lengths");
}

size_t CalCoreSkeleton::addCoreBone(const CalCoreBonePtr& coreBone) {
    cal3d::verify(coreBone->parentId == -1 || coreBone->parentId < coreBones.size(), "bones must be added in topological order");

    size_t newIndex = m_coreBones.size();
    m_coreBones.push_back(coreBone);
    return newIndex;
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
