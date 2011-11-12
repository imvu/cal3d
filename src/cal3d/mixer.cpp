//****************************************************************************//
// mixer.cpp                                                                  //
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
#include "cal3d/mixer.h"
#include "cal3d/corebone.h"
#include "cal3d/coreanimation.h"
#include "cal3d/coretrack.h"
#include "cal3d/corekeyframe.h"
#include "cal3d/skeleton.h"
#include "cal3d/bone.h"
#include "cal3d/animation.h"

void CalMixer::addAnimation(const CalAnimationPtr& animation) {
    AnimationList::iterator i = activeAnimations.begin();
    while (i != activeAnimations.end() && animation->priority < (*i)->priority) {
        ++i;
    }
    activeAnimations.insert(i, animation);
}

void CalMixer::removeAnimation(const CalAnimationPtr& animation) {
    activeAnimations.remove(animation);
}

void CalMixer::updateSkeleton(
    CalSkeleton* skeleton,
    const std::vector<BoneTransformAdjustment>& boneTransformAdjustments,
    const std::vector<BoneScaleAdjustment>& boneScaleAdjustments,
    RootTransformFlag includeRoot
) {
    skeleton->resetPose();

    CalSkeleton::BoneArray& bones = skeleton->bones;

    // The bone adjustments are "replace" so they have to go first, giving them
    // highest priority and full influence.  Subsequent animations affecting the same bones,
    // including subsequent replace animations, will have their incluence attenuated appropriately.
    applyBoneAdjustments(skeleton, boneTransformAdjustments, boneScaleAdjustments);

    // loop through all animation actions
    for (
        std::list<CalAnimationPtr>::const_iterator itaa = activeAnimations.begin();
        itaa != activeAnimations.end();
        ++itaa
    ) {
        const CalAnimation* animation = itaa->get();

        const CalCoreAnimation::TrackList& tracks = animation->coreAnimation->tracks;

        for (
            CalCoreAnimation::TrackList::const_iterator track = tracks.begin();
            track != tracks.end();
            ++track
        ) {
            if (track->coreBoneId >= bones.size()) {
                continue;
            }

            bones[track->coreBoneId].blendPose(
                animation->weight,
                track->getState(animation->time),
                // higher priority animations replace 0-priority animations
                animation->priority != 0 ? animation->rampValue : 0.0f,
                animation->rampValue);
        }
    }

    skeleton->calculateAbsolutePose(includeRoot == IncludeRootTransform);
}

void CalMixer::applyBoneAdjustments(
    CalSkeleton* skeleton,
    const std::vector<BoneTransformAdjustment>& boneTransformAdjustments,
    const std::vector<BoneScaleAdjustment>& boneScaleAdjustments
) {
    CalSkeleton::BoneArray& bones = skeleton->bones;

    for (size_t i = 0; i < boneTransformAdjustments.size(); ++i) {
        const BoneTransformAdjustment& ba = boneTransformAdjustments[i];
        CalBone& bo = bones[ba.boneId];
        bo.blendPose(
            1.0f, /* unrampedWeight */
            cal3d::RotateTranslate(
                ba.localOri,
                bo.getOriginalTranslation() /* adjustedLocalPos */),
            ba.rampValue, /* subsequentAttenuation */
            ba.rampValue /* rampValue */);
    }

    for (size_t i = 0; i < boneScaleAdjustments.size(); i++) {
        const BoneScaleAdjustment& ba = boneScaleAdjustments[i];
        bones[ba.boneId].setMeshScaleAbsolute(ba.meshScaleAbsolute);
    }
}
