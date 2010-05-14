//****************************************************************************//
// corekeyframe.h                                                             //
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

class CAL3D_API CalCoreKeyframe : public Cal::Object
{
public:
  CalCoreKeyframe();

  size_t size() const { return sizeof(*this); }
  const CalQuaternion& getRotation() const;
  float getTime() const;
  const CalVector& getTranslation() const;
  void setRotation(const CalQuaternion& rotation);
  void setTime(float time);
  void setTranslation(const CalVector& translation);

private:
  float m_time;
  CalVector m_translation;
  CalQuaternion m_rotation;
};
