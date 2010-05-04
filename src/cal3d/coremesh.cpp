//****************************************************************************//
// coremesh.cpp                                                               //
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

#include "cal3d/coremesh.h"
#include "cal3d/error.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/coresubmorphtarget.h"

size_t CalCoreMesh::size() const {
    unsigned int r = sizeof(CalCoreMesh);
    std::vector<CalCoreSubmesh *>::const_iterator iter1;
    for (iter1 = m_vectorCoreSubmesh.begin(); iter1 != m_vectorCoreSubmesh.end(); ++iter1) {
        r += (*iter1)->size();
    }
    return r;
}

CalCoreMesh::~CalCoreMesh() {
  std::vector<CalCoreSubmesh *>::iterator iteratorCoreSubmesh;
  for(iteratorCoreSubmesh = m_vectorCoreSubmesh.begin(); iteratorCoreSubmesh != m_vectorCoreSubmesh.end(); ++iteratorCoreSubmesh)
  {
    delete (*iteratorCoreSubmesh);
  }
}

int CalCoreMesh::addCoreSubmesh(CalCoreSubmesh *pCoreSubmesh) {
  int submeshId = m_vectorCoreSubmesh.size();
  m_vectorCoreSubmesh.push_back(pCoreSubmesh);
  return submeshId;
}

CalCoreSubmesh *CalCoreMesh::getCoreSubmesh(int id) {
  if((id < 0) || (id >= (int)m_vectorCoreSubmesh.size()))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return 0;
  }

  return m_vectorCoreSubmesh[id];
}

int CalCoreMesh::getCoreSubmeshCount() {
  return m_vectorCoreSubmesh.size();
}

std::vector<CalCoreSubmesh *>& CalCoreMesh::getVectorCoreSubmesh() {
  return m_vectorCoreSubmesh;
}

int CalCoreMesh::addAsMorphTarget(CalCoreMesh *pCoreMesh, std::string const & morphTargetName) {
  //Check if the numbers of vertices allow a blending
  std::vector<CalCoreSubmesh *>& otherVectorCoreSubmesh = pCoreMesh->getVectorCoreSubmesh();
  if (m_vectorCoreSubmesh.size() != otherVectorCoreSubmesh.size())
  {
    CalError::setLastError(CalError::INTERNAL, __FILE__, __LINE__, "This mesh has children with different numbers of materials");
    return -1;
  }
  if (m_vectorCoreSubmesh.size() == 0)
  {
    CalError::setLastError(CalError::INTERNAL, __FILE__, __LINE__, "Mesh has no submeshes");
    return -1;
  }
  std::vector<CalCoreSubmesh *>::iterator iteratorCoreSubmesh = m_vectorCoreSubmesh.begin();
  std::vector<CalCoreSubmesh *>::iterator otherIteratorCoreSubmesh = otherVectorCoreSubmesh.begin();
  int subMorphTargetID = (*iteratorCoreSubmesh)->getCoreSubMorphTargetCount();
  while(iteratorCoreSubmesh != m_vectorCoreSubmesh.end())
  {
	int count1 = (*iteratorCoreSubmesh)->getVertexCount();
	int count2 = (*otherIteratorCoreSubmesh)->getVertexCount();

    if( count1 != count2 )
    {
        char buf[2048] = {0}; // zero-initialize
#ifdef _MSC_VER
        _snprintf
#else
        snprintf
#endif
            (buf, sizeof(buf) - 1, "This mesh has a morph target child with different number of vertices: %s (%d vs child's %d)", morphTargetName.c_str(), count1, count2);
        CalError::setLastError(CalError::INTERNAL, __FILE__, __LINE__, buf);
        return -1;
    }
    ++iteratorCoreSubmesh;
    ++otherIteratorCoreSubmesh;
  }
  //Adding the blend targets to each of the core sub meshes
  iteratorCoreSubmesh = m_vectorCoreSubmesh.begin();
  otherIteratorCoreSubmesh = otherVectorCoreSubmesh.begin();
  while(iteratorCoreSubmesh != m_vectorCoreSubmesh.end())
  {
    int vertexCount = (*otherIteratorCoreSubmesh)->getVertexCount();
    boost::shared_ptr<CalCoreSubMorphTarget> pCalCoreSubMorphTarget(new CalCoreSubMorphTarget(morphTargetName));
    pCalCoreSubMorphTarget->reserve(vertexCount);
    const SSEArray<CalCoreSubmesh::Vertex>& vectorVertex = (*otherIteratorCoreSubmesh)->getVectorVertex();
    const std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> >& textCoordVector = (*otherIteratorCoreSubmesh)->getVectorVectorTextureCoordinate();
    
    for(int i = 0;i<vertexCount;++i)
    {
      CalCoreSubMorphTarget::BlendVertex blendVertex;
      blendVertex.position = vectorVertex[i].position.asCalVector();
      blendVertex.normal = vectorVertex[i].normal.asCalVector();
      blendVertex.textureCoords.clear();
      blendVertex.textureCoords.reserve(textCoordVector.size());
      for( size_t tcI = 0; tcI < textCoordVector.size(); tcI++ ) {
        blendVertex.textureCoords.push_back(textCoordVector[tcI][i]);
      }  
      if(!pCalCoreSubMorphTarget->setBlendVertex(i,blendVertex)) return -1;
    }
    (*iteratorCoreSubmesh)->addCoreSubMorphTarget(pCalCoreSubMorphTarget);
    ++iteratorCoreSubmesh;
    ++otherIteratorCoreSubmesh;
  }
  return subMorphTargetID;
}

void CalCoreMesh::scale(float factor) {
    std::vector<CalCoreSubmesh *>::iterator iteratorCoreSubmesh;
    for(iteratorCoreSubmesh = m_vectorCoreSubmesh.begin(); iteratorCoreSubmesh != m_vectorCoreSubmesh.end(); ++iteratorCoreSubmesh)
    {
        (*iteratorCoreSubmesh)->scale(factor);    
    }
}
