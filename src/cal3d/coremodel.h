//****************************************************************************//
// coremodel.h                                                                //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <map>
#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"

class CalCoreSkeleton;
class CalCoreMaterial;

class CAL3D_API CalCoreModel : public Cal::Object
{
public: // TODO: make private
  CalCoreSkeleton *m_pCoreSkeleton;
  std::vector< boost::shared_ptr<CalCoreMaterial> > m_vectorCoreMaterial;
  std::map<std::pair<int, int>, int> m_mapCoreMaterialThread;
  unsigned int m_magic;

public:
  CalCoreModel();
  ~CalCoreModel();

  int addCoreMaterial(boost::shared_ptr<CalCoreMaterial> pCoreMaterial);
  boost::shared_ptr<CalCoreMaterial> getCoreMaterial(int coreMaterialId);
  int getCoreMaterialCount();
  int getCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId);
  CalCoreSkeleton *getCoreSkeleton();
  bool setCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId, int coreMaterialId);
  void setCoreSkeleton(CalCoreSkeleton *pCoreSkeleton);
};
