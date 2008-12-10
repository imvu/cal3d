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
#include "cal3d/skeleton.h"

class CalSubmesh;

namespace CalPhysique
{
  CAL3D_API void calculateVerticesAndNormals_x87(
    const CalSkeleton::BoneTransform* boneTransforms,
    int vertexCount,
    const CalCoreSubmesh::Vertex* vertices,
    const CalCoreSubmesh::Influence* influences,
    CalVector4* output_vertex);
  CAL3D_API void calculateVerticesAndNormals_SSE_intrinsics(
    const CalSkeleton::BoneTransform* boneTransforms,
    int vertexCount,
    const CalCoreSubmesh::Vertex* vertices,
    const CalCoreSubmesh::Influence* influences,
    CalVector4* output_vertex);
  CAL3D_API void calculateVerticesAndNormals_SSE(
    const CalSkeleton::BoneTransform* boneTransforms,
    int vertexCount,
    const CalCoreSubmesh::Vertex* vertices,
    const CalCoreSubmesh::Influence* influences,
    CalVector4* output_vertices);

  CAL3D_API void calculateVerticesAndNormals(
    const CalSkeleton::BoneTransform* boneTransforms,
    CalSubmesh* pSubmesh,
    float* pVertexBuffer);
};
