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

  // destroy all core animated morphs
  std::vector<CalCoreAnimatedMorph *>::iterator iteratorCoreAnimatedMorph;
  for(iteratorCoreAnimatedMorph = m_vectorCoreAnimatedMorph.begin(); iteratorCoreAnimatedMorph != 
      m_vectorCoreAnimatedMorph.end(); ++iteratorCoreAnimatedMorph)
  {
    // I am always managing coreAnimatedMorphs.  However, since I allow the user to remove them,
    // I may still have gaps in my array.
    if( * iteratorCoreAnimatedMorph ) {

      (*iteratorCoreAnimatedMorph)->destroy();
      delete (*iteratorCoreAnimatedMorph);

    }
  }

  if(m_pCoreSkeleton != 0)
  {
    m_pCoreSkeleton->destroy();
    delete m_pCoreSkeleton;
    m_pCoreSkeleton = 0;
  }
}



int 

CalCoreModel::getNumCoreAnimations()

{

  int num = m_vectorCoreAnimation.size();

  int r = 0;

  int i;

  for( i = 0; i < num; i++ ) {

    if( m_vectorCoreAnimation[ i ] ) r++;

  }

  return r;

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


int CalCoreModel::addCoreAnimation(const boost::shared_ptr<CalCoreAnimation>& pCoreAnimation)
{
  int num = m_vectorCoreAnimation.size();
  for (int i = 0; i < num; i++) {
    if( !m_vectorCoreAnimation[ i ] ) {
      m_vectorCoreAnimation[ i ] = pCoreAnimation;
      return i;
    }
  }

  m_vectorCoreAnimation.push_back(pCoreAnimation);
  return num;
}


bool CalCoreModel::removeCoreAnimation( int id )

{

  int num = m_vectorCoreAnimation.size();

  if( id >= num || id < 0) return false;

  if( !m_vectorCoreAnimation[ id ] ) return false;

  m_vectorCoreAnimation[ id ].reset();

  return true;

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

int CalCoreModel::addCoreAnimatedMorph(CalCoreAnimatedMorph *pCoreAnimatedMorph)
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


bool CalCoreModel::removeCoreAnimatedMorph( int id )

{

  int num = m_vectorCoreAnimatedMorph.size();

  if( id >= num || id < 0) return false;

  if( !m_vectorCoreAnimatedMorph[ id ] ) return false;

  m_vectorCoreAnimatedMorph[ id ] = NULL;

  return true;

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

boost::shared_ptr<CalCoreAnimation> CalCoreModel::getCoreAnimation(int coreAnimationId) {
  if((coreAnimationId < 0) 
    || (coreAnimationId >= (int)m_vectorCoreAnimation.size())
    || !m_vectorCoreAnimation[ coreAnimationId ] )
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return boost::shared_ptr<CalCoreAnimation>();
  }

  return m_vectorCoreAnimation[coreAnimationId];
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

CalCoreAnimatedMorph *CalCoreModel::getCoreAnimatedMorph(int coreAnimatedMorphId)
{
  if((coreAnimatedMorphId < 0) 

    || (coreAnimatedMorphId >= (int)m_vectorCoreAnimatedMorph.size())

    || !m_vectorCoreAnimatedMorph[ coreAnimatedMorphId ] )

  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return 0;
  }

  return m_vectorCoreAnimatedMorph[coreAnimatedMorphId];
}


 /*****************************************************************************/
/** Returns the number of core animations.
  *
  * This function returns the number of core animations in the core model
  * instance.
  *
  * @return The number of core animations.
  *****************************************************************************/

int CalCoreModel::getCoreAnimationMaxId()
{
  return m_vectorCoreAnimation.size();
}


 /*****************************************************************************/
/** Provides access to a core material.
  *
  * This function returns the core material with the given ID.
  *
  * @param coreMaterialId The ID of the core material that should be returned.
  *
  * @return One of the following values:
  *         \li a pointer to the core material
  *         \li \b 0 if an error happend
  *****************************************************************************/

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
/** Loads a core animation.
  *
  * This function loads a core animation from a file.
  *
  * @param strFilename The file from which the core animation should be loaded
  *                    from.
  *
  * @return One of the following values:
  *         \li the assigned \b ID of the loaded core animation
  *         \li \b -1 if an error happend
  *****************************************************************************/

int CalCoreModel::loadCoreAnimatedMorph(const std::string& strFilename)
{

  // load a new core AnimatedMorph
  CalCoreAnimatedMorph *pCoreAnimatedMorph = CalLoader::loadCoreAnimatedMorph(strFilename);
  if(pCoreAnimatedMorph == 0) return -1;

  // add core AnimatedMorph to this core model
  int animatedMorphId = addCoreAnimatedMorph(pCoreAnimatedMorph);
  if(animatedMorphId == -1)
  {
    delete pCoreAnimatedMorph;
    return -1;
  }

  return animatedMorphId;
}


 /*****************************************************************************/
/** Loads the core skeleton.
  *
  * This function loads the core skeleton from a file.
  *
  * @param strFilename The file from which the core skeleton should be loaded
  *                    from.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreModel::loadCoreSkeleton(const std::string& strFilename)
{
  // destroy the current core skeleton
  if(m_pCoreSkeleton != 0)
  {
    m_pCoreSkeleton->destroy();
    delete m_pCoreSkeleton;
  }

  // load a new core skeleton
  m_pCoreSkeleton = CalLoader::loadCoreSkeleton(strFilename);
  if(m_pCoreSkeleton == 0) return false;

  return true;
}

 /*****************************************************************************/
/** Saves a core material.
  *
  * This function saves a core material to a file.
  *
  * @param strFilename The file to which the core material should be saved to.
  * @param coreMaterialId The ID of the core material that should be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreModel::saveCoreMaterial(const std::string& strFilename, int coreMaterialId)
{
  // check if the core material id is valid
  if((coreMaterialId < 0) || (coreMaterialId >= (int)m_vectorCoreMaterial.size()))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return false;
  }

  // save the core animation
  if(!CalSaver::saveCoreMaterial(strFilename, m_vectorCoreMaterial[coreMaterialId].get()))
  {
    return false;
  }

  return true;
}

 /*****************************************************************************/
/** Saves the core skeleton.
  *
  * This function saves the core skeleton to a file.
  *
  * @param strFilename The file to which the core skeleton should be saved to.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreModel::saveCoreSkeleton(const std::string& strFilename)
{
  // check if we have a core skeleton in this code model
  if(m_pCoreSkeleton == 0)
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return false;
  }

  // save the core skeleton
  if(!CalSaver::saveCoreSkeleton(strFilename, m_pCoreSkeleton))
  {
    return false;
  }

  return true;
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
