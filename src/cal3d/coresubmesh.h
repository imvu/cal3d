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

#include <set>
#include <map>
#include <boost/shared_ptr.hpp>
#include "cal3d/color.h"
#include "cal3d/global.h"
#include "cal3d/vector.h"
#include "cal3d/vector4.h"

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
  struct TextureCoordinate
  {
    float u, v;
  };

  struct Influence
  {
    int boneId;
    float weight;

    bool operator<(const Influence& rhs) const {
      return std::make_pair(boneId, quantize(weight)) < std::make_pair(rhs.boneId, quantize(rhs.weight));
    }

    // floating point equivalence is tricky
    static int quantize(float f) {
      return static_cast<int>(f * 10000.0);
    }
  };

  struct InfluenceSet
  {
    std::set<Influence> influences;

    //bool operator<(const InfluenceSet& rhs) const {
    //}
  };

  __declspec(align(16)) struct Vertex
  {
    CalVector4 position;
    CalVector4 normal;
    unsigned influenceStart;
    unsigned influenceEnd;
  };

  struct Face
  {
    CalIndex vertexId[3];
  };

  struct LodData
  {
    int collapseId;
    int faceCollapseCount;
  };
  
  typedef std::vector<boost::shared_ptr<CalCoreSubMorphTarget> > CoreSubMorphTargetVector;
  typedef std::vector<Face> VectorFace;
  typedef std::vector<TextureCoordinate> VectorTextureCoordinate;
  typedef std::vector<VectorTextureCoordinate > VectorVectorTextureCoordinate;
  typedef SSEArray<Vertex> VectorVertex;
  typedef std::vector<Influence> VectorInfluence;

  CalCoreSubmesh();

  unsigned int size();
  unsigned int sizeWithoutSubMorphTargets();
  int getCoreMaterialThreadId();
  int getFaceCount();
  int getLodCount();
  bool hasNonWhiteVertexColors() { return m_hasNonWhiteVertexColors; }
  const std::vector<Face>& getVectorFace() const;
  std::vector<std::vector<TextureCoordinate> >& getVectorVectorTextureCoordinate();

  const SSEArray<Vertex>& getVectorVertex() const {
    return m_vertices;
  }
  std::vector<CalColor32>& getVertexColors();
  std::vector<LodData>& getLodData() { return m_lodData; }

  int getVertexCount();
  void reserve(int vertexCount, int textureCoordinateCount, int faceCount);
  void setCoreMaterialThreadId(int coreMaterialThreadId);
  bool setFace(int faceId, const Face& face);
  void setLodCount(int lodCount);

  void setVertex(int vertexId, const Vertex& vertex, CalColor32 vertexColor, const LodData& lodData, const std::vector<Influence>& influences);
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

  std::vector<Influence> influences;

private:
  // The following arrays should always be the same size.
  SSEArray<Vertex> m_vertices;
  std::vector<CalColor32> m_vertexColors;
  std::vector<LodData> m_lodData;

  std::vector<std::vector<TextureCoordinate> > m_vectorvectorTextureCoordinate;
  std::vector<Face> m_vectorFace;

  std::map<InfluenceSet, size_t> m_influenceSetIds;

  CoreSubMorphTargetVector m_vectorCoreSubMorphTarget;
  int m_coreMaterialThreadId;
  int m_lodCount;
  std::vector<unsigned int> m_vectorSubMorphTargetGroupIndex;
  bool m_hasNonWhiteVertexColors;
};
