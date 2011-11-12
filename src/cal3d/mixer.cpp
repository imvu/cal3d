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


void CalMixer::addManualAnimation(const CalAnimationPtr& animation) {
    activeAnimations.push_front(animation);
}

void CalMixer::removeManualAnimation(const CalAnimationPtr& animation) {
    activeAnimations.remove(animation);
}

void CalMixer::setManualAnimationAttributes(
    const CalAnimationPtr& animation,
    const AnimationAttributes& p
) {
    animation->time = p.time;
    animation->weight = p.weight;
    animation->rampValue = p.rampValue;

    // now update composition function

    // If the value isn't changing, then exit here.  Otherwise I would remove it and reinsert
    // it at the front, which wouldn't preserve the property that the most recently inserted
    // animation is highest priority.
    if (animation->priority == p.priority) {
        return;
    }
    animation->priority = p.priority;

    activeAnimations.remove(animation);
    // Now insert it back in in the appropriate position.  Replace animations go in at the front.
    // Average animations go in after the replace animations.
    switch (p.priority) {
        case 2: {
            // Replace animations go on the front of the list.
            activeAnimations.push_front(animation);
            break;
        }
        case 1: {
            // Crossfade animations go after replace, but before Average.
            std::list<CalAnimationPtr>::iterator aait2 = activeAnimations.begin();
            for (; aait2 != activeAnimations.end(); aait2++) {
                if ((*aait2)->priority != 2) {
                    break;
                }
            }
            activeAnimations.insert(aait2, animation);
            break;
        }
        case 0: {
            // Average animations go before the first Average animation.
            std::list<CalAnimationPtr>::iterator aait2 = activeAnimations.begin();
            for (; aait2 != activeAnimations.end(); aait2++) {
                if ((*aait2)->priority == 0) {  // Skip over replace and crossFade animations
                    break;
                }
            }
            activeAnimations.insert(aait2, animation);
            break;
        }
        default: {
            assert(!"Unexpected");
            break;
        }
    }
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
            CalCoreAnimation::TrackList::const_iterator itct = tracks.begin();
            itct != tracks.end();
            ++itct
        ) {
            if (itct->coreBoneId >= bones.size()) {
                continue;
            }

            bones[itct->coreBoneId].blendPose(
                animation->weight,
                itct->getState(animation->time),
                animation->priority != 0, // higher priority animations replace 0-priority animations
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
            true, /* replace */
            ba.rampValue /* rampValue */);
    }

    for (size_t i = 0; i < boneScaleAdjustments.size(); i++) {
        const BoneScaleAdjustment& ba = boneScaleAdjustments[i];
        bones[ba.boneId].setMeshScaleAbsolute(ba.meshScaleAbsolute);
    }
}
