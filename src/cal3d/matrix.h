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
    // column-major
    CalVector cx;
    CalVector cy;
    CalVector cz;

    inline CalMatrix()
        : cx(1, 0, 0)
        , cy(0, 1, 0)
        , cz(0, 0, 1)
    {}

    inline CalMatrix(const CalVector& cx, const CalVector& cy, const CalVector& cz)
        : cx(cx)
        , cy(cy)
        , cz(cz)
    {}

    explicit CalMatrix(const CalQuaternion& q);
};

inline bool operator==(const CalMatrix& lhs, const CalMatrix& rhs) {
    const float* lp = &lhs.cx.x;
    const float* rp = &rhs.cx.x;
    for (size_t i = 0; i < 9; ++i) {
        if (lp[i] != rp[i]) {
            return false;
        }
    }
    return true;
}

inline CalVector operator*(const CalMatrix& m, const CalVector& v) {
    return CalVector(
        m.cx.x * v.x + m.cy.x * v.y + m.cz.x * v.z,
        m.cx.y * v.x + m.cy.y * v.y + m.cz.y * v.z,
        m.cx.z * v.x + m.cy.z * v.y + m.cz.z * v.z);
}

inline CalMatrix operator*(const CalMatrix& outer, const CalMatrix& inner) {
    float ndxdx = outer.cx.x * inner.cx.x + outer.cy.x * inner.cx.y + outer.cz.x * inner.cx.z;
    float ndydx = outer.cx.y * inner.cx.x + outer.cy.y * inner.cx.y + outer.cz.y * inner.cx.z;
    float ndzdx = outer.cx.z * inner.cx.x + outer.cy.z * inner.cx.y + outer.cz.z * inner.cx.z;

    float ndxdy = outer.cx.x * inner.cy.x + outer.cy.x * inner.cy.y + outer.cz.x * inner.cy.z;
    float ndydy = outer.cx.y * inner.cy.x + outer.cy.y * inner.cy.y + outer.cz.y * inner.cy.z;
    float ndzdy = outer.cx.z * inner.cy.x + outer.cy.z * inner.cy.y + outer.cz.z * inner.cy.z;

    float ndxdz = outer.cx.x * inner.cz.x + outer.cy.x * inner.cz.y + outer.cz.x * inner.cz.z;
    float ndydz = outer.cx.y * inner.cz.x + outer.cy.y * inner.cz.y + outer.cz.y * inner.cz.z;
    float ndzdz = outer.cx.z * inner.cz.x + outer.cy.z * inner.cz.y + outer.cz.z * inner.cz.z;

    return CalMatrix(
        CalVector(
            ndxdx,
            ndydx,
            ndzdx),
        CalVector(
            ndxdy,
            ndydy,
            ndzdy),
        CalVector(
            ndxdz,
            ndydz,
            ndzdz));
}
