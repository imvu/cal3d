//****************************************************************************//
// coremodel.cpp                                                              //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cal3d/coremodel.h"
#include "cal3d/error.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/coreanimation.h"
#include "cal3d/coreanimatedmorph.h"
#include "cal3d/coremesh.h"
#include "cal3d/corematerial.h"
#include "cal3d/loader.h"
#include "cal3d/saver.h"

boost::shared_ptr<CalCoreMaterial> CalCoreModel::getCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId)
{
    std::pair<int, int> key = std::make_pair(coreMaterialThreadId, coreMaterialSetId);
    if (m_mapCoreMaterialThread.count(key) == 0) {
        CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
        return boost::shared_ptr<CalCoreMaterial>();
    }

    return m_mapCoreMaterialThread[key];
}

void CalCoreModel::setCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId, const boost::shared_ptr<CalCoreMaterial>& coreMaterial)
{
  std::pair<int, int> key = std::make_pair(coreMaterialThreadId, coreMaterialSetId);
  m_mapCoreMaterialThread[key] = coreMaterial;
}
