//****************************************************************************//
// coreMorphKeyframe.h                                                             //
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

class CAL3D_API CalCoreMorphKeyframe : public Cal::Object
{
protected:
  float m_time;
  float m_weight;

public:
  CalCoreMorphKeyframe();
  virtual ~CalCoreMorphKeyframe();

  bool create();
  void destroy();
  float getTime() const;
  void setTime(float time);

  float getWeight() const;
  void setWeight(float);
};
