//****************************************************************************//
// model.h                                                                    //
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
#include "cal3d/vector.h"

class CalCoreSkeleton;
class CalCoreMesh;
class CalSkeleton;
class CalMixer;
class CalMesh;

class CAL3D_API CalModel {
public:
    CalMesh* attachMesh(const boost::shared_ptr<CalCoreMesh>& pCoreMesh);
    CalMesh* getMesh(const boost::shared_ptr<CalCoreMesh>& pCoreMesh);

    std::vector< boost::shared_ptr<CalMesh> > meshes;
};
