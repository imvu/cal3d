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

#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"

typedef boost::shared_ptr<class CalCoreSubmesh> CalCoreSubmeshPtr;

class CAL3D_API CalCoreMesh {
public:
    typedef std::vector<CalCoreSubmeshPtr> CalCoreSubmeshVector;

    size_t sizeInBytes() const;
    size_t addCoreSubmesh(const CalCoreSubmeshPtr& pCoreSubmesh);
    size_t addAsMorphTarget(CalCoreMesh* pCoreMesh, std::string const& morphTargetName);
    
    void scale(float factor);

    CalCoreSubmeshVector submeshes;
};
typedef boost::shared_ptr<CalCoreMesh> CalCoreMeshPtr;
