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
#include "cal3d/matrix.h"
#include "cal3d/quaternion.h"

void CalVector::operator*=(const CalQuaternion& q)
{
  CalQuaternion temp(-q.x, -q.y, -q.z, q.w);
  temp *= *this;
  temp *= q;

  x = temp.x;
  y = temp.y;
  z = temp.z;
}

void CalVector::operator*=(const CalMatrix &m)
{
  float ox = x;
  float oy = y;
  float oz = z;
  x = m.dxdx*ox + m.dxdy*oy + m.dxdz*oz;
  y = m.dydx*ox + m.dydy*oy + m.dydz*oz;
  z = m.dzdx*ox + m.dzdy*oy + m.dzdz*oz;
}  
