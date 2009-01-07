//****************************************************************************//
// coresubmorphtarget.cpp                                                     //
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

#include "cal3d/coresubmorphtarget.h"

CalCoreSubMorphTarget::CalCoreSubMorphTarget()
{
  m_morphTargetType = CalMorphTargetTypeAdditive;
}

CalCoreSubMorphTarget::~CalCoreSubMorphTarget()
{
  // destroy all data
  for( int i = 0; i < m_vectorBlendVertex.size(); i++ ) {
    if( m_vectorBlendVertex[i] ) {
      delete m_vectorBlendVertex[i];
      m_vectorBlendVertex[i] = NULL;
    }
  }
  m_vectorBlendVertex.clear();
}

unsigned int
CalCoreSubMorphTarget::size()
{
  unsigned int r = sizeof( CalCoreSubMorphTarget );
  r += sizeof( CalMorphTargetType );

  // Assume single texture coordinate pair.
  r += ( sizeof( BlendVertex ) + sizeof( CalCoreSubmesh::TextureCoordinate ) ) * m_vectorBlendVertex.size();
  r += m_morphTargetName.size();
  return r;
}


 /*****************************************************************************/
/** Returns the blend vertex vector.
  *
  * This function returns the vector that contains all blend vertices of the core
  * sub morph target instance.
  *
  * @return A reference to the blend vertex vector.
  *****************************************************************************/

std::vector<CalCoreSubMorphTarget::BlendVertex *>& CalCoreSubMorphTarget::getVectorBlendVertex()
{
  return m_vectorBlendVertex;
}


 /*****************************************************************************/
/** Returns the number of blend vertices.
  *
  * This function returns the number of blend vertices in the 
  * core sub morph target instance.
  *
  * @return The number of blend vertices.
  *****************************************************************************/

int CalCoreSubMorphTarget::getBlendVertexCount()
{
  return m_vectorBlendVertex.size();
}


 /*****************************************************************************/
/** Reserves memory for the blend vertices.
 *
 * This function reserves memory for the blend vertices
 * of the core sub morph target instance.
 *
 * @param blendVertexCount The number of blend vertices that
 *                    this core sub morph target instance should be able to hold.
 *
 * @return One of the following values:
 *         \li \b true if successful
 *         \li \b false if an error happend
 *****************************************************************************/

void CalCoreSubMorphTarget::reserve(int blendVertexCount)
{
  // reserve the space needed in all the vectors
  m_vectorBlendVertex.reserve(blendVertexCount);
  m_vectorBlendVertex.resize(blendVertexCount);
  for( int i = 0; i < m_vectorBlendVertex.size(); i++ ) {
    m_vectorBlendVertex[i] = NULL;
  }
}



 /*****************************************************************************/
/** Sets a specified blend vertex.
  *
  * This function sets a specified blend vertex in the core sub morph target instance.
  *
  * @param vertexId  The ID of the vertex.
  * @param vertex The vertex that should be set.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreSubMorphTarget::setBlendVertex(int blendVertexId, const BlendVertex& blendVertex)
{
  if((blendVertexId < 0) || (blendVertexId >= (int)m_vectorBlendVertex.size())) return false;

  if( m_vectorBlendVertex[blendVertexId] == NULL ) {
    m_vectorBlendVertex[blendVertexId] = new BlendVertex();
  }
  m_vectorBlendVertex[blendVertexId]->position = blendVertex.position;
  m_vectorBlendVertex[blendVertexId]->normal = blendVertex.normal;
  m_vectorBlendVertex[blendVertexId]->textureCoords.clear();
  m_vectorBlendVertex[blendVertexId]->textureCoords.reserve(blendVertex.textureCoords.size());
  for( int tcI = 0; tcI < blendVertex.textureCoords.size(); tcI++ ) {
    m_vectorBlendVertex[blendVertexId]->textureCoords.push_back(blendVertex.textureCoords[tcI]);
  }

  return true;
}


 /*****************************************************************************/
/** Return type of morph target, which is determined from parsing the name.
  *
  * @return One of the enum values.
  *****************************************************************************/
CalMorphTargetType
CalCoreSubMorphTarget::morphTargetType()
{
  return m_morphTargetType;
}


//****************************************************************************//

void
CalCoreSubMorphTarget::setName( std::string s )
{
  m_morphTargetName = s;

  // If the name ends in ".Additive" or ".Exclusive" or ".Clamped"
  // or ".Average" then set the type of the morph target.  By
  // default it is Additive.
  m_morphTargetType = CalMorphTargetTypeAdditive;
  char const * s2 = s.c_str();
  char const * dot = strrchr( s2, '.' );
  if( dot ) {
    dot++;
    if( _stricmp( dot, "exclusive" ) == 0 ) {
      m_morphTargetType = CalMorphTargetTypeExclusive;
    } else if( _stricmp( dot, "additive" ) == 0 ) {
      m_morphTargetType = CalMorphTargetTypeAdditive;
    } else if( _stricmp( dot, "clamped" ) == 0 ) {
      m_morphTargetType = CalMorphTargetTypeClamped;
    } else if( _stricmp( dot, "average" ) == 0 ) {
      m_morphTargetType = CalMorphTargetTypeAverage;
    }
  }
}
