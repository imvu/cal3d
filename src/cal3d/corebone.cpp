//****************************************************************************//
// corebone.cpp                                                               //
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
#include "cal3d/corebone.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/coremesh.h"
#include "cal3d/coresubmesh.h"


CalCoreBone::CalCoreBone(const std::string& n, int p)
    : parentId(p)
    , name(n) {
}


/*****************************************************************************/
/** Calculates the current state.
  *
  * This function calculates the current state (absolute translation and
  * rotation) of the core bone instance and all its children.
  *****************************************************************************/

void CalCoreBone::calculateState(CalCoreSkeleton* skeleton) {
    if (parentId == -1) {
        // no parent, this means absolute state == relative state
        absoluteTransform = relativeTransform;
    } else {
        CalCoreBone* pParent = skeleton->coreBones[parentId].get();

        // transform relative state with the absolute state of the parent
        absoluteTransform.translation = relativeTransform.translation;
        absoluteTransform.translation *= pParent->absoluteTransform.rotation;
        absoluteTransform.translation += pParent->absoluteTransform.translation;

        absoluteTransform.rotation = relativeTransform.rotation;
        absoluteTransform.rotation *= pParent->absoluteTransform.rotation;
    }

    // calculate all child bones
    std::vector<int>::iterator iteratorChildId;
    for (iteratorChildId = childIds.begin(); iteratorChildId != childIds.end(); ++iteratorChildId) {
        skeleton->coreBones[*iteratorChildId]->calculateState(skeleton);
    }
}

void CalCoreBone::scale(float factor, CalCoreSkeleton* skeleton) {
    relativeTransform.translation *= factor;
    absoluteTransform.translation *= factor;
    boneSpaceTransform.translation *= factor;

    // calculate all child bones
    std::vector<int>::const_iterator iteratorChildId;
    for (iteratorChildId = childIds.begin(); iteratorChildId != childIds.end(); ++iteratorChildId) {
        skeleton->coreBones[*iteratorChildId]->scale(factor, skeleton);
    }
}


