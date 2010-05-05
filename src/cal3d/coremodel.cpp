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

static unsigned int const CalCoreModelMagic = 0x77884455;

 /*****************************************************************************/
/** Constructs the core model instance.
  *
  * This function is the default constructor of the core model instance.
  *****************************************************************************/

CalCoreModel::CalCoreModel()
  : m_pCoreSkeleton(0)
{
  m_magic = CalCoreModelMagic;
}

 /*****************************************************************************/
/** Destructs the core model instance.
  *
  * This function is the destructor of the core model instance.
  *****************************************************************************/

CalCoreModel::~CalCoreModel() {
  assert( m_magic == CalCoreModelMagic );

  if(m_pCoreSkeleton != 0)
  {
    m_pCoreSkeleton->destroy();
    delete m_pCoreSkeleton;
    m_pCoreSkeleton = 0;
  }
}



int CalCoreModel::addCoreMaterial(boost::shared_ptr<CalCoreMaterial> pCoreMaterial)
{
  // get the id of the core material
  int materialId;
  materialId = m_vectorCoreMaterial.size();

  m_vectorCoreMaterial.push_back(pCoreMaterial);

  return materialId;
}

boost::shared_ptr<CalCoreMaterial> CalCoreModel::getCoreMaterial(int coreMaterialId)
{
  if((coreMaterialId < 0) || (coreMaterialId >= (int)m_vectorCoreMaterial.size()))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return boost::shared_ptr<CalCoreMaterial>();
  }

  return m_vectorCoreMaterial[coreMaterialId];
}

int CalCoreModel::getCoreMaterialCount()
{
  return m_vectorCoreMaterial.size();
}

int CalCoreModel::getCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId)
{
    std::pair<int, int> key = std::make_pair(coreMaterialThreadId, coreMaterialSetId);
    if (m_mapCoreMaterialThread.count(key) == 0) {
        CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
        return -1;
    }

    return m_mapCoreMaterialThread[key];
}

CalCoreSkeleton *CalCoreModel::getCoreSkeleton()
{
  return m_pCoreSkeleton;
}

bool CalCoreModel::setCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId, int coreMaterialId)
{
  std::pair<int, int> key = std::make_pair(coreMaterialThreadId, coreMaterialSetId);
    
  // remove a possible entry in the core material thread
  m_mapCoreMaterialThread.erase(key);

  // set the given set id in the core material thread to the given core material id
  m_mapCoreMaterialThread.insert(std::make_pair(key, coreMaterialId));

  return true;
}

void CalCoreModel::setCoreSkeleton(CalCoreSkeleton *pCoreSkeleton)
{
  // destroy a possible existing core skeleton
  if(m_pCoreSkeleton != 0)
  {
    m_pCoreSkeleton->destroy();
    delete m_pCoreSkeleton;
  }

  m_pCoreSkeleton = pCoreSkeleton;
}
