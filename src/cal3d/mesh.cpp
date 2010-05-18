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
: m_pCoreMesh(pCoreMesh)
{
  assert(pCoreMesh);

  // clone the mesh structure of the core mesh
  CalCoreMesh::CalCoreSubmeshVector& vectorCoreSubmesh = pCoreMesh->getVectorCoreSubmesh();

  int submeshCount = vectorCoreSubmesh.size();
  m_vectorSubmesh.reserve(submeshCount);
  for(int submeshId = 0; submeshId < submeshCount; ++submeshId)
  {
    m_vectorSubmesh.push_back(boost::shared_ptr<CalSubmesh>(new CalSubmesh(vectorCoreSubmesh[submeshId])));
  }
}


CalSubmesh *CalMesh::getSubmesh(int id)
{
  return m_vectorSubmesh[id].get();
}

int CalMesh::getSubmeshCount()
{
  return m_vectorSubmesh.size();
}

CalMesh::SubmeshVector& CalMesh::getVectorSubmesh()
{
  return m_vectorSubmesh;
}

void CalMesh::setLodLevel(float lodLevel)
{
  // change lod level of every submesh
  int submeshId;
  for(submeshId = 0; submeshId < (int)m_vectorSubmesh.size(); ++submeshId)
  {
    // set the lod level in the submesh
    m_vectorSubmesh[submeshId]->setLodLevel(lodLevel);
  }
}

void CalMesh::setMaterialSet(CalCoreModel* model, int setId) {
    for (size_t submeshId = 0; submeshId < m_vectorSubmesh.size(); ++submeshId) {
        CalSubmesh* submesh = m_vectorSubmesh[submeshId].get();
        submesh->setMaterial(
            model->getCoreMaterialId(
                submesh->getCoreSubmesh()->getCoreMaterialThreadId(),
                setId));
    }
}
