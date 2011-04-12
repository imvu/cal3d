//****************************************************************************//
// coresubmorphtarget.cpp                                                     //
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

#include <string.h>
#include "cal3d/coresubmorphtarget.h"

CalCoreSubMorphTarget::CalCoreSubMorphTarget(const std::string& name)
    : m_morphTargetName(name)
    , m_morphTargetType(CalMorphTargetTypeAdditive) {
    // If the name ends in ".Additive" or ".Exclusive" or ".Clamped"
    // or ".Average" then set the type of the morph target.  By
    // default it is Additive.
    char const* s2 = name.c_str();
    char const* dot = strrchr(s2, '.');
    if (dot) {
        dot++;
        if (cal3d_stricmp(dot, "exclusive") == 0) {
            m_morphTargetType = CalMorphTargetTypeExclusive;
        } else if (cal3d_stricmp(dot, "additive") == 0) {
            m_morphTargetType = CalMorphTargetTypeAdditive;
        } else if (cal3d_stricmp(dot, "clamped") == 0) {
            m_morphTargetType = CalMorphTargetTypeClamped;
        } else if (cal3d_stricmp(dot, "average") == 0) {
            m_morphTargetType = CalMorphTargetTypeAverage;
        }
    }
}

CalCoreSubMorphTarget::~CalCoreSubMorphTarget() {
    for (int i = 0; i < m_vectorBlendVertex.size(); i++) {
        delete m_vectorBlendVertex[i];
    }
}

unsigned int CalCoreSubMorphTarget::size() const {
    unsigned int r = sizeof(CalCoreSubMorphTarget);
    r += sizeof(CalMorphTargetType);

    // Assume single texture coordinate pair.
    r += (sizeof(BlendVertex) + sizeof(CalCoreSubmesh::TextureCoordinate)) * m_vectorBlendVertex.size();
    r += m_morphTargetName.size();
    return r;
}



const std::vector<CalCoreSubMorphTarget::BlendVertex*>& CalCoreSubMorphTarget::getVectorBlendVertex() const {
    return m_vectorBlendVertex;
}



int CalCoreSubMorphTarget::getBlendVertexCount() const {
    return m_vectorBlendVertex.size();
}

void CalCoreSubMorphTarget::reserve(int blendVertexCount) {
    // reserve the space needed in all the vectors
    m_vectorBlendVertex.reserve(blendVertexCount);
    m_vectorBlendVertex.resize(blendVertexCount);
    for (int i = 0; i < m_vectorBlendVertex.size(); i++) {
        m_vectorBlendVertex[i] = NULL;
    }
}



bool CalCoreSubMorphTarget::setBlendVertex(int blendVertexId, const BlendVertex& blendVertex) {
    if ((blendVertexId < 0) || (blendVertexId >= (int)m_vectorBlendVertex.size())) {
        return false;
    }

    if (m_vectorBlendVertex[blendVertexId] == NULL) {
        m_vectorBlendVertex[blendVertexId] = new BlendVertex();
    }
    m_vectorBlendVertex[blendVertexId]->position = blendVertex.position;
    m_vectorBlendVertex[blendVertexId]->normal = blendVertex.normal;
    m_vectorBlendVertex[blendVertexId]->textureCoords.clear();
    m_vectorBlendVertex[blendVertexId]->textureCoords.reserve(blendVertex.textureCoords.size());
    for (int tcI = 0; tcI < blendVertex.textureCoords.size(); tcI++) {
        m_vectorBlendVertex[blendVertexId]->textureCoords.push_back(blendVertex.textureCoords[tcI]);
    }

    return true;
}


CalMorphTargetType
CalCoreSubMorphTarget::morphTargetType() {
    return m_morphTargetType;
}
