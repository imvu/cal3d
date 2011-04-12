//****************************************************************************//
// physique.h                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/coresubmesh.h"
#include "cal3d/global.h"

struct BoneTransform;
class CalSubmesh;

namespace CalPhysique {
    typedef void (*SkinRoutine)(
        const BoneTransform*,
        int,
        const CalCoreSubmesh::Vertex*,
        const CalCoreSubmesh::Influence*,
        CalVector4*);

    CAL3D_API void calculateVerticesAndNormals_x87(
        const BoneTransform* boneTransforms,
        int vertexCount,
        const CalCoreSubmesh::Vertex* vertices,
        const CalCoreSubmesh::Influence* influences,
        CalVector4* output_vertex);

#ifndef IMVU_NO_INTRINSICS
    CAL3D_API void calculateVerticesAndNormals_SSE_intrinsics(
        const BoneTransform* boneTransforms,
        int vertexCount,
        const CalCoreSubmesh::Vertex* vertices,
        const CalCoreSubmesh::Influence* influences,
        CalVector4* output_vertex);
#endif

#ifndef IMVU_NO_ASM_BLOCKS
    CAL3D_API void calculateVerticesAndNormals_SSE(
        const BoneTransform* boneTransforms,
        int vertexCount,
        const CalCoreSubmesh::Vertex* vertices,
        const CalCoreSubmesh::Influence* influences,
        CalVector4* output_vertices);
#endif

    CAL3D_API void calculateVerticesAndNormals(
        const BoneTransform* boneTransforms,
        const CalSubmesh* pSubmesh,
        float* pVertexBuffer);
};
