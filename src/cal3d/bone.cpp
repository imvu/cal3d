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
    , coreBoneSpaceTransform(coreBone.boneSpaceTransform)
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
    const cal3d::Transform& transform,
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
            absoluteTransform = cal3d::Transform();
        }
    } else {
        absoluteTransform = bones[parentId].absoluteTransform * relativeTransform;
    }

    CalVector boneSpaceTranslation(coreBoneSpaceTransform.translation);

    bool meshScalingOn = m_meshScaleAbsolute.x != 1 || m_meshScaleAbsolute.y != 1 || m_meshScaleAbsolute.z != 1;
    if (meshScalingOn) {
        // The mesh transformation is intended to apply to the vector from the
        // bone node to the vert, relative to the model's global coordinate system.
        // For example, even though the head node's X axis aims up, the model's
        // global coordinate system has X to stage right, Z up, and Y stage back.
        //
        // The standard vert transformation is:
        // v = (vmesh - boneAbsPosInJpose) * boneAbsRotInAnimPose + boneAbsPosInAnimPose
        //
        // Cal3d does the calculation by:
        // u = (umesh * transformMatrix) + translationBoneSpace
        //
        // where translationBoneSpace = "coreBoneTranslationBoneSpace" * boneAbsRotInAnimPose + boneAbsPosInAnimPose
        //   and transformMatrix = "coreBoneRotBoneSpace" * boneAbsRotInAnimPose
        //
        // I don't know what "coreBoneRotBoneSpace" and "coreBoneTranslationBoneSpace" actually are,
        // but to add scale to the scandard vert transformation, I simply do:
        //
        // v' = vmesh            * scalevec * boneAbsRotInAnimPose
        //   - boneAbsPosInJpose * scalevec * boneAbsRotInAnimPose
        //   + boneAbsPosInAnimPose
        //
        // Essentially, the boneAbsPosInJpose is just an extra vector added to
        // each vertex that we want to subtract out.  We must transform the extra
        // vector in exactly the same way we transform the vmesh.  Therefore if we scale the mesh, we
        // must also scale the boneAbsPosInJpose.
        //
        // Expanding out the u2 equation, we have:
        //
        // u = umesh * "coreBoneRotBoneSpace" * boneAbsRotInAnimPose
        //   + "coreBoneTranslationBoneSpace" * boneAbsRotInAnimPose
        //   + boneAbsPosInAnimPose
        //
        // We assume that "coreBoneTranslationBoneSpace" = vectorThatMustBeSubtractedFromUmesh * "coreBoneRotBoneSpace":
        //
        // u = umesh * "coreBoneRotBoneSpace"                               * boneAbsRotInAnimPose
        //   + vectorThatMustBeSubtractedFromUmesh * "coreBoneRotBoneSpace" * boneAbsRotInAnimPose
        //   + boneAbsPosInAnimPose
        //
        // We assume that scale should be applied to umesh, not umesh * "coreBoneRotBoneSpace":
        //
        // u = umesh * scaleVec * "coreBoneRotBoneSpace" * boneAbsRotInAnimPose
        //   + "coreBoneTranslationBoneSpace" * "coreBoneRotBoneSpaceInverse" * scaleVec * "coreBoneRotBoneSpace" * boneAbsRotInAnimPose
        //   + boneAbsPosInAnimPose
        //
        // which yields,
        //
        // transformMatrix' =  scaleVec * "coreBoneRotBoneSpace" * boneAbsRotInAnimPose
        //
        // and,
        //
        // translationBoneSpace' =
        //   coreBoneTranslationBoneSpace * "coreBoneRotBoneSpaceInverse" * scaleVec * "coreBoneRotBoneSpace"
        //   * boneAbsRotInAnimPose
        //   + boneAbsPosInAnimPose

        //boneSpaceTranslation *= m_meshScaleAbsolute;
        boneSpaceTranslation = coreBoneSpaceTransform.rotation * ((-coreBoneSpaceTransform.rotation * boneSpaceTranslation) * m_meshScaleAbsolute);
    }

    CalMatrix boneSpaceRotationAndScale(coreBoneSpaceTransform.rotation);
    if (meshScalingOn) {

        // By applying each scale component to the row, instead of the column, we
        // are effectively making the scale apply prior to the rotationBoneSpace.
        boneSpaceRotationAndScale.dxdx *= m_meshScaleAbsolute.x;
        boneSpaceRotationAndScale.dydx *= m_meshScaleAbsolute.x;
        boneSpaceRotationAndScale.dzdx *= m_meshScaleAbsolute.x;

        boneSpaceRotationAndScale.dxdy *= m_meshScaleAbsolute.y;
        boneSpaceRotationAndScale.dydy *= m_meshScaleAbsolute.y;
        boneSpaceRotationAndScale.dzdy *= m_meshScaleAbsolute.y;

        boneSpaceRotationAndScale.dxdz *= m_meshScaleAbsolute.z;
        boneSpaceRotationAndScale.dydz *= m_meshScaleAbsolute.z;
        boneSpaceRotationAndScale.dzdz *= m_meshScaleAbsolute.z;
    }

    return BoneTransform(
        CalMatrix(absoluteTransform.rotation) * boneSpaceRotationAndScale,
        absoluteTransform * boneSpaceTranslation);
}
