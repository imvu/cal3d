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
void
EnlargeMiawCacheAsNecessary( unsigned int numElements )
{
  if( MiawCacheNumElements < numElements ) {
    if( MiawCache ) {
      delete [] MiawCache;
    }

    // Step up exponentially to reduce number of steps.
    MiawCacheNumElements = numElements * 2;
    MiawCache = new( MorphIdAndWeight [ MiawCacheNumElements ] );
  }
}

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

 /*****************************************************************************/
/** Calculates the transformed vertex data.
  *
  * This function calculates and returns the transformed vertex and the transformed 
  * normal datadata of a specific submesh.
  *
  * @param pSubmesh A pointer to the submesh from which the vertex data should
  *                 be calculated and returned.
  * @param pVertexBuffer A pointer to the user-provided buffer where the vertex
  *                      data is written to.
  *
  * @return The number of vertices written to the buffer.
  *****************************************************************************/

int CalPhysique::calculateVerticesAndNormals(CalModel* model, CalSubmesh *pSubmesh, float *pVertexBuffer) {
  // get bone vector of the skeleton
  CalSkeleton* skeleton = model->getSkeleton();
  const CalSkeleton::BoneTransform* boneTransforms = Cal::pointerFromVector(skeleton->boneTransforms); 

  // get vertex vector of the core submesh
  std::vector<CalCoreSubmesh::Vertex>& vectorVertex = pSubmesh->getCoreSubmesh()->getVectorVertex();

  // get the number of vertices
  int vertexCount = pSubmesh->getVertexCount();

  // get the sub morph target vector from the core sub mesh
  CalCoreSubmesh::CoreSubMorphTargetVector& vectorSubMorphTarget = pSubmesh->getCoreSubmesh()->getVectorCoreSubMorphTarget();
  int morphTargetCount = pSubmesh->getMorphTargetWeightCount();
  EnlargeMiawCacheAsNecessary( morphTargetCount );
  unsigned int numMiaws;
  pSubmesh->getMorphIdAndWeightArray( MiawCache, & numMiaws, ( unsigned int ) morphTargetCount );

  // calculate the base weight
  float baseWeight2 = pSubmesh->getBaseWeight();

  // calculate all submesh vertices
  int vertexId;
  for(vertexId = 0; vertexId < vertexCount; ++vertexId)
  {
    CalCoreSubmesh::Vertex& vertex = vectorVertex[vertexId];

    // blend the morph targets
    CalVector position(0,0,0);
    CalVector normal(0,0,0);
    if(baseWeight2 == 1.0f)
    {
      position.x = vertex.position.x;
      position.y = vertex.position.y;
      position.z = vertex.position.z;
      normal.x = vertex.normal.x;
      normal.y = vertex.normal.y;
      normal.z = vertex.normal.z;
    }
    else
    {
      float baseWeight = baseWeight2;
      position.x = 0;
      position.y = 0;
      position.z = 0;
      normal.x = 0;
      normal.y = 0;
      normal.z = 0;
      unsigned int i;
      for( i = 0; i < numMiaws; i++ ) {
        MorphIdAndWeight * miaw = & MiawCache[ i ];
        int morphTargetId = miaw->morphId_;
        CalCoreSubMorphTarget::BlendVertex const * blendVertex =
          vectorSubMorphTarget[morphTargetId]->getBlendVertex(vertexId);
        float currentWeight = miaw->weight_;
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
      position.x += baseWeight*vertex.position.x;
      position.y += baseWeight*vertex.position.y;
      position.z += baseWeight*vertex.position.z;
      normal.x += baseWeight*vertex.normal.x;
      normal.y += baseWeight*vertex.normal.y;
      normal.z += baseWeight*vertex.normal.z;
    }
    
    // initialize vertex
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    // initialize normal
    float nx = 0.0f;
    float ny = 0.0f;
    float nz = 0.0f;

    // blend together all vertex influences
    int influenceCount=(int)vertex.vectorInfluence.size();
    for(int influenceId = 0; influenceId < influenceCount; ++influenceId)
    {
      // get the influence
      const CalCoreSubmesh::Influence& influence = vertex.vectorInfluence[influenceId];
      const CalSkeleton::BoneTransform& boneTransform = boneTransforms[influence.boneId];
      
      // transform vertex with current state of the bone
      CalVector v(position);
      transform(v, boneTransform.matrix);
      v += boneTransform.translation;
      
      x += influence.weight * v.x;
      y += influence.weight * v.y;
      z += influence.weight * v.z;
      
      // transform normal with current state of the bone
      CalVector n(normal);
      transform(n, boneTransform.matrix);
      
      nx += influence.weight * n.x;
      ny += influence.weight * n.y;
      nz += influence.weight * n.z;
    }

    pVertexBuffer[0] = x;
    pVertexBuffer[1] = y;
    pVertexBuffer[2] = z;
    
    pVertexBuffer[3] = nx;
    pVertexBuffer[4] = ny;
    pVertexBuffer[5] = nz;

    pVertexBuffer += 6;
  }

  return vertexCount;
}
