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
#include "cal3d/coremorphkeyframe.h"

class CalCoreMorphKeyframe;

class CAL3D_API CalCoreMorphTrack {
public:
    std::string morphName;
    std::vector<CalCoreMorphKeyframe> keyframes;

    size_t size() const;

    float getState(float time);

    unsigned getCoreMorphKeyframeCount() {
        return keyframes.size();
    }
  
    void addCoreMorphKeyframe(CalCoreMorphKeyframe pCoreKeyframe);
    void scale(float factor);

private:
    std::vector<CalCoreMorphKeyframe>::iterator getUpperBound(float time);
};
