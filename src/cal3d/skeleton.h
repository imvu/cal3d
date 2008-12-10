//****************************************************************************//
// skeleton.h                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"
#include "cal3d/matrix.h"
#include "cal3d/vector.h"

class CalCoreSkeleton;
class CalCoreModel;
class CalBone;

class CAL3D_API CalSkeleton
{
public:
  // 3x3 transform matrix plus a translation 3-vector (stored in the w components
  // of the rows.  This struct needs to be 16-byte aligned for SSE.
  struct BoneTransform {
    CalVector4 rowx;
    CalVector4 rowy;
    CalVector4 rowz;
  };

  CalSkeleton(CalCoreSkeleton* pCoreSkeleton);
  ~CalSkeleton();

  void calculateState();
  void clearState();
  CalBone *getBone(int boneId);
  CalCoreSkeleton *getCoreSkeleton();
  std::vector<CalBone *>& getVectorBone();
  void lockState();

  SSEArray<BoneTransform> boneTransforms;

private:
  CalCoreSkeleton *m_pCoreSkeleton;
  std::vector<CalBone *> m_vectorBone;
};
