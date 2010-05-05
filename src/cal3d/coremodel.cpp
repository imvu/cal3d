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

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

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



int 

CalCoreModel::getNumCoreAnimatedMorphs()

{

  int num = m_vectorCoreAnimatedMorph.size();

  int r = 0;

  int i;

  for( i = 0; i < num; i++ ) {

    if( m_vectorCoreAnimatedMorph[ i ] ) r++;

  }

  return r;

}


 /*****************************************************************************/
/** Adds a core animated morph (different from a morph animation).
  *
  * Loads a core animated morph (different from a morph animation), and adds
  * it to the model instance.  The model instance will free the loaded core
  * animated more when the model instance is freed.
  *
  * @param pCoreAnimation A pointer to the core animated morph that should be added.
  *
  * @return One of the following values:
  *         \li the assigned animation \b ID of the added core animated morph
  *         \li \b -1 if an error happend
  *****************************************************************************/

int CalCoreModel::addCoreAnimatedMorph(const boost::shared_ptr<CalCoreAnimatedMorph>& pCoreAnimatedMorph)
{
  int num = m_vectorCoreAnimatedMorph.size();

  int i;

  for( i = 0; i < num; i++ ) {

    if( !m_vectorCoreAnimatedMorph[ i ] ) {

      m_vectorCoreAnimatedMorph[ i ] = pCoreAnimatedMorph;

      return i;

    }

  }

  m_vectorCoreAnimatedMorph.push_back(pCoreAnimatedMorph);

  return num;

}


 /*****************************************************************************/
/** Adds a core material.
  *
  * This function adds a core material to the core model instance.
  *
  * @param pCoreMaterial A pointer to the core material that should be added.
  *
  * @return One of the following values:
  *         \li the assigned material \b ID of the added core material
  *         \li \b -1 if an error happend
  *****************************************************************************/

int CalCoreModel::addCoreMaterial(boost::shared_ptr<CalCoreMaterial> pCoreMaterial)
{
  // get the id of the core material
  int materialId;
  materialId = m_vectorCoreMaterial.size();

  m_vectorCoreMaterial.push_back(pCoreMaterial);

  return materialId;
}

 /*****************************************************************************/
/** Creates the core model instance.
  *
  * This function creates the core model instance.
  *
  * @param strName A string that should be used as the name of the core model
  *                instance.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreModel::createInternal(const std::string& strName)
{
  assert(m_magic == CalCoreModelMagic );
  m_strName = strName;
  return true;
}

bool CalCoreModel::createWithName( char const * strName)
{
  std::string name = strName;
  return createInternal( name );
}

 /*****************************************************************************/
/** Provides access to a core morph animation.
  *
  * This function returns the core morph animation with the given ID.
  *
  * @param coreAnimatedMorphId The ID of the core morph animation that should be returned.
  *
  * @return One of the following values:
  *         \li a pointer to the core morph animation
  *         \li \b 0 if an error happend
  *****************************************************************************/

boost::shared_ptr<CalCoreAnimatedMorph> CalCoreModel::getCoreAnimatedMorph(int coreAnimatedMorphId)
{
  if((coreAnimatedMorphId < 0) 

    || (coreAnimatedMorphId >= (int)m_vectorCoreAnimatedMorph.size())

    || !m_vectorCoreAnimatedMorph[ coreAnimatedMorphId ] )

  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return boost::shared_ptr<CalCoreAnimatedMorph>();
  }

  return m_vectorCoreAnimatedMorph[coreAnimatedMorphId];
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

 /*****************************************************************************/
/** Returns the number of core materials.
  *
  * This function returns the number of core materials in the core model
  * instance.
  *
  * @return The number of core materials.
  *****************************************************************************/

int CalCoreModel::getCoreMaterialCount()
{
  return m_vectorCoreMaterial.size();
}

 /*****************************************************************************/
/** Returns a specified core material ID.
  *
  * This function returns the core material ID for a specified core material
  * thread / core material set pair.
  *
  * @param coreMaterialThreadId The ID of the core material thread.
  * @param coreMaterialSetId The ID of the core material set.
  *
  * @return One of the following values:
  *         \li the \b ID of the core material
  *         \li \b -1 if an error happend
  *****************************************************************************/

int CalCoreModel::getCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId)
{
    std::pair<int, int> key = std::make_pair(coreMaterialThreadId, coreMaterialSetId);
    if (m_mapCoreMaterialThread.count(key) == 0) {
        CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
        return -1;
    }

    return m_mapCoreMaterialThread[key];
}

 /*****************************************************************************/
/** Provides access to the core skeleton.
  *
  * This function returns the core skeleton.
  *
  * @return One of the following values:
  *         \li a pointer to the core skeleton
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalCoreSkeleton *CalCoreModel::getCoreSkeleton()
{
  return m_pCoreSkeleton;
}

 /*****************************************************************************/
/** Sets a core material ID.
  *
  * This function sets a core material ID for a core material thread / core
  * material set pair.
  *
  * @param coreMaterialThreadId The ID of the core material thread.
  * @param coreMaterialSetId The ID of the core maetrial set.
  * @param coreMaterialId The ID of the core maetrial.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreModel::setCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId, int coreMaterialId)
{
  std::pair<int, int> key = std::make_pair(coreMaterialThreadId, coreMaterialSetId);
    
  // remove a possible entry in the core material thread
  m_mapCoreMaterialThread.erase(key);

  // set the given set id in the core material thread to the given core material id
  m_mapCoreMaterialThread.insert(std::make_pair(key, coreMaterialId));

  return true;
}

 /*****************************************************************************/
/** Sets the core skeleton.
  *
  * This function sets the core skeleton of the core model instance..
  *
  * @param pCoreSkeleton The core skeleton that should be set.
  *****************************************************************************/

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
