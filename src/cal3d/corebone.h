//****************************************************************************//
// corebone.h                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <list>
#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"
#include "cal3d/transform.h"

// these values are important - we use c-style casts to them :x
enum CalLightType {
    LIGHT_TYPE_NONE,
    LIGHT_TYPE_OMNI,
    LIGHT_TYPE_DIRECTIONAL,
    LIGHT_TYPE_TARGET,
    LIGHT_TYPE_AMBIENT
};

class CAL3D_API CalCoreBone {
public:
    int parentId;
    const std::string name;

    CalVector lightColor;
    CalLightType lightType;

    cal3d::RotateTranslate relativeTransform;
    cal3d::RotateTranslate boneSpaceTransform;

    CalCoreBone(const std::string& name, int parentId = -1);
    void scale(float factor);

    bool hasLightingData() const {
        return lightType != LIGHT_TYPE_NONE;
    }
};
typedef boost::shared_ptr<CalCoreBone> CalCoreBonePtr;
