//****************************************************************************//
// coreMorphTrack.h                                                                //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"
#include "cal3d/vector.h"
#include "cal3d/quaternion.h"
#include "cal3d/coremorphkeyframe.h"

class CalCoreMorphKeyframe;

class CAL3D_API CalCoreMorphTrack
{
protected:
  std::string m_morphName;

  /// List of keyframes, always sorted by time.
  std::vector<CalCoreMorphKeyframe> m_keyframes;
  std::vector<CalCoreMorphKeyframe*> m_keyframesToDelete;

public:
  CalCoreMorphTrack();
  virtual ~CalCoreMorphTrack();

  bool create();
  void destroy();

  size_t size() const;

  float getState(float time);

  std::string getMorphNameTiXmlOnly() { return m_morphName; } // slow function to satisfy the Xml Bindings interface
  const std::string& getMorphName() { return m_morphName; }
  void setMorphName(std::string name);
  
  int getCoreMorphKeyframeCount();
  void reserve(int);
  
  CalCoreMorphKeyframe* getCoreMorphKeyframe(int idx);

  bool addCoreMorphKeyframe(CalCoreMorphKeyframe *pCoreKeyframe);

  std::vector<CalCoreMorphKeyframe> & getVectorCoreMorphKeyframes();
  void scale(float factor);

private:
  std::vector<CalCoreMorphKeyframe>::iterator getUpperBound(float time);
};
