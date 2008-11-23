//****************************************************************************//
// coresubmesh.h                                                              //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <boost/shared_ptr.hpp>
#include "cal3d/color.h"
#include "cal3d/global.h"
#include "cal3d/vector.h"

class CalCoreSubMorphTarget;


enum CalMorphTargetType {
  CalMorphTargetTypeNull = 0,
  CalMorphTargetTypeAdditive,
  CalMorphTargetTypeClamped,
  CalMorphTargetTypeAverage,
  CalMorphTargetTypeExclusive
};

class CAL3D_API CalCoreSubmesh
{
public:
  /// The core submesh TextureCoordinate.
  struct TextureCoordinate
  {
    float u, v;
  };

  /// The core submesh Influence.
  struct Influence
  {
    int boneId;
    float weight;
  };

  /// The core submesh Vertex.
  struct Vertex
  {
    CalVector position;
    CalVector normal;
    std::vector<Influence> vectorInfluence;
    int collapseId;
    int faceCollapseCount;
  };

  struct Face {
      CalIndex vertexId[3];
  };
  
  typedef std::vector<boost::shared_ptr<CalCoreSubMorphTarget> > CoreSubMorphTargetVector;
  typedef std::vector<Face> VectorFace;
  typedef std::vector<TextureCoordinate> VectorTextureCoordinate;
  typedef std::vector<VectorTextureCoordinate > VectorVectorTextureCoordinate;
  typedef std::vector<Vertex> VectorVertex;
  typedef std::vector<Influence> VectorInfluence;

  CalCoreSubmesh();

  unsigned int size();
  unsigned int sizeWithoutSubMorphTargets();
  int getCoreMaterialThreadId();
  int getFaceCount();
  int getLodCount();
  bool hasNonWhiteVertexColors() { return m_hasNonWhiteVertexColors; }
  std::vector<Face>& getVectorFace();
  std::vector<std::vector<TextureCoordinate> >& getVectorVectorTextureCoordinate();

  std::vector<Vertex>& getVectorVertex();
  std::vector<CalColor32>& getVertexColors();

  int getVertexCount();
  void reserve(int vertexCount, int textureCoordinateCount, int faceCount);
  void setCoreMaterialThreadId(int coreMaterialThreadId);
  bool setFace(int faceId, const Face& face);
  void setLodCount(int lodCount);

  void setVertex(int vertexId, const Vertex& vertex, CalColor32 vertexColor);
  bool setTextureCoordinate(int vertexId, int textureCoordinateId, const TextureCoordinate& textureCoordinate);

  void setHasNonWhiteVertexColors( bool p ) { m_hasNonWhiteVertexColors = p; }
  int addCoreSubMorphTarget(boost::shared_ptr<CalCoreSubMorphTarget> pCoreSubMorphTarget);
  boost::shared_ptr<CalCoreSubMorphTarget> getCoreSubMorphTarget(int id);
  int getCoreSubMorphTargetCount();
  CoreSubMorphTargetVector& getVectorCoreSubMorphTarget();
  void scale(float factor);
  void setSubMorphTargetGroupIndexArray( unsigned int len, unsigned int const * indexArray );
  inline unsigned int subMorphTargetGroupIndex( int subMorphTargetId ) { 
    if( size_t(subMorphTargetId) >= m_vectorSubMorphTargetGroupIndex.size() ) return 0xffffffff;
    return m_vectorSubMorphTargetGroupIndex[ subMorphTargetId ];
  }

private:
  // These two should always be the same size.
  std::vector<Vertex> m_vertices;
  std::vector<CalColor32> m_vertexColors;

  std::vector<std::vector<TextureCoordinate> > m_vectorvectorTextureCoordinate;
  std::vector<Face> m_vectorFace;
  CoreSubMorphTargetVector m_vectorCoreSubMorphTarget;
  int m_coreMaterialThreadId;
  int m_lodCount;
  std::vector<unsigned int> m_vectorSubMorphTargetGroupIndex;
  bool m_hasNonWhiteVertexColors;
};
