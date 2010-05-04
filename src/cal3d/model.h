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

class CalCoreMesh;
class CalCoreModel;
class CalSkeleton;
class CalAbstractMixer;
class CalMixer;
class CalMesh;

class CAL3D_API CalModel : public Cal::Object
{
public: 
  CalModel();
  ~CalModel();

  bool attachMesh(const boost::shared_ptr<CalCoreMesh>& pCoreMesh);
  void create(CalCoreModel *pCoreModel);
  void destroy();
  CalCoreModel *getCoreModel();
  CalMesh *getMesh(const boost::shared_ptr<CalCoreMesh>& pCoreMesh);
  CalMixer *getMixer();
  CalAbstractMixer *getAbstractMixer();
  void setAbstractMixer(CalAbstractMixer* pMixer);
  CalSkeleton *getSkeleton();
  std::vector<CalMesh *>& getVectorMesh();
  void setLodLevel(float lodLevel);
  void setMaterialSet(int setId);

private:
  CalCoreModel *m_pCoreModel;
  CalSkeleton *m_pSkeleton;
  CalAbstractMixer *m_pMixer;
  std::vector<CalMesh *> m_vectorMesh;
};
