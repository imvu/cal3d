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
    cx.x = 1 - yy2 - zz2;
    cy.x = xy2 + zw2;
    cz.x = xz2 - yw2;
    cx.y = xy2 - zw2;
    cy.y = 1 - xx2 - zz2;
    cz.y = yz2 + xw2;
    cx.z = xz2 + yw2;
    cy.z = yz2 - xw2;
    cz.z = 1 - xx2 - yy2;
}
