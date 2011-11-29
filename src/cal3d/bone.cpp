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
    currentTransform = defaultPose;
}

void cal3d::TransformAccumulator::addTransform(float weight, const RotateTranslate& transform) {
    if (weight) {
        totalWeight += weight;
        float factor = weight / totalWeight;
        // I'm not comfortable asserting these, given floating point
        // math.  For example, imagine that totalWeight is HUGE and
        // weight is small.  Would dividing produce a zero factor?
        // cal3d::verify(factor > 0.0f, "factor must be positive");
        // cal3d::verify(factor <= 1.0f, "factor cannot exceed 1.0f");
        currentTransform = blend(factor, currentTransform, transform);
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
    transformAccumulator.reset(coreRelativeTransform); // if no animations are applied, use this
    currentAttenuation = 1.0f;
    scale.setIdentity();
    absoluteScale.setIdentity();
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
    float weight,
    const cal3d::RotateTranslate& transform,
    float subsequentAttenuation
) {
    transformAccumulator.addTransform(weight * currentAttenuation, transform);
    currentAttenuation *= (1.0f - subsequentAttenuation);
}

static void removeScale(CalMatrix& m) {
    m.cx.normalize();
    m.cy.normalize();
    m.cz.normalize();
}

static cal3d::Transform removeScale(cal3d::Transform t) {
    removeScale(t.basis);
    return t;
}

BoneTransform CalBone::calculateAbsolutePose(const CalBone* bones, bool includeRootTransform) {
    const auto& parentBone = (parentId == -1) ? nullptr : &bones[parentId];

    const auto& parentScale = parentBone
        ? parentBone->absoluteScale
        : cal3d::Scale();
    absoluteScale = parentScale * scale;

    const auto& parentTransform = parentBone
        ? parentBone->absoluteTransform
        : cal3d::Transform();
    const auto& myTransform = (parentBone || includeRootTransform)
        ? getRelativeTransform()
        : cal3d::RotateTranslate();

    absoluteTransform = removeScale(parentTransform * myTransform) * absoluteScale;
    return absoluteTransform * coreInverseBindPoseTransform;
}
