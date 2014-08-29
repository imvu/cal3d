//****************************************************************************//
// quaternion.cpp                                                             //
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

#include "cal3d/quaternion.h"
#include "cal3d/vector.h"

CalQuaternion::CalQuaternion(CalMatrix m) {
    float t, s;
    // Computing the trace without extra 1.0f for additional stability:
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/ethan.htm
    t = m.cx.x + m.cy.y + m.cz.z;
    if (t > 0.00000001f) {
        s = 2.0f * sqrt(t + 1.0f);
        x = (m.cz.y - m.cy.z) / s;
        y = (m.cx.z - m.cz.x) / s;
        z = (m.cy.x - m.cx.y) / s;
        w = -(0.25f * s);
    } else if (m.cx.x > m.cy.y && m.cx.x > m.cz.z) {
        s = 2.0f * sqrt(1.0f + m.cx.x - m.cy.y - m.cz.z);
        x = 0.25f * s;
        y = (m.cy.x + m.cx.y) / s;
        z = (m.cx.z + m.cz.x) / s;
        w = -(m.cz.y - m.cy.z) / s;
    } else if (m.cy.y > m.cz.z) {
        s = 2.0f * sqrt(1.0f + m.cy.y - m.cx.x - m.cz.z);
        x = (m.cy.x + m.cx.y) / s;
        y = 0.25f * s;
        z = (m.cz.y + m.cy.z) / s;
        w = -(m.cx.z - m.cz.x) / s;
    } else {
        s = 2.0f * sqrt(1.0f + m.cz.z - m.cx.x - m.cy.y);
        x = (m.cx.z + m.cz.x) / s;
        y = (m.cz.y + m.cy.z) / s;
        z = 0.25f * s;
        w = -(m.cy.x - m.cx.y) / s;
    }
}
