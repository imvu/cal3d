//****************************************************************************//
// coreanimatedMorph.h                                                            //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <list>
#include "cal3d/global.h"
#include "cal3d/coremorphtrack.h"

class CalCoreMorphTrack;

class CAL3D_API CalCoreAnimatedMorph
{
public:
  CalCoreAnimatedMorph();
  ~CalCoreAnimatedMorph();

  bool addCoreTrack(CalCoreMorphTrack *pCoreTrack);
  bool create();
  void destroy();
  CalCoreMorphTrack *getCoreTrack(std::string const & trackId);
  float getDuration() const;
  std::list<CalCoreMorphTrack>& getListCoreTrack();
  void setDuration(float duration);
  void scale(float factor);
  void removeZeroScaleTracks();
  static int getNumCoreAnimatedMorphs();

private:
  float m_duration;
  std::list<CalCoreMorphTrack> m_listCoreTrack;
  std::list<CalCoreMorphTrack*> m_tracksToDelete;
};
