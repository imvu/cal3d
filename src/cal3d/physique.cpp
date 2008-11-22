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

 /*****************************************************************************/
/** Constructs the physique instance.
  *
  * This function is the default constructor of the physique instance.
  *****************************************************************************/

CalPhysique::CalPhysique()
  : m_pModel(0)
{
}

 /*****************************************************************************/
/** Destructs the physique instance.
  *
  * This function is the destructor of the physique instance.
  *****************************************************************************/

CalPhysique::~CalPhysique()
{
}

 /*****************************************************************************/
/** Calculates the transformed vertex data.
  *
  * This function calculates and returns the transformed vertex data of a
  * specific submesh.
  *
  * @param pSubmesh A pointer to the submesh from which the vertex data should
  *                 be calculated and returned.
  * @param pVertexBuffer A pointer to the user-provided buffer where the vertex
  *                      data is written to.
  *
  * @return The number of vertices written to the buffer.
  *****************************************************************************/

int CalPhysique::calculateVertices(CalSubmesh *pSubmesh, float *pVertexBuffer)
{
  // get bone vector of the skeleton
  std::vector<CalBone *>& vectorBone = m_pModel->getSkeleton()->getVectorBone();

  // get vertex vector of the core submesh
  std::vector<CalCoreSubmesh::Vertex>& vectorVertex = pSubmesh->getCoreSubmesh()->getVectorVertex();

  // get the number of vertices
  int vertexCount;
  vertexCount = pSubmesh->getVertexCount();
  
  // get the sub morph target vector from the core sub mesh
  CalCoreSubmesh::CoreSubMorphTargetVector& vectorSubMorphTarget = pSubmesh->getCoreSubmesh()->getVectorCoreSubMorphTarget();

  // calculate the base weight
  float baseWeight = pSubmesh->getBaseWeight();

  // Get the number of morph targets and cache the weights in an array
  // that can be indexed quickly inside the vertex inner loop.
  int morphTargetCount = pSubmesh->getMorphTargetWeightCount();
  static int const morphTargetCountMax = 100; // Arbitrary.
  if( morphTargetCount > morphTargetCountMax ) {
    morphTargetCount = morphTargetCountMax;
  }
  static float morphTargetScaleArray[ morphTargetCountMax ];
  int i;
  for( i = 0; i < morphTargetCount; i++ ) {
    morphTargetScaleArray[ i ] = pSubmesh->getMorphTargetWeight( i ); 
  }

  // calculate all submesh vertices
  int vertexId;
  for(vertexId = 0; vertexId < vertexCount; ++vertexId)
  {
    // get the vertex
    CalCoreSubmesh::Vertex& vertex = vectorVertex[vertexId];
    
    // blend the morph targets
    CalVector position(0,0,0);
    if(baseWeight == 1.0f)
    {
       position.x = vertex.position.x;
       position.y = vertex.position.y;
       position.z = vertex.position.z;
    }
    else
    { 
      position.x = baseWeight*vertex.position.x;
      position.y = baseWeight*vertex.position.y;
      position.z = baseWeight*vertex.position.z;
      int morphTargetId;
      for(morphTargetId=0; morphTargetId < morphTargetCount;++morphTargetId)
      {
        CalCoreSubMorphTarget::BlendVertex const * blendVertex =
          vectorSubMorphTarget[morphTargetId]->getBlendVertex(vertexId);
        float morphScale = morphTargetScaleArray[ morphTargetId ];
        if( blendVertex ) {
          position.x += morphScale * blendVertex->position.x;
          position.y += morphScale * blendVertex->position.y;
          position.z += morphScale * blendVertex->position.z;
        } else {
          position.x += morphScale * vertex.position.x;
          position.y += morphScale * vertex.position.y;
          position.z += morphScale * vertex.position.z;
        }
      }
    }

    // initialize vertex
    float x, y, z;
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;

    // blend together all vertex influences
    int influenceId;
	int influenceCount=(int)vertex.vectorInfluence.size();
    for(influenceId = 0; influenceId < influenceCount; ++influenceId)
    {
      // get the influence
      CalCoreSubmesh::Influence& influence = vertex.vectorInfluence[influenceId];

      // get the bone of the influence vertex
      CalBone *pBone;
      pBone = vectorBone[influence.boneId];

      // transform vertex with current state of the bone
      CalVector v(position);
      v *= pBone->getTransformMatrix();
      v += pBone->getTranslationBoneSpace();

      x += influence.weight * v.x;
      y += influence.weight * v.y;
      z += influence.weight * v.z;
    }

    pVertexBuffer[0] = x;
    pVertexBuffer[1] = y;
    pVertexBuffer[2] = z;

    // next vertex position in buffer
    pVertexBuffer += 3;
  }

  return vertexCount;
}

 /*****************************************************************************/
/** Calculates the transformed tangent space data.
  *
  * This function calculates and returns the transformed tangent space data of a
  * specific submesh.
  *
  * @param pSubmesh A pointer to the submesh from which the tangent space data 
  *                 should be calculated and returned.
  * @param mapId
  * @param pTangentSpaceBuffer A pointer to the user-provided buffer where the tangent 
  *                 space data is written to.
  *
  * @return The number of tangent spaces written to the buffer.
  *****************************************************************************/

int CalPhysique::calculateTangentSpaces(CalSubmesh *pSubmesh, int mapId, float *pTangentSpaceBuffer)
{
  if((mapId < 0) || (mapId >= (int)pSubmesh->getCoreSubmesh()->getVectorVectorTangentSpace().size())) return false;

  // get bone vector of the skeleton
  std::vector<CalBone *>& vectorBone = m_pModel->getSkeleton()->getVectorBone();

  // get vertex vector of the submesh
  std::vector<CalCoreSubmesh::Vertex>& vectorVertex = pSubmesh->getCoreSubmesh()->getVectorVertex();

  // get tangent space vector of the submesh
  std::vector<CalCoreSubmesh::TangentSpace>& vectorTangentSpace = pSubmesh->getCoreSubmesh()->getVectorVectorTangentSpace()[mapId];
  
  // get the number of vertices
  int vertexCount;
  vertexCount = pSubmesh->getVertexCount();

  // calculate normal for all submesh vertices
  int vertexId;
  for(vertexId = 0; vertexId < vertexCount; vertexId++)
  {
    CalCoreSubmesh::TangentSpace& tangentSpace = vectorTangentSpace[vertexId];

    // get the vertex
    CalCoreSubmesh::Vertex& vertex = vectorVertex[vertexId];

    // initialize tangent
    float tx, ty, tz;
    tx = 0.0f;
    ty = 0.0f;
    tz = 0.0f;

    // blend together all vertex influences
    int influenceId;
    int influenceCount=(int)vertex.vectorInfluence.size();

    for(influenceId = 0; influenceId < influenceCount; influenceId++)
    {
      // get the influence
      CalCoreSubmesh::Influence& influence = vertex.vectorInfluence[influenceId];

      // get the bone of the influence vertex
      CalBone *pBone;
      pBone = vectorBone[influence.boneId];

      // transform normal with current state of the bone
      CalVector v(tangentSpace.tangent);
      v *= pBone->getTransformMatrix(); 

      tx += influence.weight * v.x;
      ty += influence.weight * v.y;
      tz += influence.weight * v.z;
    }

    pTangentSpaceBuffer[0] = tx;
    pTangentSpaceBuffer[1] = ty;
    pTangentSpaceBuffer[2] = tz;

    pTangentSpaceBuffer[3] = tangentSpace.crossFactor;
    // next vertex position in buffer
    pTangentSpaceBuffer += 4;
  }

  return vertexCount;
}


 /*****************************************************************************/
/** Calculates the transformed normal data.
  *
  * This function calculates and returns the transformed normal data of a
  * specific submesh.
  *
  * @param pSubmesh A pointer to the submesh from which the normal data should
  *                 be calculated and returned.
  * @param pNormalBuffer A pointer to the user-provided buffer where the normal
  *                      data is written to.
  *
  * @return The number of normals written to the buffer.
  *****************************************************************************/

int CalPhysique::calculateNormals(CalSubmesh *pSubmesh, float *pNormalBuffer)
{
  // get bone vector of the skeleton
  std::vector<CalBone *>& vectorBone = m_pModel->getSkeleton()->getVectorBone();

  // get vertex vector of the submesh
  std::vector<CalCoreSubmesh::Vertex>& vectorVertex = pSubmesh->getCoreSubmesh()->getVectorVertex();

  // get the number of vertices
  int vertexCount;
  vertexCount = pSubmesh->getVertexCount();

  // get the sub morph target vector from the core sub mesh
  CalCoreSubmesh::CoreSubMorphTargetVector& vectorSubMorphTarget = pSubmesh->getCoreSubmesh()->getVectorCoreSubMorphTarget();

  // calculate the base weight
  float baseWeight = pSubmesh->getBaseWeight();

  // get the number of morph targets
  int morphTargetCount = pSubmesh->getMorphTargetWeightCount();

  // calculate normal for all submesh vertices
  int vertexId;
  for(vertexId = 0; vertexId < vertexCount; ++vertexId)
  {
    // get the vertex
    CalCoreSubmesh::Vertex& vertex = vectorVertex[vertexId];

    // blend the morph targets
    CalVector normal(0,0,0);
    if(baseWeight == 1.0f)
    {
      normal.x = vertex.normal.x;
      normal.y = vertex.normal.y;
      normal.z = vertex.normal.z;
    }
    else
    {
      normal.x = baseWeight*vertex.normal.x;
      normal.y = baseWeight*vertex.normal.y;
      normal.z = baseWeight*vertex.normal.z;
      int morphTargetId;
      for(morphTargetId=0; morphTargetId < morphTargetCount;++morphTargetId)
      {
        CalCoreSubMorphTarget::BlendVertex const * blendVertex = 
        vectorSubMorphTarget[morphTargetId]->getBlendVertex(vertexId);
        float currentWeight = pSubmesh->getMorphTargetWeight(morphTargetId);
        if( blendVertex ) {
          normal.x += currentWeight*blendVertex->normal.x;
          normal.y += currentWeight*blendVertex->normal.y;
          normal.z += currentWeight*blendVertex->normal.z;
        } else {
          normal.x += currentWeight*vertex.normal.x;
          normal.y += currentWeight*vertex.normal.y;
          normal.z += currentWeight*vertex.normal.z;
        }
      }
    }

    // initialize normal
    float nx, ny, nz;
    nx = 0.0f;
    ny = 0.0f;
    nz = 0.0f;

    // blend together all vertex influences
    int influenceId;
	int influenceCount=(int)vertex.vectorInfluence.size();
    for(influenceId = 0; influenceId < influenceCount; ++influenceId)
    {
      // get the influence
      CalCoreSubmesh::Influence& influence = vertex.vectorInfluence[influenceId];

      // get the bone of the influence vertex
      CalBone *pBone;
      pBone = vectorBone[influence.boneId];

      // transform normal with current state of the bone
      CalVector v(normal);
      v *= pBone->getTransformMatrix(); 

      nx += influence.weight * v.x;
      ny += influence.weight * v.y;
      nz += influence.weight * v.z;
    }

    // re-normalize normal if necessary
    pNormalBuffer[0] = nx;
    pNormalBuffer[1] = ny;
    pNormalBuffer[2] = nz;

    // next vertex position in buffer
    pNormalBuffer += 3;
  }

  return vertexCount;
}


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

int CalPhysique::calculateVerticesAndNormals(CalSubmesh *pSubmesh, float *pVertexBuffer) {
  // get bone vector of the skeleton
  std::vector<CalBone *>& vectorBone = m_pModel->getSkeleton()->getVectorBone();

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
    float x, y, z;
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;

    // initialize normal
    float nx, ny, nz;
    nx = 0.0f;
    ny = 0.0f;
    nz = 0.0f;

    // blend together all vertex influences
    int influenceCount=(int)vertex.vectorInfluence.size();
    for(int influenceId = 0; influenceId < influenceCount; ++influenceId)
    {
      // get the influence
      CalCoreSubmesh::Influence& influence = vertex.vectorInfluence[influenceId];
      
      // get the bone of the influence vertex
      CalBone *pBone;
      pBone = vectorBone[influence.boneId];
      
      // transform vertex with current state of the bone
      CalVector v(position);
      v *= pBone->getTransformMatrix();
      v += pBone->getTranslationBoneSpace();
      
      x += influence.weight * v.x;
      y += influence.weight * v.y;
      z += influence.weight * v.z;
      
      // transform normal with current state of the bone
      CalVector n(normal);
      n *= pBone->getTransformMatrix();
      
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


 /*****************************************************************************/
/** Creates the physique instance.
  *
  * This function creates the physique instance.
  *
  * @param pModel A pointer to the model that should be managed with this
  *               physique instance.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

void CalPhysique::create(CalModel *pModel)
{
  assert(pModel);

  m_pModel = pModel;

  CalCoreModel *pCoreModel;
  pCoreModel = m_pModel->getCoreModel();
  assert(pCoreModel);
}

 /*****************************************************************************/
/** Destroys the physique instance.
  *
  * This function destroys all data stored in the physique instance and frees
  * all allocated memory.
  *****************************************************************************/

void CalPhysique::destroy()
{
  m_pModel = 0;
}
