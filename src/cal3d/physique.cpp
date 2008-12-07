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

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

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

//I never thought I'd actually get a chance to use this :) --dusty
//http://www.lomont.org/Math/Papers/2003/InvSqrt.pdf
//http://www.math.purdue.edu/~clomont/Math/Papers/2003/InvSqrt.pdf
float FastInvSqrt(float x) {
    float xhalf = 0.5f*x;
    int i = *(int*)&x;      // get bits for floating value
    i = 0x5f375a86- (i>>1); // gives initial guess y0
    x = *(float*)&i;        // convert bits back to float
    x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
    return x;
}

void calculateVerticesAndNormals(
  const CalSkeleton::BoneTransform* boneTransforms,
  int vertexCount,
  const CalCoreSubmesh::Vertex* vertices,
  const CalCoreSubmesh::Influence* influences,
  float* output_buffer
) {
  // calculate all submesh vertices
  for(int vertexId = 0; vertexId < vertexCount; ++vertexId)
  {
    const CalCoreSubmesh::Vertex& vertex = vertices[vertexId];

    // initialize vertex
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    // initialize normal
    float nx = 0.0f;
    float ny = 0.0f;
    float nz = 0.0f;

    for(
      int influenceId = vertex.influenceStart;
      influenceId < vertex.influenceEnd;
      ++influenceId
    ) {
      // get the influence
      const CalCoreSubmesh::Influence& influence = influences[influenceId];
      const CalSkeleton::BoneTransform& boneTransform = boneTransforms[influence.boneId];
      
      // transform vertex with current state of the bone
      CalVector v(vertex.position);
      transform(v, boneTransform.colx, boneTransform.coly, boneTransform.colz);
      v.x += boneTransform.translation.x;
      v.y += boneTransform.translation.y;
      v.z += boneTransform.translation.z;
      
      x += influence.weight * v.x;
      y += influence.weight * v.y;
      z += influence.weight * v.z;
      
      // transform normal with current state of the bone
      CalVector n(vertex.normal);
      transform(n, boneTransform.colx, boneTransform.coly, boneTransform.colz);
      
      nx += influence.weight * n.x;
      ny += influence.weight * n.y;
      nz += influence.weight * n.z;
    }

    output_buffer[0] = x;
    output_buffer[1] = y;
    output_buffer[2] = z;
    //output_buffer[3] = 0.0f;
    
    output_buffer[4] = nx;
    output_buffer[5] = ny;
    output_buffer[6] = nz;
    //output_buffer[7] = 0.0f;

    output_buffer += 8;
  }
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
      CalVector& position = destVertex.position;
      CalVector& normal = destVertex.normal;

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
    pVertexBuffer);
}
