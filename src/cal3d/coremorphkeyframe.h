//****************************************************************************//
// coreMorphKeyframe.h                                                             //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

class CalCoreMorphKeyframe {
public:
    float time;
    float weight;

    CalCoreMorphKeyframe() {
        time = 0;
        weight = 0;
    }

    CalCoreMorphKeyframe(float t, float w)
        : time(t)
        , weight(w)
    {}
};

inline bool operator<(CalCoreMorphKeyframe lhs, CalCoreMorphKeyframe rhs) {
    return lhs.time < rhs.time;
}
