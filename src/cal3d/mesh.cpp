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
#include "cal3d/coremodel.h"
#include "cal3d/model.h"

CalMesh::CalMesh(const boost::shared_ptr<CalCoreMesh>& pCoreMesh)
: coreMesh(pCoreMesh)
{
  assert(pCoreMesh);

  // clone the mesh structure of the core mesh
  CalCoreMesh::CalCoreSubmeshVector& vectorCoreSubmesh = pCoreMesh->getVectorCoreSubmesh();

  SubmeshVector& m_vectorSubmesh = const_cast<SubmeshVector&>(submeshes); // Oh for a 'readonly' keyword like C#

  int submeshCount = vectorCoreSubmesh.size();
  m_vectorSubmesh.reserve(submeshCount);
  for(int submeshId = 0; submeshId < submeshCount; ++submeshId)
  {
    m_vectorSubmesh.push_back(boost::shared_ptr<CalSubmesh>(new CalSubmesh(vectorCoreSubmesh[submeshId])));
  }
}


void CalMesh::setLodLevel(float lodLevel)
{
  // change lod level of every submesh
  for(size_t submeshId = 0; submeshId < submeshes.size(); ++submeshId)
  {
    // set the lod level in the submesh
    submeshes[submeshId]->setLodLevel(lodLevel);
  }
}

void CalMesh::setMaterialSet(CalCoreModel* model, int setId) {
    for (size_t submeshId = 0; submeshId < submeshes.size(); ++submeshId) {
        CalSubmesh* submesh = submeshes[submeshId].get();
        submesh->setMaterial(
            model->getCoreMaterialId(
                submesh->getCoreSubmesh()->getCoreMaterialThreadId(),
                setId));
    }
}
