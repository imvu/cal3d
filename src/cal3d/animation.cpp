//****************************************************************************//
// animation.cpp                                                              //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#include "cal3d/animation.h"
#include "cal3d/error.h"
#include "cal3d/coreanimation.h"

CalAnimation::CalAnimation()
   : m_type(TYPE_NONE)
   , m_state(STATE_NONE)
   , m_time(0.0f)
   , m_timeFactor(1.0f)
   , m_weight(0.0f)
{
}

CalAnimation::State CalAnimation::getState()
{
  return m_state;
}

float CalAnimation::getTime()
{
  return m_time;
}

CalAnimation::Type CalAnimation::getType()
{
  return m_type;
}

float CalAnimation::getWeight()
{
  return m_weight;
}

void CalAnimation::setTime(float time)
{
    m_time = time;
}

void CalAnimation::setTimeFactor(float timeFactor)
{
    m_timeFactor = timeFactor;
}

/*****************************************************************************/
/** Get the time factor.
  * 
  * This function return the time factor of the animation instance.
  *
  *****************************************************************************/

float CalAnimation::getTimeFactor()
{
    return m_timeFactor;
}


//****************************************************************************//
