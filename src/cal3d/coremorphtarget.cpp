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
    , vertexCount(vertexCount)
{
}

size_t CalCoreMorphTarget::size() const {
    size_t r = sizeof(CalCoreMorphTarget);
    r += sizeof(CalMorphTargetType);

    // Assume single texture coordinate pair.
    r += (sizeof(MorphVertex) + sizeof(CalCoreSubmesh::TextureCoordinate)) * m_vectorBlendVertex.size();
    r += name.size();
    return r;
}

void CalCoreMorphTarget::addMorphVertex(const MorphVertex& blendVertex) {
    cal3d::verify(blendVertex.vertexId < vertexCount, "Cannot morph out-of-range vertex");
    cal3d::verify(m_vectorBlendVertex.size() < vertexCount, "Cannot add more morph vertices than mesh vertices");

    m_vectorBlendVertex.push_back(blendVertex);
}

void CalCoreMorphTarget::scale(float factor) {
    for (MorphVertexArray::iterator i = m_vectorBlendVertex.begin(); i != m_vectorBlendVertex.end(); ++i) {
        i->position.x *= factor;
        i->position.y *= factor;
        i->position.z *= factor;
    }
}
