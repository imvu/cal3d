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
    struct MorphVertex {
        size_t vertexId;
        CalPoint4 position;
        CalVector4 normal;
    };
    typedef cal3d::SSEArray<MorphVertex> MorphVertexArray;

    const std::string name;
    const CalMorphTargetType morphTargetType;
    const MorphVertexArray morphVertices;

    CalCoreMorphTarget(const std::string& name, const size_t vertexCount, const MorphVertexArray& morphVertices);

    size_t size() const;

    void scale(float factor);
};
typedef boost::shared_ptr<CalCoreMorphTarget> CalCoreMorphTargetPtr;
