//****************************************************************************//
// coretrack.cpp                                                              //
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

#include "cal3d/coretrack.h"
#include "cal3d/corebone.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/error.h"
#include "cal3d/corekeyframe.h"
#include "cal3d/loader.h"

CalCoreTrack::CalCoreTrack(int coreBone)
  : coreBoneId(coreBone)
{
  m_translationRequired = true;
  m_translationIsDynamic = true;
}

size_t sizeInBytes(CalCoreKeyframe* const&) {
  return sizeof(CalCoreKeyframe*) + sizeof(CalCoreKeyframe);
}

size_t CalCoreTrack::sizeInBytes() const {
  return sizeof(CalCoreTrack) + ::sizeInBytes(m_keyframes);
}

bool CalCoreTrack::addCoreKeyframe(CalCoreKeyframe *pCoreKeyframe) {
  m_keyframes.push_back(pCoreKeyframe);
  int idx = m_keyframes.size() - 1;
  while (idx > 0 && m_keyframes[idx]->time < m_keyframes[idx - 1]->time) {
    std::swap(m_keyframes[idx], m_keyframes[idx - 1]);
    --idx;
  }

  return true;
}

inline float DistanceSquared( CalVector const & v1, CalVector const & v2 ) {
  float dx = ( v1.x - v2.x );
  float dy = ( v1.y - v2.y );
  float dz = ( v1.z - v2.z );
  return dx * dx + dy * dy + dz * dz;
}

inline float Distance( CalVector const & p1, CalVector const & p2 ) {
  return sqrtf( DistanceSquared( p1, p2 ) );
}

float DistanceDegrees( CalQuaternion const & p1, CalQuaternion const & p2 ) {

  // To determine the angular distance between the oris, multiply one by the inverse
  // of the other, which should leave us with an identity ori if they are equal.  If
  // they are not equal, then the angular magnitude of the rotation in degrees is the
  // difference between the oris.
  CalQuaternion odist = p1;
  odist.invert();
  odist *= p2;
  float w = odist.w;
  if( w > 1 ) w = 1;
  if( w < -1 ) w = -1;
  float distrads = 2 * acos( w ); // Non-negative.
  float distdegrees = distrads * 180.0f / 3.141592654f; // Non-negative.
  if( distdegrees > 180.0 ) distdegrees -= 360.0;
  return fabsf( distdegrees );
}

static bool Near(
    CalVector const & p1, CalQuaternion const & q1, CalVector const & p2, CalQuaternion const & q2,
    double transTolerance,
    double rotTolerance
) {
  float distdegrees = DistanceDegrees( q1, q2 );
  if( distdegrees > rotTolerance ) return false;
  float dist = Distance( p1, p2 );
  if( dist > transTolerance ) return false;
  return true;
}


// Returns true if rounding took place and they were not exactly equal.
static bool roundTranslation(CalCoreKeyframe const * prevp, CalCoreKeyframe * p, double transTolerance) {
  CalCoreKeyframe * prev = const_cast<CalCoreKeyframe*>(prevp);
  assert(prev && p);

  // blend between the two keyframes
  CalVector translation = prev->translation;
  CalVector const ppos = p->translation;
  float dist = Distance( translation, ppos );

  // Identical returns false.
  if( dist == 0 ) return false;

  // Compare with tolerance.
  if( dist < transTolerance ) { // equal case handled above.
    p->translation = translation;
    return true;
  } else {
    return false;
  }
}

static bool keyframeEliminatable(
    const CalCoreKeyframe* prev, 
    const CalCoreKeyframe* p, 
    const CalCoreKeyframe* next,
    double transTolerance,
    double rotTolerance
) {
  assert( prev && p && next );
  float blendFactor = (p->time - prev->time) / (next->time - prev->time);

  // blend between the two keyframes
  CalVector translation = prev->translation;
  translation.blend( blendFactor, next->translation );
  CalQuaternion rotation = prev->rotation;
  rotation.blend( blendFactor, next->rotation );
  return Near(translation, rotation, p->translation, p->rotation, transTolerance, rotTolerance);
}



struct KeyLink {
  bool eliminated_;
  CalCoreKeyframe * keyframe_;
  KeyLink * next_;
};


unsigned int
KeyFrameSequenceLength( KeyLink * p, double transTolerance, double rotTolerance )
{
  CalVector translation = p->keyframe_->translation;
  CalQuaternion rotation = p->keyframe_->rotation;
  p = p->next_;
  unsigned int len = 1;
  while( p ) {
    CalVector const ppos = p->keyframe_->translation;
    CalQuaternion const pori = p->keyframe_->rotation;
    if( Near( translation, rotation, ppos, pori, transTolerance, rotTolerance ) ) {
      len++;
      p = p->next_;
    } else {
      break;
    }
  }
  return len;
}


CalCoreTrackPtr CalCoreTrack::compress(
    double translationTolerance,
    double rotationToleranceDegrees,
    CalCoreSkeleton* skelOrNull
) const {
  size_t numFrames = m_keyframes.size();
  if (!numFrames) {
      return CalCoreTrackPtr(new CalCoreTrack(coreBoneId));
  }

  // I want to iterate through the vector as a list, and remove elements easily.
  std::vector<KeyLink> keyLinkArray(numFrames);
  for (size_t i = 0; i < numFrames; i++) {
    KeyLink * kl = & keyLinkArray[ i ];
    kl->keyframe_ = m_keyframes[ i ];
    kl->next_ = ( i == numFrames - 1 ) ? NULL : & keyLinkArray[ i + 1 ];
    kl->eliminated_ = false;
  }

  // Iterate until quiescence.
  bool removedFrame = true;
  while( removedFrame ) {
    removedFrame = false;

    // Loop through the frames, starting with the second, ending with the second to last.
    // If the frame is approximately the same as the interpolated frame between its prev
    // and next frame, then eliminate it.
    KeyLink * prev = & keyLinkArray[ 0 ];
    while( true ) {
      KeyLink * p = prev->next_;
      if( !p || !p->next_ ) break;
      KeyLink * next = p->next_;
      if( keyframeEliminatable( prev->keyframe_, 
		  p->keyframe_, 
		  next->keyframe_,
		  translationTolerance, rotationToleranceDegrees) ) 
	  {
        p->eliminated_ = true;

        // Splice this eliminated keyframe out of the list.
        prev->next_ = next;

        // Pass over next frame, making it prev instead of making it p, since I don't
        // want to consider eliminating two key frames in a row because that can 
        // reduce a slow moving curve to a line no matter the arc
        // of the curve.
        prev = next;
        removedFrame = true;
      } else {
        prev = p;
      }
    }
  }

  // Now go through and round off translation values to the prev value if they are within
  // tolerance.  The reason we do this is so lossless compression algorithms will eliminate
  // redundancy.  There seems to be numerical jitter in the translation when there are rotations,
  // which will make those translation values not compress well.
  KeyLink * prev = & keyLinkArray[ 0 ];
  KeyLink * p = prev->next_;
  while( p ) {
    roundTranslation( prev->keyframe_, p->keyframe_, translationTolerance );
    prev = p;
    p = p->next_;
  }

  std::vector<CalCoreKeyframe*> output;

  // Rebuild the vector, freeing any of the eliminated keyframes.
  for( unsigned i = 0; i < numFrames; i++ ) {
    KeyLink * kl = & keyLinkArray[ i ];
    if( !kl->eliminated_ ) {
      output.push_back(new CalCoreKeyframe(*kl->keyframe_));
    }
  }

  CalCoreTrackPtr result(new CalCoreTrack(coreBoneId));
  for (KeyframeList::const_iterator i = output.begin(); i != output.end(); ++i) {
      result->addCoreKeyframe(*i);
  }

  // Update the flag saying whether the translation, which I have loaded, is actually required.
  // If translation is not required, I can't do any better than that so I leave it alone.
  if( skelOrNull && m_translationRequired ) {
    result->translationCompressibility( 
      &result->m_translationRequired, 
      &result->m_translationIsDynamic, 
      translationTolerance, CalLoader::keyframePosRangeSmall, skelOrNull );
  }

  return result;
}



void CalCoreTrack::translationCompressibility(
    bool * transRequiredResult,
    bool * transDynamicResult,
    float threshold,
    float highRangeThreshold,
    CalCoreSkeleton * skel
) const {
  * transRequiredResult = false;
  * transDynamicResult = false;
  int numFrames = m_keyframes.size();
  CalCoreBone * cb = skel->getCoreBone( coreBoneId );
  const CalVector & cbtrans = cb->getTranslation();
  CalVector trans0;
  float t2 = threshold * threshold;
  unsigned int i;
  for( i = 0; i < numFrames; i++ ) {
    CalCoreKeyframe * keyframe = m_keyframes[ i ];
    const CalVector & kftrans = keyframe->translation;
    if( i == 0 ) {
      trans0 = keyframe->translation;
    } else {
      float d2 = DistanceSquared( trans0, kftrans );
      if( d2 > t2 ) {
        * transDynamicResult = true;
      }
    }
    float d2 = DistanceSquared( cbtrans, kftrans );
    if( d2 > t2 ) {
      * transRequiredResult = true;
    }
  }
}

/*****************************************************************************/
/** Returns a specified state.
  *
  * This function returns the state (translation and rotation of the core bone)
  * for the specified time and duration.
  *
  * @param time The time in seconds at which the state should be returned.
  * @param translation A reference to the translation reference that will be
  *                    filled with the specified state.
  * @param rotation A reference to the rotation reference that will be filled
  *                 with the specified state.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreTrack::getState(float time, CalVector& translation, CalQuaternion& rotation)
{
  std::vector<CalCoreKeyframe*>::iterator iteratorCoreKeyframeBefore;
  std::vector<CalCoreKeyframe*>::iterator iteratorCoreKeyframeAfter;

  // get the keyframe after the requested time
  iteratorCoreKeyframeAfter = getUpperBound(time);

  // check if the time is after the last keyframe
  if(iteratorCoreKeyframeAfter == m_keyframes.end())
  {
    // return the last keyframe state
    --iteratorCoreKeyframeAfter;
    rotation = (*iteratorCoreKeyframeAfter)->rotation;
    translation = (*iteratorCoreKeyframeAfter)->translation;

    return true;
  }

  // check if the time is before the first keyframe
  if(iteratorCoreKeyframeAfter == m_keyframes.begin())
  {
    // return the first keyframe state
    rotation = (*iteratorCoreKeyframeAfter)->rotation;
    translation = (*iteratorCoreKeyframeAfter)->translation;

    return true;
  }

  // get the keyframe before the requested one
  iteratorCoreKeyframeBefore = iteratorCoreKeyframeAfter;
  --iteratorCoreKeyframeBefore;

  // get the two keyframe pointers
  CalCoreKeyframe *pCoreKeyframeBefore;
  pCoreKeyframeBefore = *iteratorCoreKeyframeBefore;
  CalCoreKeyframe *pCoreKeyframeAfter;
  pCoreKeyframeAfter = *iteratorCoreKeyframeAfter;

  // calculate the blending factor between the two keyframe states
  float blendFactor;
  blendFactor = (time - pCoreKeyframeBefore->time) / (pCoreKeyframeAfter->time - pCoreKeyframeBefore->time);

  // blend between the two keyframes
  translation = pCoreKeyframeBefore->translation;
  translation.blend(blendFactor, pCoreKeyframeAfter->translation);

  rotation = pCoreKeyframeBefore->rotation;
  rotation.blend(blendFactor, pCoreKeyframeAfter->rotation);

  return true;
}

std::vector<CalCoreKeyframe*>::iterator CalCoreTrack::getUpperBound(float time)
{

  int lowerBound = 0;
  int upperBound = m_keyframes.size()-1;

  while(lowerBound<upperBound-1)
  {
	  int middle = (lowerBound+upperBound)/2;

	  if(time >= m_keyframes[middle]->time)
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

int CalCoreTrack::getCoreKeyframeCount()
{
  return m_keyframes.size();
}


CalCoreKeyframe* CalCoreTrack::getCoreKeyframe(int idx)
{
  return m_keyframes[idx];
}
