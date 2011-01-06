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

class CalCoreKeyframe {
public:
  CalCoreKeyframe() {
    time = 0.0f;
  }

  CalCoreKeyframe(float t, const CalVector& tr, const CalQuaternion& ro)
      : time(t)
      , translation(tr)
      , rotation(ro)
  {}

  float time;
  CalVector translation;
  CalQuaternion rotation;
};

inline bool operator==(const CalCoreKeyframe& lhs, const CalCoreKeyframe& rhs) {
  return close(lhs.time, rhs.time) &&
    lhs.translation == rhs.translation &&
    lhs.rotation == rhs.rotation;
}
