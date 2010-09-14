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
  m_scale = 1.0;

  m_pCoreAnimation = pCoreAnimation;
}

/*****************************************************************************/
/** Executes the animation action instance.
  *
  * This function executes the animation action instance.  You cannot execute
  * a manual action.
  *
  * @param delayIn The time in seconds until the animation action instance
  *                reaches the full weight from the beginning of its execution.
  * @param delayOut The time in seconds in which the animation action instance
  *                 reaches zero weight at the end of its execution.
  * @param weightTarget No doxygen comment for this. FIXME.
  * @param autoLock     This prevents the Action from being reset and removed
  *                     on the last keyframe if true.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if was manual
  *****************************************************************************/

bool CalAnimation::execute(float delayIn, float delayOut, float weightTarget, bool autoLock) {
  return false;
}

 /*****************************************************************************/
/** Configures the action to be a manual action, and on.
  *
  * Configures the action to be a manual action, which the mixer does not
  * call update() on.  Once configuring an animation to be manual, you cannot
  * change it back to non-manual.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/
void CalAnimation::setManual() {
  weight = 0.0f;
  time = 0.0f;
  rampValue = 1.0;
  m_compositionFunction = CompositionFunctionNull; // Initially NULL so we can recognize when it changes.
}

 /*****************************************************************************/
/** Sets the composition function, which controls how animation blends with other simultaneous animations.
  *
  * If you set it to Replace, then when the animation is fully ramped on, all non-Replace
  * and lower priority Replace animations will have zero influence.  This
  * factor does not apply to cycling animations.  The priority of animations is,
  * firstly whether they are Replace or not, and secondly how recently the animations were
  * added, the most recently added animations having higher priority.
  *
  * @return One of the following values:
  *         \li \b true if not setting to CompositionFunctionNull
  *         \li \b false if setting to CompositionFunctionNull
  *****************************************************************************/
bool 
CalAnimation::setCompositionFunction( CompositionFunction p )
{
  if( p == CompositionFunctionNull ) return false;
  if( m_compositionFunction == p ) return true;
  m_compositionFunction = p;
  return true;
}

 /*****************************************************************************/
/** Gets the CompositionFunction of the animation.
  *
  * Gets the CompositionFunction of the animation.  See setRampValue().
  *
  * @return \li \b CompositionFunction value that was set with setCompositionFunction().
  *****************************************************************************/
CalAnimation::CompositionFunction 
CalAnimation::getCompositionFunction()
{
  return m_compositionFunction;
}


 /*****************************************************************************/
/** Sets the scale of the animation.
  *
  * Sets the scale of the animation.  Scale is different from weight.  Weight
  * is really relative weight.  All the weights are combined into a sum, and
  * each animation contributes according to the ratio of its weight to the sum.
  * In other words, the total influence of the weights is normalized to 1.0.
  * In contrast, scale factors apply to animation actions independently.  Scaling
  * one animation action up does not decrease the scale of other actions applying
  * to the same bones.  Whereas the weights are used to compose animations onto
  * a skeleton, scales are used to adjust the absolute, non-relative magnitude
  * of an animation.
  *
  * @return One of the following values:
  *         \li \b true if manual
  *         \li \b false if not manual
  *****************************************************************************/
