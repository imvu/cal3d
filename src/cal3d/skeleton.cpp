//****************************************************************************//
// calskeleton.cpp                                                            //
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

#include "cal3d/skeleton.h"
#include "cal3d/error.h"
#include "cal3d/bone.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/coremodel.h"
#include "cal3d/corebone.h" // DEBUG

 /*****************************************************************************/
/** Constructs the skeleton instance.
  *
  * This function is the default constructor of the skeleton instance.
  *****************************************************************************/

CalSkeleton::CalSkeleton()
  : m_pCoreSkeleton(0),m_isBoundingBoxesComputed(false)
{ 
}

 /*****************************************************************************/
/** Destructs the skeleton instance.
  *
  * This function is the destructor of the skeleton instance.
  *****************************************************************************/

CalSkeleton::~CalSkeleton()
{
  assert(m_vectorBone.empty());
}

 /*****************************************************************************/
/** Calculates the state of the skeleton instance.
  *
  * This function calculates the state of the skeleton instance by recursively
  * calculating the states of its bones.
  *****************************************************************************/

void CalSkeleton::calculateState()
{
  // calculate all bone states of the skeleton
  std::list<int>& listRootCoreBoneId = m_pCoreSkeleton->getListRootCoreBoneId();

  std::list<int>::iterator iteratorRootBoneId;
  for(iteratorRootBoneId = listRootCoreBoneId.begin(); iteratorRootBoneId != listRootCoreBoneId.end(); ++iteratorRootBoneId)
  {
    m_vectorBone[*iteratorRootBoneId]->calculateState();
  }
  m_isBoundingBoxesComputed=false;
}

 /*****************************************************************************/
/** Clears the state of the skeleton instance.
  *
  * This function clears the state of the skeleton instance by recursively
  * clearing the states of its bones.
  *****************************************************************************/

void CalSkeleton::clearState()
{
  // clear all bone states of the skeleton
  std::vector<CalBone *>::iterator iteratorBone;
  for(iteratorBone = m_vectorBone.begin(); iteratorBone != m_vectorBone.end(); ++iteratorBone)
  {
    (*iteratorBone)->clearState();
  }
  m_isBoundingBoxesComputed=false;
}

 /*****************************************************************************/
/** Creates the skeleton instance.
  *
  * This function creates the skeleton instance based on a core skeleton.
  *
  * @param pCoreSkeleton A pointer to the core skeleton on which this skeleton
  *                      instance should be based on.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

void CalSkeleton::create(CalCoreSkeleton *pCoreSkeleton)
{
  assert(pCoreSkeleton);

  m_pCoreSkeleton = pCoreSkeleton;

  // clone the skeleton structure of the core skeleton
  std::vector<CalCoreBone *>& vectorCoreBone = pCoreSkeleton->getVectorCoreBone();

  // get the number of bones
  int boneCount;
  boneCount = vectorCoreBone.size();

  // reserve space in the bone vector
  m_vectorBone.reserve(boneCount);

  // clone every core bone
  int boneId;
  for(boneId = 0; boneId < boneCount; ++boneId)
  {
    CalBone* pBone = new CalBone(vectorCoreBone[boneId]);

    // set skeleton in the bone instance
    pBone->setSkeleton(this);

    // insert bone into bone vector
    m_vectorBone.push_back(pBone);
  }
}

 /*****************************************************************************/
/** Destroys the skeleton instance.
  *
  * This function destroys all data stored in the skeleton instance and frees
  * all allocated memory.
  *****************************************************************************/

void CalSkeleton::destroy()
{
  // destroy all bones
  std::vector<CalBone *>::iterator iteratorBone;
  for(iteratorBone = m_vectorBone.begin(); iteratorBone != m_vectorBone.end(); ++iteratorBone)
  {
    delete (*iteratorBone);
  }
  m_vectorBone.clear();

  m_pCoreSkeleton = 0;
}

 /*****************************************************************************/
/** Provides access to a bone.
  *
  * This function returns the bone with the given ID.
  *
  * @param boneId The ID of the bone that should be returned.
  *
  * @return One of the following values:
  *         \li a pointer to the bone
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalBone *CalSkeleton::getBone(int boneId)
{
  return m_vectorBone[boneId];
}

 /*****************************************************************************/
/** Provides access to the core skeleton.
  *
  * This function returns the core skeleton on which this skeleton instance is
  * based on.
  *
  * @return One of the following values:
  *         \li a pointer to the core skeleton
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalCoreSkeleton *CalSkeleton::getCoreSkeleton()
{
  return m_pCoreSkeleton;
}

 /*****************************************************************************/
/** Returns the bone vector.
  *
  * This function returns the vector that contains all bones of the skeleton
  * instance.
  *
  * @return A reference to the bone vector.
  *****************************************************************************/

std::vector<CalBone *>& CalSkeleton::getVectorBone()
{
  return m_vectorBone;
}

 /*****************************************************************************/
/** Locks the state of the skeleton instance.
  *
  * This function locks the state of the skeleton instance by recursively
  * locking the states of its bones.
  *****************************************************************************/

void CalSkeleton::lockState()
{
  // lock all bone states of the skeleton
  std::vector<CalBone *>::iterator iteratorBone;
  for(iteratorBone = m_vectorBone.begin(); iteratorBone != m_vectorBone.end(); ++iteratorBone)
  {
    (*iteratorBone)->lockState();
  }
}

