//****************************************************************************//
// mixer.h                                                                    //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
// Copyright (C) 2004 Mekensleep <licensing@mekensleep.com>                   //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <list>
#include "cal3d/animation.h"
#include "cal3d/quaternion.h"

CAL3D_PTR(CalAnimation);
class CalSkeleton;

struct BoneTransformAdjustment {
    unsigned boneId;

    // Relative to the parent frame of reference.
    CalQuaternion localOri;

    // The adjustment is a highest priority "replace" animation for the bone.  Lower priority
    // animations for the bone, including other replace animations, will be attenuated by 1 - rampValue.
    float rampValue;
};

struct BoneScaleAdjustment {
    BoneScaleAdjustment(unsigned i, const CalVector& scale)
        : boneId(i)
        , scale(scale)
    {}

    unsigned boneId;
    CalVector scale;
};

class CAL3D_API CalMixer {
public:
    void addAnimation(const CalAnimationPtr& animation);
    void removeAnimation(const CalAnimationPtr& animation);

    void updateSkeleton(
        CalSkeleton* skeleton,
        const std::vector<BoneTransformAdjustment>& boneTransformAdjustments,
        const std::vector<BoneScaleAdjustment>& boneScaleAdjustments);

private:
    void applyBoneAdjustments(
        CalSkeleton* skeleton,
        const std::vector<BoneTransformAdjustment>& boneTransformAdjustments,
        const std::vector<BoneScaleAdjustment>& boneScaleAdjustments);

    typedef std::list<CalAnimationPtr> AnimationList;
    AnimationList activeAnimations;
};
