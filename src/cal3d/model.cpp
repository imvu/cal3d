//****************************************************************************//
// model.cpp                                                                  //
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

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "cal3d/error.h"
#include "cal3d/model.h"
#include "cal3d/skeleton.h"
#include "cal3d/bone.h"
#include "cal3d/renderer.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/coremesh.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/mesh.h"
#include "cal3d/physique.h"

CalModel::CalModel(const boost::shared_ptr<CalCoreSkeleton>& skeleton)
    : skeleton(new CalSkeleton(skeleton)) {
}

CalMesh* CalModel::attachMesh(const boost::shared_ptr<CalCoreMesh>& pCoreMesh) {
    // check if the mesh is already attached
    for (size_t meshId = 0; meshId < meshes.size(); ++meshId) {
        // check if we found the matching mesh
        if (meshes[meshId]->coreMesh == pCoreMesh) {
            return meshes[meshId].get();
        }
    }

    boost::shared_ptr<CalMesh> mesh(new CalMesh(pCoreMesh));
    meshes.push_back(mesh);
    return mesh.get();
}

CalMesh* CalModel::getMesh(const boost::shared_ptr<CalCoreMesh>& pCoreMesh) {

    // search the mesh
    for (size_t meshId = 0; meshId < meshes.size(); ++meshId) {
        // check if we found the matching mesh
        if (meshes[meshId]->coreMesh == pCoreMesh) {
            return meshes[meshId].get();
        }
    }

    return 0;
}

