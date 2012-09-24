//****************************************************************************//
// vector.cpp                                                                 //
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
#include <algorithm>
#include "cal3d/matrix.h"
#include "cal3d/vector4.h"
#include "cal3d/quaternion.h"
#include "cal3d/transform.h"

CalVector4 CalVector::asCalVector4() const {
    return CalVector4(x, y, z, 0.0f);
}

namespace cal3d {
    void applyZupToYup(CalVector4 &vec4) {    
        std::swap(vec4.y, vec4.z);
        vec4.z = -vec4.z;
    }

    void applyZupToYup(CalPoint4 &point4) {    
        std::swap(point4.y, point4.z);
        point4.z = -point4.z;
    }

    void applyCoordinateTransform(CalVector4 &vec4, CalQuaternion &xfm) {
        vec4 = xfm * vec4;
    }

    void applyCoordinateTransform(CalPoint4 &point4, CalQuaternion &xfm) {
        point4 = xfm * point4;
    }
}

