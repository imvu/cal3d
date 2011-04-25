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

#include <stdio.h>
#include "cal3d/coremesh.h"
#include "cal3d/error.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/coresubmorphtarget.h"

size_t sizeInBytes(const CalCoreSubmeshPtr& submesh) {
    return sizeof(CalCoreSubmeshPtr) + submesh->sizeInBytes();
}

size_t CalCoreMesh::sizeInBytes() const {
    return sizeof(*this) + ::sizeInBytes(m_vectorCoreSubmesh);
}

int CalCoreMesh::addCoreSubmesh(const boost::shared_ptr<CalCoreSubmesh>& pCoreSubmesh) {
    int submeshId = m_vectorCoreSubmesh.size();
    m_vectorCoreSubmesh.push_back(pCoreSubmesh);
    return submeshId;
}

CalCoreMesh::CalCoreSubmeshVector& CalCoreMesh::getVectorCoreSubmesh() {
    return m_vectorCoreSubmesh;
}

int CalCoreMesh::addAsMorphTarget(CalCoreMesh* pCoreMesh, std::string const& morphTargetName) {
    //Check if the numbers of vertices allow a blending
    CalCoreMesh::CalCoreSubmeshVector& otherVectorCoreSubmesh = pCoreMesh->getVectorCoreSubmesh();
    if (m_vectorCoreSubmesh.size() != otherVectorCoreSubmesh.size()) {
        CalError::setLastError(CalError::INTERNAL, __FILE__, __LINE__, "This mesh has children with different numbers of materials");
        return -1;
    }
    if (m_vectorCoreSubmesh.size() == 0) {
        CalError::setLastError(CalError::INTERNAL, __FILE__, __LINE__, "Mesh has no submeshes");
        return -1;
    }
    CalCoreMesh::CalCoreSubmeshVector::iterator iteratorCoreSubmesh = m_vectorCoreSubmesh.begin();
    CalCoreMesh::CalCoreSubmeshVector::iterator otherIteratorCoreSubmesh = otherVectorCoreSubmesh.begin();
    int subMorphTargetID = (*iteratorCoreSubmesh)->getCoreSubMorphTargetCount();
    while (iteratorCoreSubmesh != m_vectorCoreSubmesh.end()) {
        int count1 = (*iteratorCoreSubmesh)->getVertexCount();
        int count2 = (*otherIteratorCoreSubmesh)->getVertexCount();

        if (count1 != count2) {
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
    while (iteratorCoreSubmesh != m_vectorCoreSubmesh.end()) {
        int vertexCount = (*otherIteratorCoreSubmesh)->getVertexCount();
        boost::shared_ptr<CalCoreSubMorphTarget> pCalCoreSubMorphTarget(new CalCoreSubMorphTarget(morphTargetName));
        pCalCoreSubMorphTarget->reserve(vertexCount);
        const SSEArray<CalCoreSubmesh::Vertex>& vectorVertex = (*otherIteratorCoreSubmesh)->getVectorVertex();
        const std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> >& textCoordVector = (*otherIteratorCoreSubmesh)->getVectorVectorTextureCoordinate();

        for (int i = 0; i < vertexCount; ++i) {
            CalCoreSubMorphTarget::BlendVertex blendVertex;
            blendVertex.position = vectorVertex[i].position;
            blendVertex.normal = vectorVertex[i].normal;
            blendVertex.textureCoords.clear();
            blendVertex.textureCoords.reserve(textCoordVector.size());
            for (size_t tcI = 0; tcI < textCoordVector.size(); tcI++) {
                blendVertex.textureCoords.push_back(textCoordVector[tcI][i]);
            }
            if (!pCalCoreSubMorphTarget->setBlendVertex(i, blendVertex)) {
                return -1;
            }
        }
        (*iteratorCoreSubmesh)->addCoreSubMorphTarget(pCalCoreSubMorphTarget);
        ++iteratorCoreSubmesh;
        ++otherIteratorCoreSubmesh;
    }
    return subMorphTargetID;
}

void CalCoreMesh::scale(float factor) {
    CalCoreMesh::CalCoreSubmeshVector::iterator iteratorCoreSubmesh;
    for (iteratorCoreSubmesh = m_vectorCoreSubmesh.begin(); iteratorCoreSubmesh != m_vectorCoreSubmesh.end(); ++iteratorCoreSubmesh) {
        (*iteratorCoreSubmesh)->scale(factor);
    }
}
