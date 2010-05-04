//****************************************************************************//
// coreanimation.cpp                                                          //
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

#include "cal3d/coreanimation.h" 
#include "cal3d/coretrack.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h"

CalCoreAnimation::CalCoreAnimation()
{
}

CalCoreAnimation::~CalCoreAnimation()
{
  // destroy all core tracks
  while(!m_listCoreTrack.empty())
  {
    CalCoreTrack *pCoreTrack;
    pCoreTrack = m_listCoreTrack.front();
    m_listCoreTrack.pop_front();

    pCoreTrack->destroy();
    delete pCoreTrack;
  }
}

bool CalCoreAnimation::addCoreTrack(CalCoreTrack *pCoreTrack) {
  m_listCoreTrack.push_back(pCoreTrack);

  return true;
}

size_t CalCoreAnimation::size() {
  size_t r = sizeof( *this );
  std::list<CalCoreTrack *>::iterator iter1;
  for( iter1 = m_listCoreTrack.begin(); iter1 != m_listCoreTrack.end(); ++iter1 ) {
    r += (*iter1)->size() + sizeof(iter1); // Bi-directional list has two pointers.
  }
  return r;
}


CalCoreTrack *CalCoreAnimation::getCoreTrack(int coreBoneId)
{
  // loop through all core track
  std::list<CalCoreTrack *>::iterator iteratorCoreTrack;
  for(iteratorCoreTrack = m_listCoreTrack.begin(); iteratorCoreTrack != m_listCoreTrack.end(); ++iteratorCoreTrack)
  {
    // get the core bone
    CalCoreTrack *pCoreTrack;
    pCoreTrack = *iteratorCoreTrack;

    // check if we found the matching core bone
    if(pCoreTrack->getCoreBoneId() == coreBoneId) return pCoreTrack;
  }

  // no match found
  return 0;
}

void CalCoreAnimation::fillInvalidTranslations( CalCoreSkeleton * skel ) {
  std::list<CalCoreTrack *>::iterator iteratorCoreTrack;
  for(iteratorCoreTrack = m_listCoreTrack.begin(); iteratorCoreTrack != m_listCoreTrack.end(); ++iteratorCoreTrack)
  {
    CalCoreTrack * tr = * iteratorCoreTrack;
    int boneId = tr->getCoreBoneId();
    assert( boneId != -1 );
    CalCoreBone * bo = skel->getCoreBone( boneId );
    if( bo ) {
      CalVector trans = bo->getTranslation();
      tr->fillInvalidTranslations( trans );
    }
  }
}


unsigned int CalCoreAnimation::numCoreTracks() {
  return m_listCoreTrack.size();
}

CalCoreTrack* CalCoreAnimation::nthCoreTrack(unsigned int i) {
  std::list<CalCoreTrack *>::iterator iteratorCoreTrack;
  for( iteratorCoreTrack = m_listCoreTrack.begin(); iteratorCoreTrack != m_listCoreTrack.end(); ++iteratorCoreTrack ) {
    if( i == 0 ) return * iteratorCoreTrack;
    i--;
  }
  return NULL;
}


float CalCoreAnimation::getDuration()
{
  return m_duration;
}

std::list<CalCoreTrack *>& CalCoreAnimation::getListCoreTrack()
{
  return m_listCoreTrack;
}

void CalCoreAnimation::setDuration(float duration)
{
  m_duration = duration;
}

void CalCoreAnimation::scale(float factor)
{
  // loop through all core track
  std::list<CalCoreTrack *>::iterator iteratorCoreTrack;
  for(iteratorCoreTrack = m_listCoreTrack.begin(); iteratorCoreTrack != m_listCoreTrack.end(); ++iteratorCoreTrack)
  {
	  (*iteratorCoreTrack)->scale(factor);
  }
}
