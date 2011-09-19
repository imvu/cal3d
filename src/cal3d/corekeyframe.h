//****************************************************************************//
// corekeyframe.h                                                             //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"
#include "cal3d/transform.h"

extern CalVector InvalidTranslation;

class CalCoreKeyframe {
public:
    CalCoreKeyframe() {
        time = 0.0f;
    }

    CalCoreKeyframe(float t, const CalVector& tr, const CalQuaternion& ro)
        : time(t)
        , transform(ro, tr)
    {}

    void scale(float factor) {
        // don't scale the 'invalid translation' sentinel
        if (!exactlyEqual(transform.translation, InvalidTranslation)) {
            transform.translation *= factor;
        }
    }

    float time;
    cal3d::RotateTranslate transform;
};

inline bool operator<(const CalCoreKeyframe& lhs, const CalCoreKeyframe& rhs) {
    return lhs.time < rhs.time;
}

inline bool operator==(const CalCoreKeyframe& lhs, const CalCoreKeyframe& rhs) {
    return cal3d::close(lhs.time, rhs.time) && lhs.transform == rhs.transform;
}
