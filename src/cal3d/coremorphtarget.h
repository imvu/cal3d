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

class CAL3D_API CalCoreMorphTarget {
public:
    struct BlendVertex : public cal3d::AlignedMemory<16> {
        CalPoint4 position;
        CalVector4 normal;
    };
    typedef std::vector<BlendVertex*> VectorBlendVertex;

    const std::string name;
    const CalMorphTargetType morphTargetType;

    CalCoreMorphTarget(const std::string& name, size_t vertexCount);
    ~CalCoreMorphTarget();

    size_t size() const;
    const std::vector<BlendVertex*>& getVertices() const {
        return m_vectorBlendVertex;
    }
    bool setBlendVertex(int vertexId, const BlendVertex& vertex);

    void scale(float factor);

private:
    std::vector<BlendVertex*> m_vectorBlendVertex;
};
typedef boost::shared_ptr<CalCoreMorphTarget> CalCoreMorphTargetPtr;
