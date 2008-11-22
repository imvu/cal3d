//****************************************************************************//
// model.h                                                                    //
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

class CalCoreModel;
class CalSkeleton;
class CalAbstractMixer;
class CalMixer;
class CalMorphTargetMixer;
class CalMesh;

class CAL3D_API CalModel
{
// member variables
protected:
  CalCoreModel *m_pCoreModel;
  CalSkeleton *m_pSkeleton;
  CalAbstractMixer *m_pMixer;
  CalMorphTargetMixer *m_pMorphTargetMixer;
  std::vector<CalMesh *> m_vectorMesh;

// constructors/destructor
public: 
  CalModel();
  ~CalModel();

// member functions
public:
  bool attachMesh(int coreMeshId);
  void create(CalCoreModel *pCoreModel);
  void destroy();
  bool detachMesh(int coreMeshId);
  CalCoreModel *getCoreModel();
  CalMesh *getMesh(int coreMeshId);
  CalMixer *getMixer();
  CalAbstractMixer *getAbstractMixer();
  void setAbstractMixer(CalAbstractMixer* pMixer);
  CalMorphTargetMixer *getMorphTargetMixer();
  CalSkeleton *getSkeleton();
  std::vector<CalMesh *>& getVectorMesh();
  void setLodLevel(float lodLevel);
  void setMaterialSet(int setId);
  //void update(float deltaTime);
};
