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

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "cal3d/vector.h"
#include "cal3d/quaternion.h"

CalVector operator*(const CalQuaternion& q, const CalVector& v) {
    CalQuaternion temp(-q.x, -q.y, -q.z, q.w);
    temp *= v;
    temp *= q;

    return CalVector(
        temp.x,
        temp.y,
        temp.z);
}
