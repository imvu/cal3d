//****************************************************************************//
// coresubmesh.cpp                                                            //
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

#include "cal3d/coresubmesh.h"
#include "cal3d/coresubmorphtarget.h"

 /*****************************************************************************/
/** Constructs the core submesh instance.
  *
  * This function is the default constructor of the core submesh instance.
  *****************************************************************************/

CalCoreSubmesh::CalCoreSubmesh()
  : m_coreMaterialThreadId(0), m_lodCount(0)
{
  m_hasNonWhiteVertexColors = false;
}

 /*****************************************************************************/
/** Destructs the core submesh instance.
  *
  * This function is the destructor of the core submesh instance.
  *****************************************************************************/

void
CalCoreSubmesh::setSubMorphTargetGroupIndexArray( unsigned int len, unsigned int const * indexArray )
{
  m_vectorSubMorphTargetGroupIndex.reserve( len );
  m_vectorSubMorphTargetGroupIndex.resize( len );
  unsigned int i;
  for( i = 0; i < len; i++ ) {
    m_vectorSubMorphTargetGroupIndex[ i ] = indexArray[ i ];
  }
}


unsigned int
CalCoreSubmesh::sizeWithoutSubMorphTargets()
{
  unsigned int r = sizeof( CalCoreSubmesh );
  r += sizeof( Vertex ) * m_vectorVertex.size();
  r += sizeof( bool ) * m_vectorTangentsEnabled.size();
  r += sizeof( Face ) * m_vectorFace.size();
  r += sizeof( unsigned int ) * m_vectorSubMorphTargetGroupIndex.size();
  std::vector<std::vector<TangentSpace> >::iterator iter2;
  for( iter2 = m_vectorvectorTangentSpace.begin(); iter2 != m_vectorvectorTangentSpace.end(); ++iter2 ) {
    r += sizeof( TangentSpace ) * (*iter2).size();
  }
  std::vector<std::vector<TextureCoordinate> >::iterator iter3;
  for( iter3 = m_vectorvectorTextureCoordinate.begin(); iter3 != m_vectorvectorTextureCoordinate.end(); ++iter3 ) {
    r += sizeof( TextureCoordinate ) * (*iter3).size();
  }
  return r;
}


unsigned int
CalCoreSubmesh::size()
{
  unsigned int r = sizeWithoutSubMorphTargets();
  CoreSubMorphTargetVector::iterator iter1;
  for( iter1 = m_vectorCoreSubMorphTarget.begin(); iter1 != m_vectorCoreSubMorphTarget.end(); ++iter1 ) {
    r += (*iter1)->size();
  }
  return r;
}


 /*****************************************************************************/
/** Returns the ID of the core material thread.
  *
  * This function returns the ID of the core material thread of this core
  * submesh instance.
  *
  * @return The ID of the core material thread.
  *****************************************************************************/

int CalCoreSubmesh::getCoreMaterialThreadId()
{
  return m_coreMaterialThreadId;
}

 /*****************************************************************************/
/** Returns the number of faces.
  *
  * This function returns the number of faces in the core submesh instance.
  *
  * @return The number of faces.
  *****************************************************************************/

int CalCoreSubmesh::getFaceCount()
{
  return m_vectorFace.size();
}

 /*****************************************************************************/
/** Returns the number of LOD steps.
  *
  * This function returns the number of LOD steps in the core submesh instance.
  *
  * @return The number of LOD steps.
  *****************************************************************************/

int CalCoreSubmesh::getLodCount()
{
  return m_lodCount;
}

 /*****************************************************************************/
/** Returns true if tangent vectors are enabled.
  *
  * This function returns true if the core submesh contains tangent vectors.
  *
  * @return True if tangent vectors are enabled.
  *****************************************************************************/

bool CalCoreSubmesh::isTangentsEnabled(int mapId)
{
  if((mapId < 0) || (mapId >= (int)m_vectorTangentsEnabled.size())) return false;

  return m_vectorTangentsEnabled[mapId];
}


 /*****************************************************************************/
/** UpdateTangentVector
  *
  *****************************************************************************/


void CalCoreSubmesh::UpdateTangentVector(int v0, int v1, int v2, int mapId)
{
  std::vector<CalCoreSubmesh::Vertex> &vvtx = getVectorVertex();
  std::vector<CalCoreSubmesh::TextureCoordinate> &vtex = m_vectorvectorTextureCoordinate[mapId];

  // Step 1. Compute the approximate tangent vector.
  double du1 = vtex[v1].u - vtex[v0].u;
  double dv1 = vtex[v1].v - vtex[v0].v;
  double du2 = vtex[v2].u - vtex[v0].u;
  double dv2 = vtex[v2].v - vtex[v0].v;

  double prod1 = (du1*dv2-dv1*du2);
  double prod2 = (du2*dv1-dv2*du1);
  if ((fabs(prod1) < 0.000001)||(fabs(prod2) < 0.000001)) return;

  double x = dv2/prod1;
  double y = dv1/prod2;

  CalVector vec1 = vvtx[v1].position - vvtx[v0].position;
  CalVector vec2 = vvtx[v2].position - vvtx[v0].position;
  CalVector tangent = (vec1 * ((float)x)) + (vec2 * ((float)y));

  // Step 2. Orthonormalize the tangent.
  double component = (tangent * vvtx[v0].normal);
  tangent -= (vvtx[v0].normal * ((float)component));
  tangent.normalize();

  // Step 3: Add the estimated tangent to the overall estimate for the vertex.


  m_vectorvectorTangentSpace[mapId][v0].tangent+=tangent;
}


 /*****************************************************************************/
/** Enables (and calculates) or disables the storage of tangent spaces.
  *
  * This function enables or disables the storage of tangent space bases.
  *****************************************************************************/

bool CalCoreSubmesh::enableTangents(int mapId, bool enabled)
{
  if((mapId < 0) || (mapId >= (int)m_vectorTangentsEnabled.size())) return false;
  
  m_vectorTangentsEnabled[mapId] = enabled;

  if(!enabled)
  {
    m_vectorvectorTangentSpace[mapId].clear();
    return true;
  }

  m_vectorvectorTangentSpace[mapId].reserve(m_vectorVertex.size());
  m_vectorvectorTangentSpace[mapId].resize(m_vectorVertex.size());

  int tangentId;
  for(tangentId=0;tangentId< (int)m_vectorvectorTangentSpace[mapId].size();tangentId++)
  {
    m_vectorvectorTangentSpace[mapId][tangentId].tangent= CalVector(0.0f,0.0f,0.0f);
    m_vectorvectorTangentSpace[mapId][tangentId].crossFactor=1;

  }


  int faceId;
  for(faceId=0;faceId<(int)m_vectorFace.size();faceId++)
  {
    UpdateTangentVector(m_vectorFace[faceId].vertexId[0],m_vectorFace[faceId].vertexId[1],m_vectorFace[faceId].vertexId[2],mapId);
    UpdateTangentVector(m_vectorFace[faceId].vertexId[1],m_vectorFace[faceId].vertexId[2],m_vectorFace[faceId].vertexId[0],mapId);
    UpdateTangentVector(m_vectorFace[faceId].vertexId[2],m_vectorFace[faceId].vertexId[0],m_vectorFace[faceId].vertexId[1],mapId);
  }

  for(tangentId=0;tangentId< (int)m_vectorvectorTangentSpace[mapId].size();tangentId++)
  {
    m_vectorvectorTangentSpace[mapId][tangentId].tangent.normalize();
  }
  
  return true;
}


 /*****************************************************************************/
/** Returns the face vector.
  *
  * This function returns the vector that contains all faces of the core submesh
  * instance.
  *
  * @return A reference to the face vector.
  *****************************************************************************/

std::vector<CalCoreSubmesh::Face>& CalCoreSubmesh::getVectorFace()
{
  return m_vectorFace;
}

std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> > & CalCoreSubmesh::getVectorVectorTextureCoordinate()
{
  return m_vectorvectorTextureCoordinate;
}

 /*****************************************************************************/
/** Returns the tangent space vector-vector.
  *
  * This function returns the vector that contains all tangent space bases of
  * the core submesh instance. This vector contains another vector
  * because there can be more than one texture map at each vertex.
  *
  * @return A reference to the tangent space vector-vector.
  *****************************************************************************/

std::vector<std::vector<CalCoreSubmesh::TangentSpace> >& CalCoreSubmesh::getVectorVectorTangentSpace()
{
  return m_vectorvectorTangentSpace;
}


 /*****************************************************************************/
/** Returns the vertex vector.
  *
  * This function returns the vector that contains all vertices of the core
  * submesh instance.
  *
  * @return A reference to the vertex vector.
  *****************************************************************************/

std::vector<CalCoreSubmesh::Vertex>& CalCoreSubmesh::getVectorVertex()
{
  return m_vectorVertex;
}

 /*****************************************************************************/
/** Returns the number of vertices.
  *
  * This function returns the number of vertices in the core submesh instance.
  *
  * @return The number of vertices.
  *****************************************************************************/

int CalCoreSubmesh::getVertexCount()
{
  return m_vectorVertex.size();
}

 /*****************************************************************************/
/** Reserves memory for the vertices, faces and texture coordinates.
  *
  * This function reserves memory for the vertices, faces, texture coordinates
  * and springs of the core submesh instance.
  *
  * @param vertexCount The number of vertices that this core submesh instance
  *                    should be able to hold.
  * @param textureCoordinateCount The number of texture coordinates that this
  *                               core submesh instance should be able to hold.
  * @param faceCount The number of faces that this core submesh instance should
  *                  be able to hold.
  * @param springCount The number of springs that this core submesh instance
  *                  should be able to hold.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

void CalCoreSubmesh::reserve(int vertexCount, int textureCoordinateCount, int faceCount)
{
  // reserve the space needed in all the vectors
  m_vectorVertex.resize(vertexCount);

  m_vectorTangentsEnabled.resize(textureCoordinateCount);

  m_vectorvectorTangentSpace.resize(textureCoordinateCount);

  m_vectorvectorTextureCoordinate.resize(textureCoordinateCount);

  int textureCoordinateId;
  for(textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; ++textureCoordinateId)
  {
    m_vectorvectorTextureCoordinate[textureCoordinateId].resize(vertexCount);
	
    if (m_vectorTangentsEnabled[textureCoordinateId])
    {
      m_vectorvectorTangentSpace[textureCoordinateId].resize(vertexCount);
    }
    else
    {
      m_vectorvectorTangentSpace[textureCoordinateId].clear();
    }
  }

  m_vectorFace.resize(faceCount);
}

 /*****************************************************************************/
/** Sets the ID of the core material thread.
  *
  * This function sets the ID of the core material thread of the core submesh
  * instance.
  *
  * @param coreMaterialThreadId The ID of the core material thread that should
  *                             be set.
  *****************************************************************************/

void CalCoreSubmesh::setCoreMaterialThreadId(int coreMaterialThreadId)
{
  m_coreMaterialThreadId = coreMaterialThreadId;
}

 /*****************************************************************************/
/** Sets a specified face.
  *
  * This function sets a specified face in the core submesh instance.
  *
  * @param faceId  The ID of the face.
  * @param face The face that should be set.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setFace(int faceId, const Face& face)
{
  if((faceId < 0) || (faceId >= (int)m_vectorFace.size())) return false;

  m_vectorFace[faceId] = face;

  return true;
}

 /*****************************************************************************/
/** Sets the number of LOD steps.
  *
  * This function sets the number of LOD steps of the core submesh instance.
  *
  * @param lodCount The number of LOD steps that should be set.
  *****************************************************************************/

void CalCoreSubmesh::setLodCount(int lodCount)
{
  m_lodCount = lodCount;
}

 /*****************************************************************************/
/** Sets the tangent vector associated with a specified texture coordinate pair.
  *
  * This function sets the tangent vector associated with a specified
  * texture coordinate pair in the core submesh instance.
  *
  * @param vertexId  The ID of the vertex.
  * @param textureCoordinateId The ID of the texture coordinate channel.
  * @param tangent   The tangent vector that should be stored.
  * @param crossFactor The cross-product factor that should be stored.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setTangentSpace(int vertexId, int textureCoordinateId, const CalVector& tangent, float crossFactor)
{
  if((vertexId < 0) || (vertexId >= (int)m_vectorVertex.size())) return false;
  if((textureCoordinateId < 0) || (textureCoordinateId >= (int)m_vectorvectorTextureCoordinate.size())) return false;
  if(!m_vectorTangentsEnabled[textureCoordinateId]) return false;
  
  m_vectorvectorTangentSpace[textureCoordinateId][vertexId].tangent = tangent;
  m_vectorvectorTangentSpace[textureCoordinateId][vertexId].crossFactor = crossFactor;
  return true;
}


 /*****************************************************************************/
/** Sets a specified texture coordinate.
  *
  * This function sets a specified texture coordinate in the core submesh
  * instance.
  *
  * @param vertexId  The ID of the vertex.
  * @param textureCoordinateId  The ID of the texture coordinate.
  * @param textureCoordinate The texture coordinate that should be set.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setTextureCoordinate(int vertexId, int textureCoordinateId, const TextureCoordinate& textureCoordinate)
{
  if((textureCoordinateId < 0) || (textureCoordinateId >= (int)m_vectorvectorTextureCoordinate.size())) return false;
  if((vertexId < 0) || (vertexId >= (int)m_vectorvectorTextureCoordinate[textureCoordinateId].size())) return false;

  m_vectorvectorTextureCoordinate[textureCoordinateId][vertexId] = textureCoordinate;

  return true;
}

 /*****************************************************************************/
/** Sets a specified vertex.
  *
  * This function sets a specified vertex in the core submesh instance.
  *
  * @param vertexId  The ID of the vertex.
  * @param vertex The vertex that should be set.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubmesh::setVertex(int vertexId, const Vertex& vertex)
{
  if((vertexId < 0) || (vertexId >= (int)m_vectorVertex.size())) return false;

  m_vectorVertex[vertexId] = vertex;

  return true;
}

 /*****************************************************************************/
/** Adds a core sub morph target.
  *
  * This function adds a core sub morph target to the core sub mesh instance.
  *
  * @param pCoreSubMorphTarget A pointer to the core sub morph target that should be added.
  *
  * @return One of the following values:
  *         \li the assigned sub morph target \b ID of the added core sub morph target
  *         \li \b -1 if an error happend
  *****************************************************************************/

int CalCoreSubmesh::addCoreSubMorphTarget(boost::shared_ptr<CalCoreSubMorphTarget> pCoreSubMorphTarget)
{
  // get next sub morph target id
  int subMorphTargetId;
  subMorphTargetId = m_vectorCoreSubMorphTarget.size();

  m_vectorCoreSubMorphTarget.push_back(pCoreSubMorphTarget);

  return subMorphTargetId;
}

 /*****************************************************************************/
/** Provides access to a core sub morph target.
  *
  * This function returns the core sub morph target with the given ID.
  *
  * @param id The ID of the core sub morph target that should be returned.
  *
  * @return One of the following values:
  *         \li a pointer to the core sub morph target
  *         \li \b 0 if an error happend
  *****************************************************************************/

boost::shared_ptr<CalCoreSubMorphTarget> CalCoreSubmesh::getCoreSubMorphTarget(int id)
{
  if((id < 0) || (id >= (int)m_vectorCoreSubMorphTarget.size()))
  {
    // should assert
    return boost::shared_ptr<CalCoreSubMorphTarget>();
  }

  return m_vectorCoreSubMorphTarget[id];
}

 /*****************************************************************************/
/** Returns the number of core sub morph targets.
  *
  * This function returns the number of core sub morph targets in the core sub mesh
  * instance.
  *
  * @return The number of core sub morph targets.
  *****************************************************************************/

int CalCoreSubmesh::getCoreSubMorphTargetCount()
{
  return m_vectorCoreSubMorphTarget.size();
}

 /*****************************************************************************/
/** Returns the core sub morph target vector.
  *
  * This function returns the vector that contains all core sub morph target
  *  of the core submesh instance.
  *
  * @return A reference to the core sub morph target vector.
  *****************************************************************************/

CalCoreSubmesh::CoreSubMorphTargetVector& CalCoreSubmesh::getVectorCoreSubMorphTarget()
{
  return m_vectorCoreSubMorphTarget;
}

 /*****************************************************************************/
/** Scale the Submesh.
  *
  * This function rescale all the data that are in the core submesh instance.
  *
  * @param factor A float with the scale factor
  *
  *****************************************************************************/


void CalCoreSubmesh::scale(float factor)
{
	// rescale all vertices

	int vertexId;
	for(vertexId = 0; vertexId < m_vectorVertex.size() ; vertexId++)
	{
		m_vectorVertex[vertexId].position*=factor;		
	}
}
