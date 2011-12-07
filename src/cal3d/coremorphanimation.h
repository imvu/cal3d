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
#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"
#include "cal3d/coremorphtrack.h"

class CalCoreMorphTrack;

class CAL3D_API CalCoreMorphAnimation {
public:
    float duration;
    std::vector<CalCoreMorphTrack> tracks;

    CalCoreMorphAnimation()
        : duration(0.0f)
    {}

    CalCoreMorphTrack* getCoreTrack(const std::string& trackId);

    void removeZeroScaleTracks();
    void scale(float factor);

    size_t sizeInBytes() const;
};
CAL3D_PTR(CalCoreMorphAnimation);
