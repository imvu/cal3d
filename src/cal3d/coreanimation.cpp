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

size_t sizeInBytes(const CalCoreTrackPtr& t) {
    return sizeof(CalCoreTrackPtr) + t->sizeInBytes();
}

size_t CalCoreAnimation::sizeInBytes() const {
  return sizeof(*this) + ::sizeInBytes(tracks);
}


CalCoreTrackPtr CalCoreAnimation::getCoreTrack(int coreBoneId)
{
  // loop through all core track
  TrackList::iterator iteratorCoreTrack;
  for(iteratorCoreTrack = tracks.begin(); iteratorCoreTrack != tracks.end(); ++iteratorCoreTrack)
  {
    CalCoreTrackPtr& pCoreTrack = *iteratorCoreTrack;
    if (pCoreTrack->coreBoneId == coreBoneId) {
        return pCoreTrack;
    }
  }

  // no match found
  return CalCoreTrackPtr();
}
