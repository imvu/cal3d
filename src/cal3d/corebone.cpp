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
        m_translationAbsolute = m_translation;
        m_rotationAbsolute = m_rotation;
    } else {
        // get the parent bone
        CalCoreBone* pParent = skeleton->coreBones[parentId].get();

        // transform relative state with the absolute state of the parent
        m_translationAbsolute = m_translation;
        m_translationAbsolute *= pParent->getRotationAbsolute();
        m_translationAbsolute += pParent->getTranslationAbsolute();

        m_rotationAbsolute = m_rotation;
        m_rotationAbsolute *= pParent->getRotationAbsolute();
    }

    // calculate all child bones
    std::vector<int>::iterator iteratorChildId;
    for (iteratorChildId = childIds.begin(); iteratorChildId != childIds.end(); ++iteratorChildId) {
        skeleton->coreBones[*iteratorChildId]->calculateState(skeleton);
    }
}

void CalCoreBone::setRotation(const CalQuaternion& rotation) {
    m_rotation = rotation;
}

void CalCoreBone::setRotationBoneSpace(const CalQuaternion& rotation) {
    m_rotationBoneSpace = rotation;
}

void CalCoreBone::setTranslation(const CalVector& translation) {
    m_translation = translation;
}

void CalCoreBone::setTranslationBoneSpace(const CalVector& translation) {
    m_translationBoneSpace = translation;
}

void CalCoreBone::scale(float factor, CalCoreSkeleton* skeleton) {
    m_translation *= factor;
    m_translationAbsolute *= factor;
    m_translationBoneSpace *= factor;

    // calculate all child bones
    std::vector<int>::const_iterator iteratorChildId;
    for (iteratorChildId = childIds.begin(); iteratorChildId != childIds.end(); ++iteratorChildId) {
        skeleton->coreBones[*iteratorChildId]->scale(factor, skeleton);
    }
}


bool CalCoreBone::hasLightingData() {
    return lightType != LIGHT_TYPE_NONE;
}
