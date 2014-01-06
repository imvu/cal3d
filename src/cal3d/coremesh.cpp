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

bool CalCoreMesh::addAsMorphTarget(CalCoreMesh* pCoreMesh, std::string const& morphTargetName) {
    //Check if the numbers of vertices allow a blending
    CalCoreMesh::CalCoreSubmeshVector& otherVectorCoreSubmesh = pCoreMesh->submeshes;
    if (submeshes.size() != otherVectorCoreSubmesh.size()) {
        CalError::setLastError(CalError::INTERNAL, __FILE__, __LINE__, "This mesh has children with different numbers of materials");
        return false;
    }
    if (submeshes.size() == 0) {
        CalError::setLastError(CalError::INTERNAL, __FILE__, __LINE__, "Mesh has no submeshes");
        return false;
    }
    CalCoreMesh::CalCoreSubmeshVector::iterator iteratorCoreSubmesh = submeshes.begin();
    CalCoreMesh::CalCoreSubmeshVector::iterator otherIteratorCoreSubmesh = otherVectorCoreSubmesh.begin();
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
            (buf, sizeof(buf) - 1, "This mesh has a morph target child with different number of vertices: %s (%zu vs child's %zu)", morphTargetName.c_str(), count1, count2);
            CalError::setLastError(CalError::INTERNAL, __FILE__, __LINE__, buf);
            return false;
        }
        ++iteratorCoreSubmesh;
        ++otherIteratorCoreSubmesh;
    }
    //Adding the blend targets to each of the core sub meshes
    iteratorCoreSubmesh = submeshes.begin();
    otherIteratorCoreSubmesh = otherVectorCoreSubmesh.begin();
    while (iteratorCoreSubmesh != submeshes.end()) {
        size_t vertexCount = (*otherIteratorCoreSubmesh)->getVertexCount();
        const CalCoreSubmesh::VectorVertex& vectorVertex = (*otherIteratorCoreSubmesh)->getVectorVertex();

        CalCoreMorphTarget::VertexOffsetArray vertices;
        for (unsigned i = 0; i < vertexCount; ++i) {
            vertices.push_back(VertexOffset(
                i,
                vectorVertex[i].position,
                vectorVertex[i].normal));
        }
        CalCoreMorphTargetPtr pCalCoreMorphTarget(new CalCoreMorphTarget(morphTargetName, vertexCount, vertices));
        (*iteratorCoreSubmesh)->addMorphTarget(pCalCoreMorphTarget);
        ++iteratorCoreSubmesh;
        ++otherIteratorCoreSubmesh;
    }
    return true;
}

void CalCoreMesh::replaceMeshWithMorphTarget(const std::string& morphTargetName) {
    for (auto i = submeshes.begin(); i != submeshes.end(); ++i) {
        (*i)->replaceMeshWithMorphTarget(morphTargetName);
    }
}

void CalCoreMesh::scale(float factor) {
    for (auto i = submeshes.begin(); i != submeshes.end(); ++i) {
        (*i)->scale(factor);
    }
}

void CalCoreMesh::applyZUpToYUp() {
    for (auto i = submeshes.begin(); i != submeshes.end(); ++i) {
        (*i)->applyZUpToYUp();
    }
}

void CalCoreMesh::fixup(const CalCoreSkeletonPtr& skeleton) {
    for (auto i = submeshes.begin(); i != submeshes.end(); ++i) {
        (*i)->fixup(skeleton);
    }
}
