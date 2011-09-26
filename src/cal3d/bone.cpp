//****************************************************************************//
// bone.cpp                                                                   //
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
#include "cal3d/bone.h"
#include "cal3d/coremesh.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/corebone.h"
#include "cal3d/matrix.h"
#include "cal3d/skeleton.h"
#include "cal3d/coreskeleton.h"


CalBone::CalBone(const CalCoreBone& coreBone)
    : parentId(coreBone.parentId)
    , coreRelativeTransform(coreBone.relativeTransform)
    , coreBoneSpaceTransform(coreBone.inverseBindPoseTransform)
{
    resetPose();
}

void CalBone::resetPose() {
    relativeTransform = coreRelativeTransform; // if no animations are applied, use this
    m_accumulatedWeight = 0.0f;
    m_accumulatedReplacementAttenuation = 1.0f;
    m_meshScaleAbsolute.set(1, 1, 1);
}

/*****************************************************************************/
/** Interpolates the current state to another state.
  *
  * This function interpolates the current state (relative translation and
  * rotation) of the bone instance to another state of a given weight.
  *
  * @param replace If true, subsequent animations will have their weight attenuated by 1 - rampValue.
  * @param rampValue Amount to attenuate weight when ramping in/out the animation.
  *****************************************************************************/

void CalBone::blendPose(
    const cal3d::RotateTranslate& transform,
    bool replace,
    const float rampValue
) {
    const float attenuatedWeight = rampValue * m_accumulatedReplacementAttenuation;
    if (replace) {
        m_accumulatedReplacementAttenuation *= (1.0f - rampValue);
    }

    m_accumulatedWeight += attenuatedWeight;

    float factor = m_accumulatedWeight
        ? attenuatedWeight / m_accumulatedWeight
        : 0.0f;

    assert(factor <= 1.0f);
    relativeTransform = blend(factor, relativeTransform, transform);
}

BoneTransform CalBone::calculateAbsolutePose(const CalBone* bones, bool includeRootTransform) {
    if (parentId == -1) {
        if (includeRootTransform) {
            // no parent, this means absolute state == relative state
            absoluteTransform = relativeTransform;
        } else {
            absoluteTransform = cal3d::RotateTranslate();
        }
    } else {
        absoluteTransform = bones[parentId].absoluteTransform * relativeTransform;
    }

    if (m_meshScaleAbsolute.x != 1 || m_meshScaleAbsolute.y != 1 || m_meshScaleAbsolute.z != 1) {
        absoluteTransform = absoluteTransform * cal3d::Scale(m_meshScaleAbsolute);
    }

    return absoluteTransform * coreBoneSpaceTransform;
}
