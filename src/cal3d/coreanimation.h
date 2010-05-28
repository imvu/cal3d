//****************************************************************************//
// coreanimation.h                                                            //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <list>
#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"

typedef boost::shared_ptr<class CalCoreTrack> CalCoreTrackPtr;
class CalCoreSkeleton;

class CAL3D_API CalCoreAnimation : public Cal::Object
{
public:
  size_t sizeInBytes() const;
  CalCoreTrackPtr getCoreTrack(int coreBoneId);

  float duration;
  typedef std::vector<CalCoreTrackPtr> TrackList;
  TrackList tracks;
};
