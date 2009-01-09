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
#include "cal3d/vector4.h"

class CalCoreSkeleton;
class CalCoreModel;
class CalBone;

class CAL3D_API CalSkeleton
{
public:
  CalSkeleton(CalCoreSkeleton* pCoreSkeleton);

  void calculateState();
  void clearState();
  CalBone* getBone(int boneId);
  CalCoreSkeleton* getCoreSkeleton();
  std::vector<CalBone>& getVectorBone();
  void lockState();

  SSEArray<BoneTransform> boneTransforms;

private:
  CalCoreSkeleton *m_pCoreSkeleton;
  std::vector<CalBone> m_vectorBone;
};
