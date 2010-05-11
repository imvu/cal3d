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
public:
  size_t getCoreMaterialCount() const {
    return m_mapCoreMaterialThread.size();
  }
  boost::shared_ptr<CalCoreMaterial> getCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId);
  void setCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId, const boost::shared_ptr<CalCoreMaterial>& coreMaterialId);

  const boost::shared_ptr<CalCoreSkeleton>& getCoreSkeleton() const {
    return m_pCoreSkeleton;
  }

  void setCoreSkeleton(const boost::shared_ptr<CalCoreSkeleton>& pCoreSkeleton) {
    m_pCoreSkeleton = pCoreSkeleton;
  }

  boost::shared_ptr<CalCoreSkeleton> m_pCoreSkeleton;

  typedef std::map<std::pair<int, int>, boost::shared_ptr<CalCoreMaterial> > MaterialMap;
  MaterialMap m_mapCoreMaterialThread;
};
