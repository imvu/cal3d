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

CalAnimation::CalAnimation(const boost::shared_ptr<CalCoreAnimation>& pCoreAnimation)
  : time(0.0f)
  , timeFactor(1.0f)
  , weight(0.0f)
{
  scale = 1.0;
  rampValue = 1.0;

  m_pCoreAnimation = pCoreAnimation;
  compositionFunction = CompositionFunctionCrossFade; // Initially NULL (CrossFade) so we can recognize when it changes.
}
