//****************************************************************************//
// coremesh.h                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"

CAL3D_PTR(CalCoreSkeleton);
CAL3D_PTR(CalCoreSubmesh);
class CalQuaternion;

class CAL3D_API CalCoreMesh {
public:
    typedef std::vector<CalCoreSubmeshPtr> CalCoreSubmeshVector;

    size_t sizeInBytes() const;
    bool addAsMorphTarget(CalCoreMesh* pCoreMesh, std::string const& morphTargetName);
    
    void replaceMeshWithMorphTarget(const std::string& morphTargetName);

    void scale(float factor);
    void fixup(const CalCoreSkeletonPtr& skeleton);

    CalCoreSubmeshVector submeshes;
};
CAL3D_PTR(CalCoreMesh);
