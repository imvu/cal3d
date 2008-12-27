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
    Influence() {
      boneId = -1;
      weight = 0.0f;
      lastInfluenceForThisVertex = 0;
    }

    Influence(unsigned b, float w, bool last) {
      boneId = b;
      weight = w;
      lastInfluenceForThisVertex = last ? 1 : 0;
    }

    unsigned boneId;
    float weight;
    unsigned lastInfluenceForThisVertex;

    bool operator==(const Influence& rhs) const {
      return std::make_pair(boneId, quantize(weight)) == std::make_pair(rhs.boneId, quantize(rhs.weight));
    }

    bool operator<(const Influence& rhs) const {
      return std::make_pair(boneId, quantize(weight)) < std::make_pair(rhs.boneId, quantize(rhs.weight));
    }

    // floating point equivalence is tricky
    static int quantize(float f) {
      return static_cast<int>(f * 10000.0);
    }
  };

  struct InfluenceSet {
    InfluenceSet() {
    }

    InfluenceSet(const std::vector<Influence>& vi)
    : influences(vi.begin(), vi.end()) {
    }

    std::set<Influence> influences;

    bool operator==(const InfluenceSet& rhs) const {
      if (influences.size() != rhs.influences.size()) {
        return false;
      }
      std::set<Influence>::const_iterator begin1 = influences.begin();
      std::set<Influence>::const_iterator begin2 = rhs.influences.begin();

      while (begin1 != influences.end()) {
        if (!(*begin1 == *begin2)) {
          return false;
        }
        ++begin1;
        ++begin2;
      }

      return true;
    }
  };

  __declspec(align(16)) struct Vertex
  {
    CalPoint4 position;
    CalVector4 normal;
  };

  struct InfluenceRange
  {
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
  typedef std::vector<Influence> InfluenceVector;

  CalCoreSubmesh(int vertexCount, int textureCoordinateCount, int faceCount);

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
  void setCoreMaterialThreadId(int coreMaterialThreadId);
  bool setFace(int faceId, const Face& face);
  void setLodCount(int lodCount);

  void addVertex(const Vertex& vertex, CalColor32 vertexColor, const LodData& lodData, std::vector<Influence> influences);
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

  const InfluenceRange& getInfluenceRange(size_t vertexId) {
    return m_influenceRanges[vertexId];
  }

  bool isStatic() const {
    return m_isStatic;
  }

  const InfluenceVector& getInfluences() const {
    return influences;
  }

private:
  int m_currentVertexId;

  // The following arrays should always be the same size.
  SSEArray<Vertex> m_vertices;
  std::vector<CalColor32> m_vertexColors;
  std::vector<LodData> m_lodData;
  std::vector<InfluenceRange> m_influenceRanges;

  std::vector<std::vector<TextureCoordinate> > m_vectorvectorTextureCoordinate;
  std::vector<Face> m_vectorFace;

  std::map<InfluenceSet, size_t> m_influenceSetIds;

  CoreSubMorphTargetVector m_vectorCoreSubMorphTarget;
  int m_coreMaterialThreadId;
  int m_lodCount;
  std::vector<unsigned int> m_vectorSubMorphTargetGroupIndex;
  bool m_hasNonWhiteVertexColors;

  bool m_isStatic;
  InfluenceSet m_staticInfluenceSet;

  std::vector<Influence> influences;
};
