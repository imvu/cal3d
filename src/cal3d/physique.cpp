//****************************************************************************//
// physique.cpp                                                               //
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

#include <assert.h>
#ifndef IMVU_NO_INTRINSICS
#include <xmmintrin.h>
#endif
#include <boost/static_assert.hpp>
#include "cal3d/error.h"
#include "cal3d/physique.h"
#include "cal3d/model.h"
#include "cal3d/mesh.h"
#include "cal3d/submesh.h"
#include "cal3d/skeleton.h"
#include "cal3d/bone.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/coresubmorphtarget.h"

static MorphIdAndWeight* MiawCache = NULL;
static unsigned int MiawCacheNumElements = 0;
void EnlargeMiawCacheAsNecessary(unsigned int numElements) {
    if (MiawCacheNumElements < numElements) {
        if (MiawCache) {
            delete [] MiawCache;
        }

        // Step up exponentially to reduce number of steps.
        MiawCacheNumElements = numElements * 2;
        MiawCache = new(MorphIdAndWeight [ MiawCacheNumElements ]);
    }
}

SSEArray<CalCoreSubmesh::Vertex> MorphSubmeshCache;

// calculateVerticesAndNormals_{x87,SSE_intrinsics,SSE} are:
/*
  SSE Optimized Skinning Without Normals or Tangents
  Copyright (C) 2005 Id Software, Inc.
  Written by J.M.P. van Waveren
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.
*/

CAL3D_FORCEINLINE void ScaleMatrix(BoneTransform& result, const BoneTransform& mat, const float s) {
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
CAL3D_FORCEINLINE void AddScaledMatrix(BoneTransform& result, const BoneTransform& mat, const float s) {
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
CAL3D_FORCEINLINE void TransformPoint(CalVector4& result, const BoneTransform& m, const CalBase4& v) {
    result.x = m.rowx.x * v.x + m.rowx.y * v.y + m.rowx.z * v.z + m.rowx.w;
    result.y = m.rowy.x * v.x + m.rowy.y * v.y + m.rowy.z * v.z + m.rowy.w;
    result.z = m.rowz.x * v.x + m.rowz.y * v.y + m.rowz.z * v.z + m.rowz.w;
}
CAL3D_FORCEINLINE void TransformVector(CalVector4& result, const BoneTransform& m, const CalBase4& v) {
    result.x = m.rowx.x * v.x + m.rowx.y * v.y + m.rowx.z * v.z;
    result.y = m.rowy.x * v.x + m.rowy.y * v.y + m.rowy.z * v.z;
    result.z = m.rowz.x * v.x + m.rowz.y * v.y + m.rowz.z * v.z;
}

void CalPhysique::calculateVerticesAndNormals_x87(
    const BoneTransform* boneTransforms,
    int vertexCount,
    const CalCoreSubmesh::Vertex* vertices,
    const CalCoreSubmesh::Influence* influences,
    CalVector4* output_vertex
) {

    BoneTransform total_transform;

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


#ifndef IMVU_NO_INTRINSICS
void CalPhysique::calculateVerticesAndNormals_SSE_intrinsics(
    const BoneTransform* boneTransforms,
    int vertexCount,
    const CalCoreSubmesh::Vertex* vertices,
    const CalCoreSubmesh::Influence* influences,
    CalVector4* output_vertex
) {

    __m128 rowx;
    __m128 rowy;
    __m128 rowz;
    __m128 weight;

    // calculate all submesh vertices
    while (vertexCount--) {
        weight = _mm_load_ss(&influences->weight);
        weight = _mm_shuffle_ps(weight, weight, _MM_SHUFFLE(0, 0, 0, 0));

        const BoneTransform& bt = boneTransforms[influences->boneId];

        rowx = _mm_mul_ps(_mm_load_ps((const float*)&bt.rowx), weight);
        rowy = _mm_mul_ps(_mm_load_ps((const float*)&bt.rowy), weight);
        rowz = _mm_mul_ps(_mm_load_ps((const float*)&bt.rowz), weight);

        while (!influences++->lastInfluenceForThisVertex) {
            weight = _mm_load_ss(&influences->weight);
            weight = _mm_shuffle_ps(weight, weight, _MM_SHUFFLE(0, 0, 0, 0));

            const BoneTransform& bt = boneTransforms[influences->boneId];

            rowx = _mm_add_ps(rowx, _mm_mul_ps(_mm_load_ps((const float*)&bt.rowx), weight));
            rowy = _mm_add_ps(rowy, _mm_mul_ps(_mm_load_ps((const float*)&bt.rowy), weight));
            rowz = _mm_add_ps(rowz, _mm_mul_ps(_mm_load_ps((const float*)&bt.rowz), weight));
        }

        {
            // transform position
            const __m128 position = _mm_load_ps((const float*)&vertices->position);

            const __m128 mulx = _mm_mul_ps(position, rowx);
            const __m128 muly = _mm_mul_ps(position, rowy);
            const __m128 mulz = _mm_mul_ps(position, rowz);

            const __m128 copylo = _mm_unpacklo_ps(mulx, muly);
            const __m128 copyhi = _mm_unpackhi_ps(mulx, muly);
            const __m128 sum1 = _mm_add_ps(copylo, copyhi);

            const __m128 lhps = _mm_movelh_ps(mulz, sum1);
            const __m128 hlps = _mm_movehl_ps(sum1, mulz);
            const __m128 sum2 = _mm_add_ps(lhps, hlps);

            _mm_storeh_pi((__m64*)output_vertex, sum2);

            __m128 sum3 = _mm_shuffle_ps(sum2, sum2, _MM_SHUFFLE(1, 1, 1, 1));
            sum3 = _mm_add_ss(sum2, sum3);
            _mm_store_ss(&output_vertex->z, sum3);
        }

        // transform normal
        {
            const __m128 normal = _mm_load_ps((const float*)&vertices->normal);

            const __m128 mulx = _mm_mul_ps(normal, rowx);
            const __m128 muly = _mm_mul_ps(normal, rowy);
            const __m128 mulz = _mm_mul_ps(normal, rowz);

            const __m128 copylo = _mm_unpacklo_ps(mulx, muly);
            const __m128 copyhi = _mm_unpackhi_ps(mulx, muly);
            const __m128 sum1 = _mm_add_ps(copylo, copyhi);

            const __m128 lhps = _mm_movelh_ps(mulz, sum1);
            const __m128 hlps = _mm_movehl_ps(sum1, mulz);
            const __m128 sum2 = _mm_add_ps(lhps, hlps);

            _mm_storeh_pi((__m64*)&output_vertex[1], sum2);

            __m128 sum3 = _mm_shuffle_ps(sum2, sum2, _MM_SHUFFLE(1, 1, 1, 1));
            sum3 = _mm_add_ss(sum2, sum3);
            _mm_store_ss(&output_vertex[1].z, sum3);
        }

        ++vertices;
        output_vertex += 2;
    }
}
#endif

#ifndef IMVU_NO_ASM_BLOCKS
#define R_SHUFFLE_D(o0, o1, o2, o3) ((o3 & 3) << 6 | (o2 & 3) << 4 | (o1 & 3) << 2 | (o0 & 3))

void CalPhysique::calculateVerticesAndNormals_SSE(
    const BoneTransform* boneTransforms,
    int vertexCount,
    const CalCoreSubmesh::Vertex* vertices,
    const CalCoreSubmesh::Influence* influences,
    CalVector4* output_vertices
) {
#define OUTPUT_VERTEX_SIZE 0x20
    BOOST_STATIC_ASSERT(OUTPUT_VERTEX_SIZE == 2 * sizeof(CalVector4));

#define INPUT_VERTEX_SIZE 0x20
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
    BOOST_STATIC_ASSERT(BONE_TRANSFORM_SIZE == sizeof(BoneTransform));

    // This is true on my Xeons anyway.
#define CACHE_LINE_SIZE 64

    __asm {
        mov eax, vertexCount
        test eax, eax
        jz done

        mov ecx, output_vertices
        mov edx, influences
        mov esi, vertices
        mov edi, boneTransforms

        loopVertex:
        // Load weight into xmm0, xmm1, and xmm2
        // Load bone index into ebx
        movss xmm0, [edx+INFLUENCE_WEIGHT_OFFSET]
        mov ebx, dword ptr [edx+INFLUENCE_BONEID_OFFSET]
        shufps xmm0, xmm0, _MM_SHUFFLE(0, 0, 0, 0)
        lea ebx, [ebx*2+ebx]
        movaps xmm1, xmm0
        lea ebx, [ebx+ebx]
        movaps xmm2, xmm0

        add edx, INFLUENCE_SIZE

        // prefetch vertex position (important - memory bandwidth/latency is our biggest cost on the P4)
        movaps xmm7, [esi+INPUT_VERTEX_OFFSET_POSITION]
        prefetcht0 [esi+INPUT_VERTEX_OFFSET_POSITION+CACHE_LINE_SIZE]

        // Scale matrix by weight.
        mulps xmm0, [edi+ebx*8+0x00] // xmm0 = m0, m1, m2, t0
        add esi, INPUT_VERTEX_SIZE
        mulps xmm1, [edi+ebx*8+0x10] // xmm1 = m3, m4, m5, t1
        test dword ptr [edx-INFLUENCE_SIZE+INFLUENCE_LAST_INFLUENCE_OFFSET], 1
        mulps xmm2, [edi+ebx*8+0x20] // xmm2 = m6, m7, m8, t2

        jnz doneWeight

        loopWeight:
        // Load weight into xmm3, xmm4, xmm5
        // Load bone index into ebx
        movss xmm3, [edx+INFLUENCE_WEIGHT_OFFSET]
        mov ebx, dword ptr [edx+INFLUENCE_BONEID_OFFSET]
        shufps xmm3, xmm3, _MM_SHUFFLE(0, 0, 0, 0)
        lea ebx, [ebx*2+ebx]
        movaps xmm4, xmm3
        lea ebx, [ebx+ebx]
        movaps xmm5, xmm3

        mulps xmm3, [edi+ebx*8+ 0] // xmm3 = m0, m1, m2, t0
        add edx, INFLUENCE_SIZE
        mulps xmm4, [edi+ebx*8+16] // xmm4 = m3, m4, m5, t1
        mulps xmm5, [edi+ebx*8+32] // xmm5 = m6, m7, m8, t2

        addps xmm0, xmm3
        test dword ptr [edx-INFLUENCE_SIZE+INFLUENCE_LAST_INFLUENCE_OFFSET], 1
        addps xmm1, xmm4
        addps xmm2, xmm5

        jz loopWeight

        doneWeight:
        // transform vertex
        movaps xmm3, xmm7
        movaps xmm4, xmm7
        movaps xmm5, xmm7

        mulps xmm3, xmm0
        mulps xmm4, xmm1
        mulps xmm5, xmm2

        movaps xmm6, xmm3 // xmm6 = m0, m1, m2, t0
        unpcklps xmm6, xmm4 // xmm6 = m0, m3, m1, m4
        unpckhps xmm3, xmm4 // xmm3 = m2, m5, t0, t1
        addps xmm6, xmm3 // xmm6 = m0+m2, m3+m5, m1+t0, m4+t1

        // prefetch vertex normal (important - memory bandwidth/latency is our biggest cost on the P4)
        movaps xmm3, [esi-INPUT_VERTEX_SIZE+INPUT_VERTEX_OFFSET_NORMAL]

        movaps xmm7, xmm5 // xmm7 = m6, m7, m8, t2
        movlhps xmm5, xmm6 // xmm5 = m6, m7, m0+m2, m3+m5
        movhlps xmm6, xmm7 // xmm6 = m8, t2, m1+t0, m4+t1
        addps xmm6, xmm5 // xmm6 = m6+m8, m7+t2, m0+m1+m2+t0, m3+m4+m5+t1

        movhps [ecx+0], xmm6

        pshufd xmm7, xmm6, R_SHUFFLE_D(1, 1, 1, 1) // xmm7 = m7+t2
        addss xmm7, xmm6 // xmm7 = m6+m8+m7+t2
        movss [ecx+8], xmm7

        // transform normal (copy into xmm4 and xmm5)
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

        movhps [ecx-OUTPUT_VERTEX_SIZE+0x10], xmm6
        pshufd xmm7, xmm6, R_SHUFFLE_D(1, 1, 1, 1) // xmm7 = m7+t2
        addss xmm7, xmm6 // xmm7 = m6+m8+m7+t2
        movss [ecx-OUTPUT_VERTEX_SIZE+0x18], xmm7

        jnz loopVertex // see |sub eax, 1| above

        done:
    }
}
#endif

void automaticallyDetectSkinRoutine(
    const BoneTransform* boneTransforms,
    int vertexCount,
    const CalCoreSubmesh::Vertex* vertices,
    const CalCoreSubmesh::Influence* influences,
    CalVector4* output_vertices);

static CalPhysique::SkinRoutine optimizedSkinRoutine = automaticallyDetectSkinRoutine;

void automaticallyDetectSkinRoutine(
    const BoneTransform* boneTransforms,
    int vertexCount,
    const CalCoreSubmesh::Vertex* vertices,
    const CalCoreSubmesh::Influence* influences,
    CalVector4* output_vertices
) {

#ifdef IMVU_NO_ASM_BLOCKS
#ifdef IMVU_NO_INTRINSICS
    optimizedSkinRoutine = CalPhysique::calculateVerticesAndNormals_x87;
#else
    optimizedSkinRoutine = CalPhysique::calculateVerticesAndNormals_SSE_intrinsics;
#endif
#else
    unsigned features = 0;
    /* For some reason, on OSX, we need to pop and push EBX to prevent
     * byproducts from striking ventilation machinery.
     * -- andy 7 April 2010
     */
    __asm {
        push ebx
        mov eax, 1
        cpuid
        mov features, edx
        pop ebx
    }

    // Technically I should be verifying that the OS saves and restores
    // the XMM0-7 registers as part of the thread state, but I'm going
    // to assume that all of the OSes we support do so.
    const int SSE_BIT  = 1 << 25;
    const int SSE2_BIT = 1 << 26;

    if ((features & SSE_BIT) && (features & SSE2_BIT)) {
        optimizedSkinRoutine = CalPhysique::calculateVerticesAndNormals_SSE;
    } else {
        optimizedSkinRoutine = CalPhysique::calculateVerticesAndNormals_x87;
    }
#endif
    return optimizedSkinRoutine(boneTransforms, vertexCount, vertices, influences, output_vertices);
}

void CalPhysique::calculateVerticesAndNormals(
    const BoneTransform* boneTransforms,
    const CalSubmesh* pSubmesh,
    float* pVertexBuffer
) {
    const int vertexCount = pSubmesh->getVertexCount();
    const CalCoreSubmesh::Vertex* vertices = Cal::pointerFromVector(pSubmesh->coreSubmesh->getVectorVertex());

    if (pSubmesh->getMorphTargetWeightCount() && pSubmesh->getBaseWeight() != 1.0f) {
        if (vertexCount > MorphSubmeshCache.size()) {
            MorphSubmeshCache.resize(vertexCount);
        }

        // get the sub morph target vector from the core sub mesh
        CalCoreSubmesh::CoreSubMorphTargetVector& vectorSubMorphTarget = pSubmesh->coreSubmesh->getVectorCoreSubMorphTarget();
        int morphTargetCount = pSubmesh->getMorphTargetWeightCount();
        EnlargeMiawCacheAsNecessary(morphTargetCount);
        unsigned int numMiaws;
        pSubmesh->getMorphIdAndWeightArray(MiawCache, & numMiaws, (unsigned int) morphTargetCount);

        // Fill MorphSubmeshCache w/ outputs of morph target calculation
        for (size_t vertexId = 0; vertexId < vertexCount; ++vertexId) {
            const CalCoreSubmesh::Vertex& sourceVertex = vertices[vertexId];

            float baseWeight = pSubmesh->getBaseWeight();
            CalVector position;
            CalVector normal;
            for (unsigned i = 0; i < numMiaws; i++) {
                MorphIdAndWeight& miaw = MiawCache[ i ];
                int morphTargetId = miaw.morphId_;
                CalCoreSubMorphTarget::BlendVertex const* blendVertex =
                    vectorSubMorphTarget[morphTargetId]->getBlendVertex(vertexId);
                float currentWeight = miaw.weight_;
                if (blendVertex) {
                    position.x += currentWeight * blendVertex->position.x;
                    position.y += currentWeight * blendVertex->position.y;
                    position.z += currentWeight * blendVertex->position.z;
                    normal.x += currentWeight * blendVertex->normal.x;
                    normal.y += currentWeight * blendVertex->normal.y;
                    normal.z += currentWeight * blendVertex->normal.z;
                } else {
                    baseWeight += currentWeight;
                }
            }
            position.x += baseWeight * sourceVertex.position.x;
            position.y += baseWeight * sourceVertex.position.y;
            position.z += baseWeight * sourceVertex.position.z;
            normal.x += baseWeight * sourceVertex.normal.x;
            normal.y += baseWeight * sourceVertex.normal.y;
            normal.z += baseWeight * sourceVertex.normal.z;

            CalCoreSubmesh::Vertex& destVertex = MorphSubmeshCache[vertexId];
            destVertex.position.setAsPoint(position);
            destVertex.normal.setAsVector(normal);
        }

        vertices = MorphSubmeshCache.data;
    }

    return optimizedSkinRoutine(
               boneTransforms,
               vertexCount,
               vertices,
               Cal::pointerFromVector(pSubmesh->coreSubmesh->getInfluences()),
               reinterpret_cast<CalVector4*>(pVertexBuffer));
}
