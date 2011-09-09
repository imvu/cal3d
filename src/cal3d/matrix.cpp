//****************************************************************************//
// matrix.cpp                                                                 //
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

#include "cal3d/matrix.h"
#include "cal3d/quaternion.h"
#include "cal3d/vector.h"

CalMatrix::CalMatrix(const CalQuaternion& q) {
    float xx2 = q.x * q.x * 2;
    float yy2 = q.y * q.y * 2;
    float zz2 = q.z * q.z * 2;
    float xy2 = q.x * q.y * 2;
    float zw2 = q.z * q.w * 2;
    float xz2 = q.x * q.z * 2;
    float yw2 = q.y * q.w * 2;
    float yz2 = q.y * q.z * 2;
    float xw2 = q.x * q.w * 2;
    dxdx = 1 - yy2 - zz2;
    dxdy =  xy2 + zw2;
    dxdz =  xz2 - yw2;
    dydx =  xy2 - zw2;
    dydy = 1 - xx2 - zz2;
    dydz =  yz2 + xw2;
    dzdx =  xz2 + yw2;
    dzdy =  yz2 - xw2;
    dzdz = 1 - xx2 - yy2;
}
