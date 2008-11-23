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
  r += sizeof(Vertex) * m_vertices.size();
  r += sizeof(CalColor32) * m_vertexColors.size();
  r += sizeof(Face) * m_vectorFace.size();
  r += sizeof(unsigned int) * m_vectorSubMorphTargetGroupIndex.size();
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

std::vector<CalCoreSubmesh::Face>& CalCoreSubmesh::getVectorFace()
{
  return m_vectorFace;
}

std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> > & CalCoreSubmesh::getVectorVectorTextureCoordinate()
{
  return m_vectorvectorTextureCoordinate;
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
  return m_vertices;
}

std::vector<CalColor32>& CalCoreSubmesh::getVertexColors()
{
  return m_vertexColors;
}

int CalCoreSubmesh::getVertexCount()
{
  return m_vertices.size();
}

void CalCoreSubmesh::reserve(int vertexCount, int textureCoordinateCount, int faceCount)
{
  // reserve the space needed in all the vectors
  m_vertices.resize(vertexCount);
  m_vertexColors.resize(vertexCount);
  m_vectorvectorTextureCoordinate.resize(textureCoordinateCount);

  for(int textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; ++textureCoordinateId)
  {
    m_vectorvectorTextureCoordinate[textureCoordinateId].resize(vertexCount);
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

void CalCoreSubmesh::setVertex(int vertexId, const Vertex& vertex, CalColor32 vertexColor) {
  m_vertices[vertexId] = vertex;
  m_vertexColors[vertexId] = vertexColor;
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
    for(int vertexId = 0; vertexId < m_vertices.size() ; vertexId++)
    {
        m_vertices[vertexId].position*=factor;		
    }
}
