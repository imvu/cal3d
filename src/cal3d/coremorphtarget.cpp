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
#include "cal3d/coremorphtarget.h"

static CalMorphTargetType calculateType(const char* s2) {
    const char* dot = strrchr(s2, '.');
    if (dot) {
        dot++;
        if (cal3d_stricmp(dot, "exclusive") == 0) {
            return CalMorphTargetTypeExclusive;
        } else if (cal3d_stricmp(dot, "additive") == 0) {
            return CalMorphTargetTypeAdditive;
        } else if (cal3d_stricmp(dot, "clamped") == 0) {
            return CalMorphTargetTypeClamped;
        } else if (cal3d_stricmp(dot, "average") == 0) {
            return CalMorphTargetTypeAverage;
        }
    }
    return CalMorphTargetTypeAdditive;
}

CalCoreMorphTarget::CalCoreMorphTarget(const std::string& n, size_t vertexCount)
    : name(n)
    , morphTargetType(calculateType(n.c_str()))
    , m_vectorBlendVertex(vertexCount)
{
    BlendVertex* null = 0;
    std::fill(m_vectorBlendVertex.begin(), m_vectorBlendVertex.end(), null);
}

CalCoreMorphTarget::~CalCoreMorphTarget() {
    for (size_t i = 0; i < m_vectorBlendVertex.size(); i++) {
        delete m_vectorBlendVertex[i];
    }
}

size_t CalCoreMorphTarget::size() const {
    size_t r = sizeof(CalCoreMorphTarget);
    r += sizeof(CalMorphTargetType);

    // Assume single texture coordinate pair.
    r += (sizeof(BlendVertex) + sizeof(CalCoreSubmesh::TextureCoordinate)) * m_vectorBlendVertex.size();
    r += name.size();
    return r;
}

bool CalCoreMorphTarget::setBlendVertex(int blendVertexId, const BlendVertex& blendVertex) {
    if ((blendVertexId < 0) || (blendVertexId >= (int)m_vectorBlendVertex.size())) {
        return false;
    }

    if (m_vectorBlendVertex[blendVertexId] == NULL) {
        m_vectorBlendVertex[blendVertexId] = new BlendVertex();
    }
    m_vectorBlendVertex[blendVertexId]->position = blendVertex.position;
    m_vectorBlendVertex[blendVertexId]->normal = blendVertex.normal;

    return true;
}

void CalCoreMorphTarget::scale(float factor) {
    for (std::vector<BlendVertex*>::iterator i = m_vectorBlendVertex.begin(); i != m_vectorBlendVertex.end(); ++i) {
        BlendVertex* v = *i;
        if (v) {
            v->position.x *= factor;
            v->position.y *= factor;
            v->position.z *= factor;
        }
    }
}
