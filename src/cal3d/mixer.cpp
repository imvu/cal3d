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


boost::shared_ptr<CalAnimation> CalMixer::addManualAnimation(const boost::shared_ptr<CalCoreAnimation>& coreAnimation) {
    boost::shared_ptr<CalAnimation> pAnimationAction(new CalAnimation(coreAnimation));
    m_listAnimationAction.push_front(pAnimationAction);
    return pAnimationAction;
}

void CalMixer::removeManualAnimation(const boost::shared_ptr<CalAnimation>& animation) {
    m_listAnimationAction.remove(animation);
}


void CalMixer::setManualAnimationAttributes(
    const boost::shared_ptr<CalAnimation>& aa,
    const CalMixerManualAnimationAttributes& p) {
    aa->time = p.time_;
    aa->weight = p.weight_;
    aa->scale = p.scale_;
    aa->rampValue = p.rampValue_;

    // now update composition function

    CalAnimation::CompositionFunction oldValue = aa->compositionFunction;

    // If the value isn't changing, then exit here.  Otherwise I would remove it and reinsert
    // it at the front, which wouldn't preserve the property that the most recently inserted
    // animation is highest priority.
    if (oldValue == p.compositionFunction_) {
        return;
    }
    aa->compositionFunction = p.compositionFunction_;

    // Iterate through the list and remove this element.
    m_listAnimationAction.remove(aa);

    // Now insert it back in in the appropriate position.  Replace animations go in at the front.
    // Average animations go in after the replace animations.
    switch (p.compositionFunction_) {
        case CalAnimation::CompositionFunctionReplace: {
            // Replace animations go on the front of the list.
            m_listAnimationAction.push_front(aa);
            break;
        }
        case CalAnimation::CompositionFunctionCrossFade: {
            // Average animations go after replace, but before Average.
            std::list<boost::shared_ptr<CalAnimation> >::iterator aait2;
            for (aait2 = m_listAnimationAction.begin(); aait2 != m_listAnimationAction.end(); aait2++) {
                CalAnimation::CompositionFunction cf = (*aait2)->compositionFunction;
                if (cf != CalAnimation::CompositionFunctionReplace) {
                    break;
                }
            }
            m_listAnimationAction.insert(aait2, aa);
            break;
        }
        case CalAnimation::CompositionFunctionAverage: {
            // Average animations go before the first Average animation.
            std::list<boost::shared_ptr<CalAnimation> >::iterator aait2;
            for (aait2 = m_listAnimationAction.begin(); aait2 != m_listAnimationAction.end(); aait2++) {
                CalAnimation::CompositionFunction cf = (*aait2)->compositionFunction;
                if (cf == CalAnimation::CompositionFunctionAverage) {  // Skip over replace and crossFade animations
                    break;
                }
            }
            m_listAnimationAction.insert(aait2, aa);
            break;
        }
        default: {
            assert(!"Unexpected");
            break;
        }
    }
}


void CalMixer::applyBoneAdjustments(CalSkeleton* pSkeleton) {
    std::vector<CalBone>& vectorBone = pSkeleton->bones;

    for (size_t i = 0; i < m_boneAdjustmentAndBoneIdArray.size(); i++) {
        CalMixerBoneAdjustmentAndBoneId& ba = m_boneAdjustmentAndBoneIdArray[ i ];
        CalBone* bo = &vectorBone[ ba.boneId_ ];
        const CalCoreBone& cbo = bo->getCoreBone();
        if (ba.boneAdjustment_.flags_ & CalMixerBoneAdjustmentFlagMeshScale) {
            bo->setMeshScaleAbsolute(ba.boneAdjustment_.meshScaleAbsolute_);
        }
        if (ba.boneAdjustment_.flags_ & CalMixerBoneAdjustmentFlagPosRot) {
            const CalVector& localPos = cbo.relativeTransform.translation;
            CalVector adjustedLocalPos = localPos;
            CalQuaternion adjustedLocalOri = ba.boneAdjustment_.localOri_;
            float const scale = 1.0f;
            float rampValue = ba.boneAdjustment_.rampValue_;
            bool const replace = true;
            float const unrampedWeight = 1.0f;
            bo->blendState(
                unrampedWeight,
                adjustedLocalPos,
                adjustedLocalOri,
                scale,
                replace,
                rampValue);
        }
    }
}

void CalMixer::addBoneAdjustment(int boneId, CalMixerBoneAdjustment const& ba) {
    CalMixerBoneAdjustmentAndBoneId baid;
    baid.boneAdjustment_ = ba;
    baid.boneId_ = boneId;
    m_boneAdjustmentAndBoneIdArray.push_back(baid);
}

void CalMixer::removeAllBoneAdjustments() {
    m_boneAdjustmentAndBoneIdArray.clear();
}


void CalMixer::updateSkeleton(CalSkeleton* pSkeleton) {
    pSkeleton->clearState();

    std::vector<CalBone>& vectorBone = pSkeleton->bones;

    // The bone adjustments are "replace" so they have to go first, giving them
    // highest priority and full influence.  Subsequent animations affecting the same bones,
    // including subsequent replace animations, will have their incluence attenuated appropriately.
    applyBoneAdjustments(pSkeleton);

    // loop through all animation actions
    std::list<boost::shared_ptr<CalAnimation> >::iterator itaa;
    for (itaa = m_listAnimationAction.begin(); itaa != m_listAnimationAction.end(); itaa++) {
        CalAnimation* aa = itaa->get();

        const boost::shared_ptr<CalCoreAnimation>& pCoreAnimation = aa->getCoreAnimation();
        CalCoreAnimation::TrackList& listCoreTrack = pCoreAnimation->tracks;

        CalCoreAnimation::TrackList::iterator itct;
        for (itct = listCoreTrack.begin(); itct != listCoreTrack.end(); itct++) {
            if (itct->coreBoneId >= int(vectorBone.size())) {
                continue;
            }
            CalBone* pBone = &vectorBone[itct->coreBoneId];

            // get the current translation and rotation
            CalVector translation;
            CalQuaternion rotation;
            itct->getState(aa->time, translation, rotation);

            // Replace and CrossFade both blend with the replace function.
            bool replace = aa->compositionFunction != CalAnimation::CompositionFunctionAverage;
            pBone->blendState(aa->weight, translation, rotation, aa->scale, replace, aa->rampValue);
        }
    }

    // === What does lockState() mean?  Why do we need it at all?  It seems only to allow us
    // to blend all the animation actions together into a temporary sum, and then
    // blend all the animation cycles together into a different sum, and then blend
    // the two sums together according to their relative weight sums.  I believe this is mathematically
    // equivalent of blending all the animation actions and cycles together into a single sum,
    // according to their relative weights.
    pSkeleton->lockState();

    // let the skeleton calculate its final state
    pSkeleton->calculateState();
}
