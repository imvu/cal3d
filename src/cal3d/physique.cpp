//****************************************************************************//
// physique.cpp                                                               //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#define SECURE_SCL 0

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <boost/static_assert.hpp>
#include "cal3d/error.h"
#include "cal3d/physique.h"
#include "cal3d/coremodel.h"
#include "cal3d/model.h"
#include "cal3d/mesh.h"
#include "cal3d/submesh.h"
#include "cal3d/skeleton.h"
#include "cal3d/bone.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/coresubmorphtarget.h"

static MorphIdAndWeight * MiawCache = NULL;
static unsigned int MiawCacheNumElements = 0;
void EnlargeMiawCacheAsNecessary(unsigned int numElements) {
  if( MiawCacheNumElements < numElements ) {
    if( MiawCache ) {
      delete [] MiawCache;
    }

    // Step up exponentially to reduce number of steps.
    MiawCacheNumElements = numElements * 2;
    MiawCache = new( MorphIdAndWeight [ MiawCacheNumElements ] );
  }
}

SSEArray<CalCoreSubmesh::Vertex> MorphSubmeshCache;

__forceinline void ScaleMatrix(CalSkeleton::BoneTransform& result, const CalSkeleton::BoneTransform& mat, const float s) {
  result.rowx.x = s * mat.rowx.x;
  result.rowx.y = s * mat.rowx.y;
  result.rowx.z = s * mat.rowx.z;
  result.rowx.w = s * mat.rowx.w;
  result.rowy.x = s * mat.rowy.x;
  result.rowy.y = s * mat.rowy.y;
  result.rowy.z = s * mat.rowy.z;
  result.rowy.w = s * mat.rowy.w;
  result.rowz.x = s * mat.rowz.x;
  result.rowz.y = s * mat.rowz.y;
  result.rowz.z = s * mat.rowz.z;
  result.rowz.w = s * mat.rowz.w;
}
__forceinline void AddScaledMatrix(CalSkeleton::BoneTransform& result, const CalSkeleton::BoneTransform& mat, const float s) {
  result.rowx.x += s * mat.rowx.x;
  result.rowx.y += s * mat.rowx.y;
  result.rowx.z += s * mat.rowx.z;
  result.rowx.w += s * mat.rowx.w;
  result.rowy.x += s * mat.rowy.x;
  result.rowy.y += s * mat.rowy.y;
  result.rowy.z += s * mat.rowy.z;
  result.rowy.w += s * mat.rowy.w;
  result.rowz.x += s * mat.rowz.x;
  result.rowz.y += s * mat.rowz.y;
  result.rowz.z += s * mat.rowz.z;
  result.rowz.w += s * mat.rowz.w;
}
__forceinline void TransformPoint(CalVector4& result, const CalSkeleton::BoneTransform& m, const CalVector4& v) {
  result.x = m.rowx.x * v.x + m.rowx.y * v.y + m.rowx.z * v.z + m.rowx.w;
  result.y = m.rowy.x * v.x + m.rowy.y * v.y + m.rowy.z * v.z + m.rowy.w;
  result.z = m.rowz.x * v.x + m.rowz.y * v.y + m.rowz.z * v.z + m.rowz.w;
}
__forceinline void TransformVector(CalVector4& result, const CalSkeleton::BoneTransform& m, const CalVector4& v) {
  result.x = m.rowx.x * v.x + m.rowx.y * v.y + m.rowx.z * v.z;
  result.y = m.rowy.x * v.x + m.rowy.y * v.y + m.rowy.z * v.z;
  result.z = m.rowz.x * v.x + m.rowz.y * v.y + m.rowz.z * v.z;
}

void calculateVerticesAndNormals_x87(
  const CalSkeleton::BoneTransform* boneTransforms,
  int vertexCount,
  const CalCoreSubmesh::Vertex* vertices,
  const CalCoreSubmesh::Influence* influences,
  CalVector4* output_vertex
) {

  CalSkeleton::BoneTransform total_transform;

  // calculate all submesh vertices
  while (vertexCount--) {
    ScaleMatrix(total_transform, boneTransforms[influences->boneId], influences->weight);

    while (!influences++->lastInfluenceForThisVertex) {
      AddScaledMatrix(total_transform, boneTransforms[influences->boneId], influences->weight);
    }

    TransformPoint(output_vertex[0], total_transform, vertices->position);
    TransformVector(output_vertex[1], total_transform, vertices->normal);
    ++vertices;
    output_vertex += 2;
  }
}

#define R_SHUFFLE(z, y, x, w) (z<<6) | (y<<4) | (x<<2) | w

void calculateVerticesAndNormals_SSE(
  const CalSkeleton::BoneTransform* boneTransforms,
  int vertexCount,
  const CalCoreSubmesh::Vertex* vertices,
  const CalCoreSubmesh::Influence* influences,
  CalVector4* output_vertices
) {
  #define OUTPUT_VERTEX_SIZE 0x20
  BOOST_STATIC_ASSERT(OUTPUT_VERTEX_SIZE == 2 * sizeof(CalVector4));

  #define INPUT_VERTEX_SIZE 0x30
  BOOST_STATIC_ASSERT(INPUT_VERTEX_SIZE == sizeof(CalCoreSubmesh::Vertex));

  #define INPUT_VERTEX_OFFSET_POSITION 0x00
  BOOST_STATIC_ASSERT(INPUT_VERTEX_OFFSET_POSITION == offsetof(CalCoreSubmesh::Vertex, position));
  #define INPUT_VERTEX_OFFSET_NORMAL 0x10
  BOOST_STATIC_ASSERT(INPUT_VERTEX_OFFSET_NORMAL == offsetof(CalCoreSubmesh::Vertex, normal));

  #define INFLUENCE_SIZE 12
  BOOST_STATIC_ASSERT(INFLUENCE_SIZE == sizeof(CalCoreSubmesh::Influence));
  #define INFLUENCE_BONEID_OFFSET 0
  BOOST_STATIC_ASSERT(INFLUENCE_BONEID_OFFSET == offsetof(CalCoreSubmesh::Influence, boneId));
  #define INFLUENCE_WEIGHT_OFFSET 4
  BOOST_STATIC_ASSERT(INFLUENCE_WEIGHT_OFFSET == offsetof(CalCoreSubmesh::Influence, weight));
  #define INFLUENCE_LAST_INFLUENCE_OFFSET 8
  BOOST_STATIC_ASSERT(INFLUENCE_LAST_INFLUENCE_OFFSET == offsetof(CalCoreSubmesh::Influence, lastInfluenceForThisVertex));

  #define BONE_TRANSFORM_SIZE 0x30
  BOOST_STATIC_ASSERT(BONE_TRANSFORM_SIZE == sizeof(CalSkeleton::BoneTransform));

  __asm {
    mov eax, vertexCount
    test eax, eax
    jz done

    mov ecx, output_vertices
    mov edx, influences
    mov esi, vertices
    mov edi, boneTransforms

loopVertex:

    // ScaleMatrix(total_transform, boneTransforms[influences->boneId], influences->weight);
    // total_transform in xmm0, xmm1, xmm2

    // Load weight into xmm0, xmm1, and xmm2
    // Load bone index into ebx
    movss xmm0, [edx+INFLUENCE_WEIGHT_OFFSET]
    mov ebx, dword ptr [edx+INFLUENCE_BONEID_OFFSET]
    shufps xmm0, xmm0, R_SHUFFLE(0, 0, 0, 0)
    lea ebx, [ebx*2+ebx]
    movaps xmm1, xmm0
    lea ebx, [ebx+ebx]
    movaps xmm2, xmm0

    // Scale matrix by weight.
    mulps xmm0, [edi+ebx*8+0x00] // xmm0 = m0, m1, m2, t0
    add esi, INPUT_VERTEX_SIZE
    mulps xmm1, [edi+ebx*8+0x10] // xmm1 = m3, m4, m5, t1
    test dword ptr [edx+INFLUENCE_LAST_INFLUENCE_OFFSET], 1
    mulps xmm2, [edi+ebx*8+0x20] // xmm2 = m6, m7, m8, t2

    je doneWeight

loopWeight:
    // Load weight into xmm3, xmm4, xmm5
    // Load bone index into ebx
    movss xmm3, [edx+INFLUENCE_SIZE+INFLUENCE_WEIGHT_OFFSET]
    mov ebx, dword ptr [edx+INFLUENCE_SIZE+INFLUENCE_BONEID_OFFSET]
    shufps xmm3, xmm3, R_SHUFFLE(0, 0, 0, 0)
    lea ebx, [ebx*2+ebx]
    movaps xmm4, xmm3
    lea ebx, [ebx+ebx]
    movaps xmm5, xmm3

    mulps xmm3, [edi+ebx*8+ 0] // xmm3 = m0, m1, m2, t0
    add edx, INFLUENCE_SIZE
    mulps xmm4, [edi+ebx*8+16] // xmm4 = m3, m4, m5, t1
    mulps xmm5, [edi+ebx*8+32] // xmm5 = m6, m7, m8, t2

    addps xmm0, xmm3
    test dword ptr [edx+INFLUENCE_LAST_INFLUENCE_OFFSET], 1
    addps xmm1, xmm4
    addps xmm2, xmm5

    jne loopWeight

doneWeight:
    // transform vertex
    movaps xmm3, [esi-INPUT_VERTEX_SIZE+INPUT_VERTEX_OFFSET_POSITION]
    movaps xmm4, xmm3
    movaps xmm5, xmm3

    mulps xmm3, xmm0
    mulps xmm4, xmm1
    mulps xmm5, xmm2

    movaps xmm6, xmm3 // xmm6 = m0, m1, m2, t0
    unpcklps xmm6, xmm4 // xmm6 = m0, m3, m1, m4
    unpckhps xmm3, xmm4 // xmm4 = m2, m5, t0, t1
    addps xmm6, xmm3 // xmm6 = m0+m2, m3+m5, m1+t0, m4+t1

    movaps xmm7, xmm5 // xmm7 = m6, m7, m8, t2
    movlhps xmm5, xmm6 // xmm5 = m6, m7, m0+m2, m3+m5
    movhlps xmm6, xmm7 // xmm6 = m8, t2, m1+t0, m4+t1
    addps xmm6, xmm5 // xmm6 = m6+m8, m7+t2, m0+m1+m2+t0, m3+m4+m5+t1

    movhps [ecx+0], xmm6

    pshufd xmm7, xmm6, R_SHUFFLE(1, 0, 2, 3) // xmm7 = m7+t2, m6+m8
    addss xmm7, xmm6 // xmm7 = m6+m8+m7+t2

    movss [ecx+8], xmm7

    // transform normal
    movaps xmm3, [esi-INPUT_VERTEX_SIZE+INPUT_VERTEX_OFFSET_NORMAL]
    movaps xmm4, xmm3
    add ecx, OUTPUT_VERTEX_SIZE
    movaps xmm5, xmm3

    mulps xmm3, xmm0
    mulps xmm4, xmm1
    sub eax, 1
    mulps xmm5, xmm2

    movaps xmm6, xmm3 // xmm6 = m0, m1, m2, t0
    unpcklps xmm6, xmm4 // xmm6 = m0, m3, m1, m4
    unpckhps xmm3, xmm4 // xmm3 = m2, m5, t0, t1
    addps xmm6, xmm3 // xmm6 = m0+m2, m3+m5, m1+t0, m4+t1

    movaps xmm7, xmm5 // xmm7 = m6, m7, m8, t2
    movlhps xmm5, xmm6 // xmm5 = m6, m7, m0+m2, m3+m5
    movhlps xmm6, xmm7 // xmm6 = m8, t2, m1+t0, m4+t1
    addps xmm6, xmm5 // xmm6 = m6+m8, m7+t2, m0+m1+m2+t0, m3+m4+m5+t1

    movhps [ecx+0x10], xmm6
    pshufd xmm7, xmm6, R_SHUFFLE(1, 0, 2, 3) // xmm7 = m7+t2, m6+m8
    addss xmm7, xmm6 // xmm7 = m6+m8+m7+t2
    movss [ecx+0x18], xmm7

    jnz loopVertex

done:
  }
}

void calculateVerticesAndNormals(
  const CalSkeleton::BoneTransform* boneTransforms,
  int vertexCount,
  const CalCoreSubmesh::Vertex* vertices,
  const CalCoreSubmesh::Influence* influences,
  CalVector4* output_vertices
) {
  return calculateVerticesAndNormals_x87(
    boneTransforms,
    vertexCount,
    vertices,
    influences,
    output_vertices);
}

void CalPhysique::calculateVerticesAndNormals(
  const CalSkeleton::BoneTransform* boneTransforms,
  CalSubmesh *pSubmesh,
  float *pVertexBuffer
) {
  const int vertexCount = pSubmesh->getVertexCount();
  const CalCoreSubmesh::Vertex* vertices = Cal::pointerFromVector(pSubmesh->getCoreSubmesh()->getVectorVertex());

  if (pSubmesh->getBaseWeight() != 1.0f) {
    if (vertexCount > MorphSubmeshCache.size()) {
      MorphSubmeshCache.resize(vertexCount);
    }

    // get the sub morph target vector from the core sub mesh
    CalCoreSubmesh::CoreSubMorphTargetVector& vectorSubMorphTarget = pSubmesh->getCoreSubmesh()->getVectorCoreSubMorphTarget();
    int morphTargetCount = pSubmesh->getMorphTargetWeightCount();
    EnlargeMiawCacheAsNecessary( morphTargetCount );
    unsigned int numMiaws;
    pSubmesh->getMorphIdAndWeightArray( MiawCache, & numMiaws, ( unsigned int ) morphTargetCount );

    // Fill MorphSubmeshCache w/ outputs of morph target calculation
    for (size_t vertexId = 0; vertexId < vertexCount; ++vertexId) {
      const CalCoreSubmesh::Vertex& sourceVertex = vertices[vertexId];
      CalCoreSubmesh::Vertex& destVertex = MorphSubmeshCache[vertexId];
      destVertex = sourceVertex;
      CalVector4& position = destVertex.position;
      CalVector4& normal = destVertex.normal;

      float baseWeight = pSubmesh->getBaseWeight();
      position.x = 0;
      position.y = 0;
      position.z = 0;
      normal.x = 0;
      normal.y = 0;
      normal.z = 0;
      for( unsigned i = 0; i < numMiaws; i++ ) {
        MorphIdAndWeight& miaw = MiawCache[ i ];
        int morphTargetId = miaw.morphId_;
        CalCoreSubMorphTarget::BlendVertex const * blendVertex =
          vectorSubMorphTarget[morphTargetId]->getBlendVertex(vertexId);
        float currentWeight = miaw.weight_;
        if( blendVertex ) {
          position.x += currentWeight*blendVertex->position.x;
          position.y += currentWeight*blendVertex->position.y;
          position.z += currentWeight*blendVertex->position.z;
          normal.x += currentWeight*blendVertex->normal.x;
          normal.y += currentWeight*blendVertex->normal.y;
          normal.z += currentWeight*blendVertex->normal.z;
        } else {
          baseWeight += currentWeight;
        }
      }
      position.x += baseWeight*sourceVertex.position.x;
      position.y += baseWeight*sourceVertex.position.y;
      position.z += baseWeight*sourceVertex.position.z;
      normal.x += baseWeight*sourceVertex.normal.x;
      normal.y += baseWeight*sourceVertex.normal.y;
      normal.z += baseWeight*sourceVertex.normal.z;
    }

    vertices = MorphSubmeshCache.data;
  }

  return calculateVerticesAndNormals(
    boneTransforms,
    vertexCount,
    vertices,
    Cal::pointerFromVector(pSubmesh->getCoreSubmesh()->influences),
    reinterpret_cast<CalVector4*>(pVertexBuffer));
}
