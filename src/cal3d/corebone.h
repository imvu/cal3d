//****************************************************************************//
// corebone.h                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <list>
#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"
#include "cal3d/vector.h"
#include "cal3d/quaternion.h"

class CalCoreSkeleton;

enum CalLightType {
  LIGHT_TYPE_NONE,
  LIGHT_TYPE_OMNI,
  LIGHT_TYPE_DIRECTIONAL,
  LIGHT_TYPE_TARGET,
  LIGHT_TYPE_AMBIENT
};


class CAL3D_API CalCoreBone {
public:
  CalCoreBone(const std::string& name, int parentId = -1);

  bool addChildId(int childId);
  void calculateState(CalCoreSkeleton* skeleton);
  const std::vector<int>& getListChildId() const { return m_listChildId; }
  const CalQuaternion& getRotation() const { return m_rotation; }
  const CalQuaternion& getRotationAbsolute() const { return m_rotationAbsolute; }
  const CalQuaternion& getRotationBoneSpace() const { return m_rotationBoneSpace; }
  const CalVector& getTranslation() const { return m_translation; }
  const CalVector& getTranslationAbsolute() const { return m_translationAbsolute; }
  const CalVector& getTranslationBoneSpace() const { return m_translationBoneSpace; }
  void setRotation(const CalQuaternion& rotation);
  void setRotationBoneSpace(const CalQuaternion& rotation);
  void setTranslation(const CalVector& translation);
  void setTranslationBoneSpace(const CalVector& translation);

  void scale(float factor, CalCoreSkeleton* skeleton);

  bool hasLightingData();

  const int parentId;
  const std::string name;

  CalVector lightColor;
  CalLightType lightType;

private:
  std::vector<int> m_listChildId;
  CalVector m_translation;
  CalQuaternion m_rotation;
  CalVector m_translationAbsolute;
  CalQuaternion m_rotationAbsolute;
  CalVector m_translationBoneSpace;
  CalQuaternion m_rotationBoneSpace;
};
