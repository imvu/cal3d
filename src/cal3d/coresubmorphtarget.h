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
    struct BlendVertex : public AlignedMemory<16> {
        CalPoint4 position;
        CalVector4 normal;
        std::vector<CalCoreSubmesh::TextureCoordinate> textureCoords;
    };

    const std::string name;
    const CalMorphTargetType morphTargetType;

    CalCoreSubMorphTarget(const std::string& name);
    ~CalCoreSubMorphTarget();

    typedef std::vector<BlendVertex*> VectorBlendVertex;
    size_t size() const;
    size_t getBlendVertexCount() const {
        return m_vectorBlendVertex.size();
    }
    const std::vector<BlendVertex*>& getVectorBlendVertex() const {
        return m_vectorBlendVertex;
    }
    inline bool hasBlendVertex(int blendVertexId) const {
        return m_vectorBlendVertex[blendVertexId] != NULL;
    }
    inline BlendVertex const* getBlendVertex(int blendVertexId) {
        return m_vectorBlendVertex[blendVertexId];
    }
    void reserve(size_t blendVertexCount);
    bool setBlendVertex(int vertexId, const BlendVertex& vertex);

private:
    std::vector<BlendVertex*> m_vectorBlendVertex;
    CalMorphTargetType m_morphTargetType;
};
typedef boost::shared_ptr<CalCoreSubMorphTarget> CalCoreSubMorphTargetPtr;
