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
#include "cal3d/global.h"
#include "cal3d/animation.h"
#include "cal3d/quaternion.h"

//****************************************************************************//
// Forward declarations                                                       //
//****************************************************************************//

class CalAnimation;
class CalModel;
class CalSkeleton;

struct CalMixerManualAnimationAttributes {
  bool on_;
  float time_;
  float weight_;
  float scale_;
  float rampValue_;
  CalAnimation::CompositionFunction compositionFunction_;
};

enum CalMixerBoneAdjustmentFlag {
  CalMixerBoneAdjustmentFlagPosRot = 1,
  CalMixerBoneAdjustmentFlagMeshScale = 2
};


struct CalMixerBoneAdjustment {

  // What parts of the adjustment are to be applied?
  unsigned int flags_;

  // Relative to the parent frame of reference.
  CalVector localPos_;
  CalQuaternion localOri_;

  // Scales X, Y, and Z of mesh by these parameters.  The scale parameters are with
  // respect to the absolute coordinate space, e.g., Z is up in 3dMax, as opposed
  // to the local coordinate space of the bone.
  CalVector meshScaleAbsolute_;

  // The adjustment is a highest priority "replace" animation for the bone.  Lower priority
  // animations for the bone, including other replace animations, will be attenuated by 1 - rampValue.
  float rampValue_;
};


struct CalMixerBoneAdjustmentAndBoneId {
  CalMixerBoneAdjustment boneAdjustment_;
  int boneId_;
};

// Total number of bone adjustments per mixer.
#define CalMixerBoneAdjustmentsMax ( 20 ) // Arbitrary.


class CAL3D_API CalMixer
{
public:
  CalMixer();

  boost::shared_ptr<CalAnimation> addManualAnimation(const boost::shared_ptr<CalCoreAnimation>& coreAnimation);
  void removeManualAnimation(const boost::shared_ptr<CalAnimation>& coreAnimation);
  void setManualAnimationAttributes(const boost::shared_ptr<CalAnimation>& coreAnimation, CalMixerManualAnimationAttributes const& p);

  void updateSkeleton(CalSkeleton* skeleton);

  bool addBoneAdjustment( int boneId, CalMixerBoneAdjustment const & );
  bool removeBoneAdjustment( int boneId );
  void removeAllBoneAdjustments();

  void applyBoneAdjustments(CalSkeleton* skeleton);

private:
  std::list< boost::shared_ptr<CalAnimation> > m_listAnimationAction;
  unsigned int m_numBoneAdjustments;
  CalMixerBoneAdjustmentAndBoneId m_boneAdjustmentAndBoneIdArray[ CalMixerBoneAdjustmentsMax ];
};
