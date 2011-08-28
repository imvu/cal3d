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
        , dxdz(0.0), dydz(0.0), dzdz(0.0) {
    }

    explicit CalMatrix(const CalQuaternion& q);

    void operator=(const CalQuaternion& q);

    inline void operator *= (const CalMatrix& m) {
        float ndxdx = m.dxdx * dxdx + m.dxdy * dydx + m.dxdz * dzdx;
        float ndydx = m.dydx * dxdx + m.dydy * dydx + m.dydz * dzdx;
        float ndzdx = m.dzdx * dxdx + m.dzdy * dydx + m.dzdz * dzdx;

        float ndxdy = m.dxdx * dxdy + m.dxdy * dydy + m.dxdz * dzdy;
        float ndydy = m.dydx * dxdy + m.dydy * dydy + m.dydz * dzdy;
        float ndzdy = m.dzdx * dxdy + m.dzdy * dydy + m.dzdz * dzdy;

        float ndxdz = m.dxdx * dxdz + m.dxdy * dydz + m.dxdz * dzdz;
        float ndydz = m.dydx * dxdz + m.dydy * dydz + m.dydz * dzdz;
        float ndzdz = m.dzdx * dxdz + m.dzdy * dydz + m.dzdz * dzdz;

        dxdx = ndxdx;
        dydx = ndydx;
        dzdx = ndzdx;
        dxdy = ndxdy;
        dydy = ndydy;
        dzdy = ndzdy;
        dxdz = ndxdz;
        dydz = ndydz;
        dzdz = ndzdz;
    }

    inline float det() {
        return dxdx * (dydy * dzdz - dydz * dzdy)
               - dxdy * (dydx * dzdz - dzdx * dydz)
               + dxdz * (dydx * dzdy - dzdx * dydy);
    }
};

inline void extractRows(
    const CalMatrix& m,
    const CalVector& translation,
    CalVector4& rowx,
    CalVector4& rowy,
    CalVector4& rowz
) {
    rowx.set(m.dxdx, m.dxdy, m.dxdz, translation.x);
    rowy.set(m.dydx, m.dydy, m.dydz, translation.y);
    rowz.set(m.dzdx, m.dzdy, m.dzdz, translation.z);
}
