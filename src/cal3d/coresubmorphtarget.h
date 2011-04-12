//****************************************************************************//
// coresubmorphtarget.h                                                       //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"
#include "cal3d/vector.h"
#include "cal3d/coresubmesh.h"

class CAL3D_API CalCoreSubMorphTarget {
public:
    /// The core sub morph target BlendVertex.
    typedef struct {
        CalVector position;
        CalVector normal;
        std::vector<CalCoreSubmesh::TextureCoordinate> textureCoords;
    } BlendVertex;

public:
    CalCoreSubMorphTarget(const std::string& name);
    ~CalCoreSubMorphTarget();

    typedef std::vector<BlendVertex*> VectorBlendVertex;
    unsigned int size() const;
    int getBlendVertexCount() const;
    const std::vector<BlendVertex*>& getVectorBlendVertex() const;
    inline bool hasBlendVertex(int blendVertexId) const {
        return m_vectorBlendVertex[blendVertexId] != NULL;
    }
    inline BlendVertex const* getBlendVertex(int blendVertexId) {
        return m_vectorBlendVertex[blendVertexId];
    }
    void reserve(int blendVertexCount);
    bool setBlendVertex(int vertexId, const BlendVertex& vertex);
    CalMorphTargetType morphTargetType();

    const std::string& name() const {
        return m_morphTargetName;
    }

private:
    std::vector<BlendVertex*> m_vectorBlendVertex;
    std::string m_morphTargetName;
    CalMorphTargetType m_morphTargetType;
};
