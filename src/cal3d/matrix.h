//****************************************************************************//
// matrix.h                                                                   //
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
#include "cal3d/vector4.h"

class CalQuaternion;

class CAL3D_API CalMatrix {
public:
    float dxdx, dydx, dzdx;
    float dxdy, dydy, dzdy;
    float dxdz, dydz, dzdz;

    inline CalMatrix()
        : dxdx(0.0), dydx(0.0), dzdx(0.0)
        , dxdy(0.0), dydy(0.0), dzdy(0.0)
        , dxdz(0.0), dydz(0.0), dzdz(0.0)
    {}

    inline CalMatrix(float xx, float yx, float zx, float xy, float yy, float zy, float xz, float yz, float zz)
        : dxdx(xx), dydx(yx), dzdx(zx)
        , dxdy(xy), dydy(yy), dzdy(zy)
        , dxdz(xz), dydz(yz), dzdz(zz)
    {}

    explicit CalMatrix(const CalQuaternion& q);
};

inline CalMatrix operator*(const CalMatrix& outer, const CalMatrix& inner) {
    float ndxdx = outer.dxdx * inner.dxdx + outer.dxdy * inner.dydx + outer.dxdz * inner.dzdx;
    float ndydx = outer.dydx * inner.dxdx + outer.dydy * inner.dydx + outer.dydz * inner.dzdx;
    float ndzdx = outer.dzdx * inner.dxdx + outer.dzdy * inner.dydx + outer.dzdz * inner.dzdx;

    float ndxdy = outer.dxdx * inner.dxdy + outer.dxdy * inner.dydy + outer.dxdz * inner.dzdy;
    float ndydy = outer.dydx * inner.dxdy + outer.dydy * inner.dydy + outer.dydz * inner.dzdy;
    float ndzdy = outer.dzdx * inner.dxdy + outer.dzdy * inner.dydy + outer.dzdz * inner.dzdy;

    float ndxdz = outer.dxdx * inner.dxdz + outer.dxdy * inner.dydz + outer.dxdz * inner.dzdz;
    float ndydz = outer.dydx * inner.dxdz + outer.dydy * inner.dydz + outer.dydz * inner.dzdz;
    float ndzdz = outer.dzdx * inner.dxdz + outer.dzdy * inner.dydz + outer.dzdz * inner.dzdz;

    return CalMatrix(
        ndxdx,
        ndydx,
        ndzdx,
        ndxdy,
        ndydy,
        ndzdy,
        ndxdz,
        ndydz,
        ndzdz);
}
