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
#include "cal3d/coremorphtarget.h"

size_t sizeInBytes(const CalCoreSubmeshPtr& submesh) {
    return sizeof(CalCoreSubmeshPtr) + submesh->sizeInBytes();
}

size_t CalCoreMesh::sizeInBytes() const {
    return sizeof(*this) + ::sizeInBytes(submeshes);
}

size_t CalCoreMesh::addAsMorphTarget(CalCoreMesh* pCoreMesh, std::string const& morphTargetName) {
    //Check if the numbers of vertices allow a blending
    CalCoreMesh::CalCoreSubmeshVector& otherVectorCoreSubmesh = pCoreMesh->submeshes;
    if (submeshes.size() != otherVectorCoreSubmesh.size()) {
        CalError::setLastError(CalError::INTERNAL, __FILE__, __LINE__, "This mesh has children with different numbers of materials");
        return -1;
    }
    if (submeshes.size() == 0) {
        CalError::setLastError(CalError::INTERNAL, __FILE__, __LINE__, "Mesh has no submeshes");
        return -1;
    }
    CalCoreMesh::CalCoreSubmeshVector::iterator iteratorCoreSubmesh = submeshes.begin();
    CalCoreMesh::CalCoreSubmeshVector::iterator otherIteratorCoreSubmesh = otherVectorCoreSubmesh.begin();
    size_t subMorphTargetID = (*iteratorCoreSubmesh)->getCoreSubMorphTargetCount();
    while (iteratorCoreSubmesh != submeshes.end()) {
        size_t count1 = (*iteratorCoreSubmesh)->getVertexCount();
        size_t count2 = (*otherIteratorCoreSubmesh)->getVertexCount();

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
    iteratorCoreSubmesh = submeshes.begin();
    otherIteratorCoreSubmesh = otherVectorCoreSubmesh.begin();
    while (iteratorCoreSubmesh != submeshes.end()) {
        size_t vertexCount = (*otherIteratorCoreSubmesh)->getVertexCount();
        CalCoreMorphTargetPtr pCalCoreMorphTarget(new CalCoreMorphTarget(morphTargetName, vertexCount));
        const SSEArray<CalCoreSubmesh::Vertex>& vectorVertex = (*otherIteratorCoreSubmesh)->getVectorVertex();

        for (int i = 0; i < vertexCount; ++i) {
            CalCoreMorphTarget::BlendVertex blendVertex;
            blendVertex.position = vectorVertex[i].position;
            blendVertex.normal = vectorVertex[i].normal;
            if (!pCalCoreMorphTarget->setBlendVertex(i, blendVertex)) {
                return -1;
            }
        }
        (*iteratorCoreSubmesh)->addCoreSubMorphTarget(pCalCoreMorphTarget);
        ++iteratorCoreSubmesh;
        ++otherIteratorCoreSubmesh;
    }
    return subMorphTargetID;
}

void CalCoreMesh::scale(float factor) {
    for (CalCoreSubmeshVector::iterator i = submeshes.begin(); i != submeshes.end(); ++i) {
        (*i)->scale(factor);
    }
}

void CalCoreMesh::fixup(const CalCoreSkeletonPtr& skeleton) {
    for (CalCoreSubmeshVector::iterator i = submeshes.begin(); i != submeshes.end(); ++i) {
        (*i)->fixup(skeleton);
    }
}
