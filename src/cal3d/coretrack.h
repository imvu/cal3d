//****************************************************************************//
// coretrack.h                                                                //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <boost/shared_ptr.hpp>
#include "cal3d/corekeyframe.h"
#include "cal3d/global.h"
#include "cal3d/vector.h"
#include "cal3d/quaternion.h"

class CalCoreBone;
class CalCoreSkeleton;
typedef boost::shared_ptr<class CalCoreTrack> CalCoreTrackPtr;

class CAL3D_API CalCoreTrack : public Cal::Object {
public:
  typedef std::vector<CalCoreKeyframe> KeyframeList;

  const int coreBoneId; 
  const KeyframeList keyframes;

  CalCoreTrack(int coreBoneId, const KeyframeList& keyframes);

  size_t sizeInBytes() const;

  void getState(float time, CalVector& translation, CalQuaternion& rotation) const;

  bool getTranslationRequired() { return m_translationRequired; }
  void setTranslationRequired( bool p ) { m_translationRequired = p; }
  bool getTranslationIsDynamic() { return m_translationIsDynamic; }
  void setTranslationIsDynamic( bool p ) { m_translationIsDynamic = p; }

  CalCoreTrackPtr compress(double translationTolerance, double rotationToleranceDegrees, CalCoreSkeleton* skelOrNull) const;
  void translationCompressibility(
    bool* transRequiredResult, bool* transDynamicResult,
    float threshold, float highRangeThreshold, CalCoreSkeleton* skel
  ) const;

private:
  KeyframeList::const_iterator getUpperBound(float time) const;

  bool m_translationRequired;
  bool m_translationIsDynamic;
};
typedef boost::shared_ptr<CalCoreTrack> CalCoreTrackPtr;
