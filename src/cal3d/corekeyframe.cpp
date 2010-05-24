//****************************************************************************//
// corekeyframe.cpp                                                           //
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

#include "cal3d/corekeyframe.h"

CalCoreKeyframe::CalCoreKeyframe()
  : m_time(0.0f)
{
}

const CalQuaternion& CalCoreKeyframe::getRotation()
{
  return m_rotation;
}

float CalCoreKeyframe::getTime()
{
  return m_time;
}

const CalVector& CalCoreKeyframe::getTranslation()
{
  return m_translation;
}

void CalCoreKeyframe::setRotation(const CalQuaternion& rotation)
{
  m_rotation = rotation;
}

void CalCoreKeyframe::setTime(float time)
{
  m_time = time;
}

void CalCoreKeyframe::setTranslation(const CalVector& translation)
{
  m_translation = translation;
}
