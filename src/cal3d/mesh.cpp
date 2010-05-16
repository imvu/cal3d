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

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "cal3d/mesh.h"
#include "cal3d/error.h"
#include "cal3d/coremesh.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/submesh.h"
#include "cal3d/coremodel.h"
#include "cal3d/model.h"

 /*****************************************************************************/
/** Constructs the mesh instance.
  *
  * This function is the default constructor of the mesh instance.
  *****************************************************************************/

CalMesh::CalMesh(CalModel* pModel, const boost::shared_ptr<CalCoreMesh>& pCoreMesh)
  : m_pModel(pModel)
  , m_pCoreMesh(pCoreMesh)
{
  assert(pModel);
  assert(pCoreMesh);

  // clone the mesh structure of the core mesh
  CalCoreMesh::CalCoreSubmeshVector& vectorCoreSubmesh = pCoreMesh->getVectorCoreSubmesh();

  int submeshCount = vectorCoreSubmesh.size();
  m_vectorSubmesh.reserve(submeshCount);
  for(int submeshId = 0; submeshId < submeshCount; ++submeshId)
  {
    m_vectorSubmesh.push_back(new CalSubmesh(vectorCoreSubmesh[submeshId]));
  }
}

CalMesh::~CalMesh()
{
  std::vector<CalSubmesh *>::iterator iteratorSubmesh;
  for(iteratorSubmesh = m_vectorSubmesh.begin(); iteratorSubmesh != m_vectorSubmesh.end(); ++iteratorSubmesh)
  {
    delete (*iteratorSubmesh);
  }
}

 /*****************************************************************************/
/** Provides access to a submesh.
  *
  * This function returns the submesh with the given ID.
  *
  * @param id The ID of the submesh that should be returned.
  *
  * @return One of the following values:
  *         \li a pointer to the submesh
  *         \li \b 0 if an error happend
  *****************************************************************************/

CalSubmesh *CalMesh::getSubmesh(int id)
{
  if((id < 0) || (id >= (int)m_vectorSubmesh.size()))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return 0;
  }

  return m_vectorSubmesh[id];
}

 /*****************************************************************************/
/** Returns the number of submeshes.
  *
  * This function returns the number of submeshes in the mesh instance.
  *
  * @return The number of submeshes.
  *****************************************************************************/

int CalMesh::getSubmeshCount()
{
  return m_vectorSubmesh.size();
}

 /*****************************************************************************/
/** Returns the submesh vector.
  *
  * This function returns the vector that contains all submeshes of the mesh
  * instance.
  *
  * @return A reference to the submesh vector.
  *****************************************************************************/

std::vector<CalSubmesh *>& CalMesh::getVectorSubmesh()
{
  return m_vectorSubmesh;
}

 /*****************************************************************************/
/** Sets the LOD level.
  *
  * This function sets the LOD level of the mesh instance.
  *
  * @param lodLevel The LOD level in the range [0.0, 1.0].
  *****************************************************************************/

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

 /*****************************************************************************/
/** Sets the material set.
  *
  * This function sets the material set of the mesh instance.
  *
  * @param setId The ID of the material set.
  *****************************************************************************/

void CalMesh::setMaterialSet(int setId) {
  for(int submeshId = 0; submeshId < (int)m_vectorSubmesh.size(); ++submeshId) {
    int coreMaterialThreadId = m_vectorSubmesh[submeshId]->getCoreSubmesh()->getCoreMaterialThreadId();

    boost::shared_ptr<CalCoreMaterial> material = m_pModel->getCoreModel()->getCoreMaterialId(coreMaterialThreadId, setId);

    m_vectorSubmesh[submeshId]->setMaterial(material);
  }
}
