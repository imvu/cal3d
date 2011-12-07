//****************************************************************************//
// mesh.h                                                                     //
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

CAL3D_PTR(CalCoreMesh);
CAL3D_PTR(CalSubmesh);

class CAL3D_API CalMesh {
public:
    typedef std::vector<CalSubmeshPtr> SubmeshVector;

    CalMesh(const CalCoreMeshPtr& pCoreMesh);

    const CalCoreMeshPtr coreMesh;
    const SubmeshVector submeshes;
};
CAL3D_PTR(CalMesh);
