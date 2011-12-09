//****************************************************************************//
// mesh.cpp                                                                   //
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

#include "cal3d/mesh.h"
#include "cal3d/error.h"
#include "cal3d/coremesh.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/submesh.h"

CalMesh::SubmeshVector fromCoreSubmeshes(const CalCoreMesh::CalCoreSubmeshVector& coreSubmeshes) {
    CalMesh::SubmeshVector rv;
    rv.reserve(coreSubmeshes.size());
    for (size_t i = 0; i < coreSubmeshes.size(); ++i) {
        rv.push_back(CalSubmeshPtr(new CalSubmesh(coreSubmeshes[i])));
    }
    return rv;
}

CalMesh::CalMesh(const CalCoreMeshPtr& pCoreMesh, const std::vector<int>& backfacingBones)
    : coreMesh(pCoreMesh)
    , submeshes(fromCoreSubmeshes(pCoreMesh->submeshes))
    , backfacingBones(backfacingBones)
{}
