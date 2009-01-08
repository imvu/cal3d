//****************************************************************************//
// bone.h                                                                     //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"
#include "cal3d/vector.h"
#include "cal3d/quaternion.h"
#include "cal3d/corebone.h"

class CalSkeleton;
class CalModel;
class CalCoreModel;

class CAL3D_API CalBone
{
public:
  CalBone(CalCoreBone* pCoreBone);

  void blendState(
    float unrampedWeight, const CalVector& translation, 
    const CalQuaternion & rotation, float scale = 1.0f,
    bool replace = false, float rampValue = 1.0f );
  void calculateState(CalSkeleton* skeleton, unsigned myIndex);
  void clearState();
  const CalCoreBone& getCoreBone() {
    return m_coreBone;
  }
  void setRotation(const CalQuaternion& rotation);
  const CalQuaternion& getRotation();
  const CalQuaternion& getRotationAbsolute();
  const CalQuaternion& getRotationBoneSpace();
  void setTranslation(const CalVector& translation);
  const CalVector& getTranslation();
  const CalVector& getTranslationAbsolute();
  inline void setMeshScaleAbsolute( CalVector const & sv ) {
    m_meshScaleAbsolute = sv; }
  void lockState();

private:
  // Store a copy of the CoreBone locally for better cache locality.
  const CalCoreBone m_coreBone;

  float m_accumulatedWeight;
  float m_accumulatedWeightAbsolute;
  float m_accumulatedReplacementAttenuation;
  float m_firstBlendScale;
  CalVector m_meshScaleAbsolute; // w.r.t. absolute coord system in 3dsMax (Z up), not local coord of bone.

  CalVector m_translation;
  CalQuaternion m_rotation;

  CalVector m_translationAbsolute;
  CalQuaternion m_rotationAbsolute;

  //CalVector m_translationBoneSpace;
  CalQuaternion m_rotationBoneSpace;
};
