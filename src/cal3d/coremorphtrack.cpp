//****************************************************************************//
// coremorphtrack.cpp                                                              //
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

#include "cal3d/coremorphtrack.h"
#include "cal3d/error.h"
#include "cal3d/coremorphkeyframe.h"

CalCoreMorphTrack::CalCoreMorphTrack()
  : m_morphName("")
{
}

CalCoreMorphTrack::~CalCoreMorphTrack()
{
  //when CalCoreMorphTrack value objects die (from copying around etc), they might have keyframes still?
  //assert(m_keyframes.empty());
}

CAL3D_DEFINE_SIZE(CalCoreMorphKeyframe*);

size_t sizeInBytes(CalCoreMorphKeyframe const&) {
    return sizeof(CalCoreMorphKeyframe);
}

size_t CalCoreMorphTrack::size() const {
    return sizeof(*this) + m_morphName.capacity() + sizeInBytes(m_keyframesToDelete) + sizeInBytes(m_keyframes);
}

bool CalCoreMorphTrack::addCoreMorphKeyframe(CalCoreMorphKeyframe *pCoreMorphKeyframe)
{
  m_keyframes.push_back(*pCoreMorphKeyframe);
  m_keyframesToDelete.push_back(pCoreMorphKeyframe);
  int idx = m_keyframes.size() - 1;
  while (idx > 0 && m_keyframes[idx].time < m_keyframes[idx - 1].time) {
    std::swap(m_keyframes[idx], m_keyframes[idx - 1]);
    --idx;
  }

  return true;
}

 /*****************************************************************************/
/** Creates the core track instance.
  *
  * This function creates the core track instance.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreMorphTrack::create()
{
  return true;
}

 /*****************************************************************************/
/** Destroys the core track instance.
  *
  * This function destroys all data stored in the core track instance and frees
  * all allocated memory.
  *****************************************************************************/

void CalCoreMorphTrack::destroy()
{
  for (size_t i = 0; i < m_keyframesToDelete.size(); ++i)
  {
    delete m_keyframesToDelete[i];
  }
  m_keyframesToDelete.clear();

  m_morphName = "";
}

float CalCoreMorphTrack::getState(float time) {
  std::vector<CalCoreMorphKeyframe>::iterator iteratorCoreMorphKeyframeBefore;
  std::vector<CalCoreMorphKeyframe>::iterator iteratorCoreMorphKeyframeAfter;

  // get the keyframe after the requested time
  iteratorCoreMorphKeyframeAfter = getUpperBound(time);

  // check if the time is after the last keyframe
  if(iteratorCoreMorphKeyframeAfter == m_keyframes.end())
  {
    // return the last keyframe state
    --iteratorCoreMorphKeyframeAfter;
    return (*iteratorCoreMorphKeyframeAfter).weight;
  }

  // check if the time is before the first keyframe
  if(iteratorCoreMorphKeyframeAfter == m_keyframes.begin())
  {
    // return the first keyframe state
    return iteratorCoreMorphKeyframeAfter->weight;
  }

  // get the keyframe before the requested one
  iteratorCoreMorphKeyframeBefore = iteratorCoreMorphKeyframeAfter;
  --iteratorCoreMorphKeyframeBefore;

  // get the two keyframe pointers
  CalCoreMorphKeyframe* pCoreMorphKeyframeBefore = &(*iteratorCoreMorphKeyframeBefore);
  CalCoreMorphKeyframe* pCoreMorphKeyframeAfter = &(*iteratorCoreMorphKeyframeAfter);

  // calculate the blending factor between the two keyframe states
  float blendFactor = (time - pCoreMorphKeyframeBefore->time) / (pCoreMorphKeyframeAfter->time - pCoreMorphKeyframeBefore->time);

  // blend between the two keyframes
  float weight = pCoreMorphKeyframeBefore->weight;
  float otherWeight = pCoreMorphKeyframeAfter->weight;
  weight += blendFactor * (otherWeight - weight);
  return weight;
}

std::vector<CalCoreMorphKeyframe>::iterator CalCoreMorphTrack::getUpperBound(float time)
{

  int lowerBound = 0;
  int upperBound = m_keyframes.size()-1;

  while(lowerBound<upperBound-1)
  {
      int middle = (lowerBound+upperBound)/2;

      if(time >= m_keyframes[middle].time)
      {
          lowerBound=middle;
      }
      else
      {
          upperBound=middle;
      }
  }

  return m_keyframes.begin() + upperBound;

}

 /*****************************************************************************/
/** Sets the ID of the core bone.
  *
  * This function sets the ID of the core bone to which the core track instance
  * is attached to.
  *
  * @param morphname The ID of the bone to which the core track instance should
  *                   be attached to.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

void CalCoreMorphTrack::setMorphName(std::string morphname)
{
  m_morphName = morphname;
}


int CalCoreMorphTrack::getCoreMorphKeyframeCount()
{
  return m_keyframes.size();
}

void
CalCoreMorphTrack::reserve(int size)
{
  m_keyframes.reserve(size);
}




CalCoreMorphKeyframe* CalCoreMorphTrack::getCoreMorphKeyframe(int idx) 
{
  return &(m_keyframes[idx]);
}

void CalCoreMorphTrack::scale(float factor) {
    for(size_t keyframeId = 0; keyframeId < m_keyframes.size(); keyframeId++)
    {
        m_keyframes[keyframeId].weight *= factor;
    }
}

std::vector<CalCoreMorphKeyframe> &
CalCoreMorphTrack::getVectorCoreMorphKeyframes()
{
  return m_keyframes;
}
