//****************************************************************************//
// mixer.cpp                                                                  //
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

#include "cal3d/error.h"
#include "cal3d/mixer.h"
#include "cal3d/corebone.h"
#include "cal3d/coreanimation.h"
#include "cal3d/coretrack.h"
#include "cal3d/corekeyframe.h"
#include "cal3d/model.h"
#include "cal3d/skeleton.h"
#include "cal3d/bone.h"
#include "cal3d/animation.h"

 
CalMixer::~CalMixer()
{
  // destroy all active animation actions
  while(!m_listAnimationAction.empty())
  {
    CalAnimation* pAnimationAction = m_listAnimationAction.front();
    m_listAnimationAction.pop_front();

    delete pAnimationAction;
  }
}

CalAnimation* CalMixer::animationActionFromCoreAnimationId(const boost::shared_ptr<CalCoreAnimation>& coreAnimation) {
  std::list<CalAnimation*>::iterator iteratorAnimationAction;
  iteratorAnimationAction = m_listAnimationAction.begin();
  while(iteratorAnimationAction != m_listAnimationAction.end())
  {
    // update and check if animation action is still active
    CalAnimation* aa = *iteratorAnimationAction;
    boost::shared_ptr<CalCoreAnimation> ca = aa->getCoreAnimation();
    if( ca ) {
      if( ca == coreAnimation ) {
        return aa;
      }
    }
    ++iteratorAnimationAction;
  }
  return NULL;
}


 /*****************************************************************************/
/** Is action playing?
  *
  * Actions turn off automatically so you might need to know if one is playing.
  *
  * @param id The ID of the core animation.
  *
  * @return One of the following values:
  *         \li \b true if playing
  *         \li \b false if not
  *****************************************************************************/
bool CalMixer::actionOn(const boost::shared_ptr<CalCoreAnimation>& coreAnimation) {
  return animationActionFromCoreAnimationId( coreAnimation ) ? true : false;
}


 /*****************************************************************************/
/** Add a manual animation instance.
  *
  * Add a manual animation instance for this core animation if one
  * does not already exist.  Only one instance can exist per core animation.
  * A manual animation instance can be on or off while still existing.
  * If it is off, it retains its state (time, amplitude), but
  * doesn't have any effect on the skeleton.
  *
  * @param id The ID of the core animation.
  *
  * @return One of the following values:
  *         \li \b true if didn't already exist
  *         \li \b false if already existed or allocation failed
  *****************************************************************************/
bool 
CalMixer::addManualAnimation( const boost::shared_ptr<CalCoreAnimation>& coreAnimation )
{ 
  if( animationActionFromCoreAnimationId( coreAnimation ) ) {
    return false; // Already existed.
  }

  newAnimationAction(coreAnimation);
  return true;
}

 /*****************************************************************************/
/** Remove a manual animation instance.
  *
  * Remove a manual animation instance for this core animation if one
  * already exists.
  *
  * @param id The ID of the core animation.
  *
  * @return One of the following values:
  *         \li \b true if already exist
  *         \li \b false if didn't exist
  *****************************************************************************/
bool 
CalMixer::removeManualAnimation(const boost::shared_ptr<CalCoreAnimation>& coreAnimation)
{
  CalAnimation* aa = animationActionFromCoreAnimationId(coreAnimation);
  if( !aa ) return false;
  m_listAnimationAction.remove( aa );  
  delete aa;
  return true;
}


 /*****************************************************************************/
/** Sets all the manual animation attributes.
  *
  * Sets all the manual animation attributes.  Action must already be manual.
  *
  * @return One of the following values:
  *         \li \b true if exists and manual
  *         \li \b false otherwise
  *****************************************************************************/
bool
CalMixer::setManualAnimationAttributes(const boost::shared_ptr<CalCoreAnimation>& coreAnimation, CalMixerManualAnimationAttributes const & p)
{
  CalAnimation* aa = animationActionFromCoreAnimationId(coreAnimation);
  if( !aa ) return false;
  aa->time = p.time_;
  aa->weight = p.weight_;
  aa->scale = p.scale_;
  aa->rampValue = p.rampValue_;

  // now update composition function

  CalAnimation::CompositionFunction oldValue = aa->compositionFunction;

  // If the value isn't changing, then exit here.  Otherwise I would remove it and reinsert
  // it at the front, which wouldn't preserve the property that the most recently inserted
  // animation is highest priority.
  if( oldValue == p.compositionFunction_ ) {
      return true;
  }
  aa->compositionFunction = p.compositionFunction_;

  // Iterate through the list and remove this element.
  m_listAnimationAction.remove( aa );

  // Now insert it back in in the appropriate position.  Replace animations go in at the front.
  // Average animations go in after the replace animations.
  switch (p.compositionFunction_) {
    case CalAnimation::CompositionFunctionReplace: {
      // Replace animations go on the front of the list.
      m_listAnimationAction.push_front( aa );
      break;
    }
    case CalAnimation::CompositionFunctionCrossFade: {
      // Average animations go after replace, but before Average.
      std::list<CalAnimation *>::iterator aait2;
      for( aait2 = m_listAnimationAction.begin(); aait2 != m_listAnimationAction.end(); aait2++ ) {
        CalAnimation * aa3 = * aait2;
        CalAnimation::CompositionFunction cf = aa3->compositionFunction;
        if( cf != CalAnimation::CompositionFunctionReplace ) {
          break;
        }
      }
      m_listAnimationAction.insert( aait2, aa );
      break;
    }
    case CalAnimation::CompositionFunctionAverage: {
      // Average animations go before the first Average animation.
      std::list<CalAnimation *>::iterator aait2;
      for( aait2 = m_listAnimationAction.begin(); aait2 != m_listAnimationAction.end(); aait2++ ) {
        CalAnimation * aa3 = * aait2;
        CalAnimation::CompositionFunction cf = aa3->compositionFunction;
        if( cf == CalAnimation::CompositionFunctionAverage ) { // Skip over replace and crossFade animations
          break;
        }
      }
      m_listAnimationAction.insert( aait2, aa );
      break;
    }
    default: {
      assert( !"Unexpected" );
      break;
    }
  }
  return true;
}



 /*****************************************************************************/
/** Stop the action.
  *
  * Turn off an action.
  *
  * @param id The ID of the core animation.
  *
  * @return One of the following values:
  *         \li \b true was playing (now stopped)
  *         \li \b false if already not playing
  *****************************************************************************/
bool
CalMixer::stopAction( const boost::shared_ptr<CalCoreAnimation>& coreAnimation )
{
  CalAnimation * aa = animationActionFromCoreAnimationId( coreAnimation );
  if( !aa ) return false;
  m_listAnimationAction.remove( aa );  
  delete aa;
  return true;
}



CalMixer::CalMixer() {
  // set the animation time/duration values to default
  m_animationTime = 0.0f;
  m_animationDuration = 0.0f;
  timeFactor = 1.0f;
  m_numBoneAdjustments = 0;
}

CalAnimation * CalMixer::newAnimationAction(const boost::shared_ptr<CalCoreAnimation>& pCoreAnimation) {
  // allocate a new animation action instance
  CalAnimation* pAnimationAction = new CalAnimation(pCoreAnimation);

  // insert new animation into the table
  m_listAnimationAction.push_front(pAnimationAction);
  return pAnimationAction;
}



 /*****************************************************************************/
/** Updates all active animations.
  *
  * This function updates all active non-manual animations of the mixer instance for a
  * given amount of time.  If you only use manual animations, you don't need
  * to call this function.
  *
  * @param deltaTime The elapsed time in seconds since the last update.
  *****************************************************************************/

void CalMixer::updateAnimation(float deltaTime)
{
  // update the current animation time
  if(m_animationDuration == 0.0f)
  {
    m_animationTime = 0.0f;
  }
  else
  {
    m_animationTime += deltaTime * timeFactor;
    if(m_animationTime >= m_animationDuration)
    {
      m_animationTime = (float) fmod(m_animationTime, m_animationDuration);
    }
	if (m_animationTime < 0)
      m_animationTime += m_animationDuration;

  }
}


void CalMixer::applyBoneAdjustments(CalSkeleton* pSkeleton) {
  std::vector<CalBone>& vectorBone = pSkeleton->getVectorBone();
  unsigned int i;
  for( i = 0; i < m_numBoneAdjustments; i++ ) {
    CalMixerBoneAdjustmentAndBoneId * ba = & m_boneAdjustmentAndBoneIdArray[ i ];
    CalBone * bo = &vectorBone[ ba->boneId_ ];
    const CalCoreBone& cbo = bo->getCoreBone();
    if( ba->boneAdjustment_.flags_ & CalMixerBoneAdjustmentFlagMeshScale ) {
      bo->setMeshScaleAbsolute( ba->boneAdjustment_.meshScaleAbsolute_ );
    }
    if( ba->boneAdjustment_.flags_ & CalMixerBoneAdjustmentFlagPosRot ) {
      const CalVector & localPos = cbo.getTranslation();
      CalVector adjustedLocalPos = localPos;
      CalQuaternion adjustedLocalOri = ba->boneAdjustment_.localOri_;
      static float const scale = 1.0f;
      float rampValue = ba->boneAdjustment_.rampValue_;
      static bool const replace = true;
      static float const unrampedWeight = 1.0f;
      bo->blendState( unrampedWeight, 
        adjustedLocalPos,
        adjustedLocalOri, 
        scale, replace, rampValue );
    }
  }
}

bool
CalMixer::addBoneAdjustment( int boneId, CalMixerBoneAdjustment const & ba )
{
  if( m_numBoneAdjustments == CalMixerBoneAdjustmentsMax ) return false;
  m_boneAdjustmentAndBoneIdArray[ m_numBoneAdjustments ].boneAdjustment_ = ba;
  m_boneAdjustmentAndBoneIdArray[ m_numBoneAdjustments ].boneId_ = boneId;
  m_numBoneAdjustments++;
  return true;
}

void
CalMixer::removeAllBoneAdjustments()
{
  m_numBoneAdjustments = 0;
}

bool 
CalMixer::removeBoneAdjustment( int boneId )
{
  unsigned int i;
  for( i = 0; i < m_numBoneAdjustments; i++ ) {
    CalMixerBoneAdjustmentAndBoneId * ba = & m_boneAdjustmentAndBoneIdArray[ i ];
    if( ba->boneId_ == boneId ) break;
  }
  if( i == m_numBoneAdjustments ) return false; // Couldn't find it.
  i++;
  while( i < m_numBoneAdjustments ) {
    m_boneAdjustmentAndBoneIdArray[ i - 1 ] = m_boneAdjustmentAndBoneIdArray[ i ];
    i++;
  }
  m_numBoneAdjustments--;
  return true;
}


void CalMixer::updateSkeleton(CalSkeleton* pSkeleton) {
  pSkeleton->clearState();

  // get the bone vector of the skeleton
  std::vector<CalBone>& vectorBone = pSkeleton->getVectorBone();

  // The bone adjustments are "replace" so they have to go first, giving them
  // highest priority and full influence.  Subsequent animations affecting the same bones, 
  // including subsequent replace animations, will have their incluence attenuated appropriately.
  applyBoneAdjustments(pSkeleton);

  // loop through all animation actions
  std::list<CalAnimation *>::iterator itaa;
  for( itaa = m_listAnimationAction.begin(); itaa != m_listAnimationAction.end(); itaa++ ) {

    // get the core animation instance
    CalAnimation * aa = * itaa;
    
    const boost::shared_ptr<CalCoreAnimation>& pCoreAnimation = aa->getCoreAnimation();
    
    // get the list of core tracks of above core animation
    CalCoreAnimation::TrackList& listCoreTrack = pCoreAnimation->tracks;
    
    // loop through all core tracks of the core animation
    CalCoreAnimation::TrackList::iterator itct;
    for( itct = listCoreTrack.begin(); itct != listCoreTrack.end(); itct++ ) {
      
      // get the appropriate bone of the track
      CalCoreTrack* ct = itct->get();
      if( ct->coreBoneId >= int(vectorBone.size()) ) {
        continue;
      }
      CalBone * pBone = &vectorBone[ct->coreBoneId];
      
      // get the current translation and rotation
      CalVector translation;
      CalQuaternion rotation;
      ct->getState(aa->time, translation, rotation);
      
      // Replace and CrossFade both blend with the replace function.
      bool replace = aa->compositionFunction != CalAnimation::CompositionFunctionAverage;
      pBone->blendState(aa->weight, translation, rotation, aa->scale, replace, aa->rampValue);
    }
  }

  // === What does lockState() mean?  Why do we need it at all?  It seems only to allow us
  // to blend all the animation actions together into a temporary sum, and then
  // blend all the animation cycles together into a different sum, and then blend
  // the two sums together according to their relative weight sums.  I believe this is mathematically
  // equivalent of blending all the animation actions and cycles together into a single sum,
  // according to their relative weights.
  pSkeleton->lockState();

  // let the skeleton calculate its final state
  pSkeleton->calculateState();
}

/*****************************************************************************/
/** Returns the animation time.
  *
  * This function returns the animation time of the mixer instance.
  *
  * @return The animation time in seconds.
  *****************************************************************************/


float CalMixer::getAnimationTime()
{
	return m_animationTime;
}

/*****************************************************************************/
/** Returns the animation duration.
  *
  * This function returns the animation duration of the mixer instance.
  *
  * @return The animation duration in seconds.
  *****************************************************************************/


float CalMixer::getAnimationDuration()
{
	return m_animationDuration;
}


/*****************************************************************************/
/** Sets the animation time.
  *
  * This function sets the animation time of the mixer instance.
  *
  *****************************************************************************/


void CalMixer::setAnimationTime(float animationTime)
{
	m_animationTime=animationTime;
}
