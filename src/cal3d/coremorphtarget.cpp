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

CalCoreMorphTarget::CalCoreMorphTarget(const std::string& n, size_t vertexCount, const VertexOffsetArray& vertexOffsets)
    : name(n)
    , morphTargetType(calculateType(n.c_str()))
    , vertexOffsets(vertexOffsets)
{
    cal3d::verify(vertexOffsets.size() <= vertexCount, "Cannot morph more vertices than in the base mesh");
    for (size_t i = 0; i < vertexOffsets.size(); ++i) {
        cal3d::verify(vertexOffsets[i].vertexId < vertexCount, "Cannot morph vertices outside of the base mesh");
    }
}

size_t CalCoreMorphTarget::size() const {
    size_t r = sizeof(CalCoreMorphTarget);
    r += sizeof(CalMorphTargetType);

    // Assume single texture coordinate pair.
    r += ::sizeInBytes(vertexOffsets);
    r += name.size();
    return r;
}

void CalCoreMorphTarget::scale(float factor) {
    VertexOffsetArray& mv = const_cast<VertexOffsetArray&>(vertexOffsets);
    for (VertexOffsetArray::iterator i = mv.begin(); i != mv.end(); ++i) {
        i->position.x *= factor;
        i->position.y *= factor;
        i->position.z *= factor;
    }
}
