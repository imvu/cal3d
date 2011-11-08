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

cal3d::TransformAccumulator::TransformAccumulator() {
    totalWeight = 0;
}

void cal3d::TransformAccumulator::reset(const RotateTranslate& defaultPose) {
    totalWeight = 0;
}

void cal3d::TransformAccumulator::addTransform(float weight, const RotateTranslate& transform) {
    if (weight) {
        totalWeight += weight;
        float factor = weight / totalWeight;
        // I'm not comfortable asserting these, given floating point math.
        //cal3d::verify(factor > 0.0f, "factor must be positive");
        //cal3d::verify(factor <= 1.0f, "factor cannot exceed 1.0f");
        currentTransform = blend(factor, transform, currentTransform);
    }
}

CalBone::CalBone(const CalCoreBone& coreBone)
    : parentId(coreBone.parentId)
    , coreRelativeTransform(coreBone.relativeTransform)
    , coreInverseBindPoseTransform(coreBone.inverseBindPoseTransform)
{
    resetPose();
}

void CalBone::resetPose() {
    relativeTransform = coreRelativeTransform; // if no animations are applied, use this
    m_accumulatedWeightAbsolute = 0.0f;
    m_accumulatedReplacementAttenuation = 1.0f;
    m_meshScaleAbsolute.set(1, 1, 1);
}

/*****************************************************************************/
/** Interpolates the current state to another state.
  *
  * This function interpolates the current state (relative translation and
  * rotation) of the bone instance to another state of a given weight.
  *
  * @param unrampedWeight The blending weight, not incorporating ramp value
  * @param translation The relative translation to be interpolated to.
  * @param rotation The relative rotation to be interpolated to.
  * @param scale Optional scale from 0-1 applies to transformation directly without affecting weights.
  * @param replace If true, subsequent animations will have their weight attenuated by 1 - rampValue.
  * @param rampValue Amount to attenuate weight when ramping in/out the animation.
  *****************************************************************************/

void CalBone::blendPose(
    float unrampedWeight,
    const cal3d::RotateTranslate& transform,
    bool replace,
    float rampValue
) {
    const float attenuatedWeight = unrampedWeight * rampValue * m_accumulatedReplacementAttenuation;
    if (!attenuatedWeight) {
        return;
    }

    if (replace) {
        m_accumulatedReplacementAttenuation *= (1.0f - rampValue);
    }

    float factor = attenuatedWeight / (m_accumulatedWeightAbsolute + attenuatedWeight);
    assert(factor <= 1.0f);
    relativeTransform = blend(factor, relativeTransform, transform);
    m_accumulatedWeightAbsolute += attenuatedWeight;
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

    return absoluteTransform * coreInverseBindPoseTransform;
}
