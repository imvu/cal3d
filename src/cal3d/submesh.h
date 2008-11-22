//****************************************************************************//
// submesh.h                                                                  //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"
#include "cal3d/vector.h"

class CalCoreMaterial;
class CalCoreSubmesh;

// Structure used to return an array of the morphs that have non-zero weights.
struct MorphIdAndWeight {
  int morphId_;
  float weight_;
};

class CAL3D_API CalSubmesh
{
// misc
public:
  /// The submesh PhysicalProperty.
  typedef struct
  {
    CalVector position;
    CalVector positionOld;
    CalVector force;
  } PhysicalProperty;

  /// The submesh TangentSpace.
  struct TangentSpace
  {
    CalVector tangent;
    float crossFactor;
  };


  /// The submesh Face.
  typedef struct
  {
    CalIndex vertexId[3];
  } Face;

// member variables
protected:
  CalCoreSubmesh *m_pCoreSubmesh;
  std::vector<float> m_vectorMorphTargetWeight;
  std::vector<float> m_vectorAccumulatedWeight;
  std::vector<float> m_vectorReplacementAttenuation;
  std::vector<CalVector> m_vectorVertex;
  std::vector<CalVector> m_vectorNormal;
  std::vector<std::vector<TangentSpace> > m_vectorvectorTangentSpace;
  std::vector<Face> m_vectorFace;
  std::vector<PhysicalProperty> m_vectorPhysicalProperty;
  std::vector<int> m_vectorSubMorphTargetGroupAttenuator;
  std::vector<float> m_vectorSubMorphTargetGroupAttenuation;  
  int m_vertexCount;
  int m_faceCount;
  boost::shared_ptr<CalCoreMaterial> m_material;

public:
  CalSubmesh();

  bool create(CalCoreSubmesh *pCoreSubmesh);
  CalCoreSubmesh *getCoreSubmesh();
  const std::vector<Face>& getVectorFace() const {
      return m_vectorFace;
  }
  boost::shared_ptr<CalCoreMaterial> getMaterial() {
      return m_material;
  }
  void setMaterial(boost::shared_ptr<CalCoreMaterial> material) {
      m_material = material;
  }

  std::vector<CalVector>& getVectorNormal();
  std::vector<std::vector<TangentSpace> >& getVectorVectorTangentSpace();
  std::vector<PhysicalProperty>& getVectorPhysicalProperty();
  int getFaceCount() { return m_faceCount; }
  std::vector<CalVector>& getVectorVertex();
  int getVertexCount();
  void setLodLevel(float lodLevel);
  bool isTangentsEnabled(int mapId);
  bool enableTangents(int mapId, bool enabled);
  std::vector<float>& getVectorWeight();
  void setMorphTargetWeight(int blendId,float weight);
  float getMorphTargetWeight(int blendId);
  void setMorphTargetWeight(std::string const & morphName,float weight);
  bool getMorphTargetWeight(std::string const & morphName, float * weightOut);
  void getMorphIdAndWeightArray( MorphIdAndWeight * arrayResult, 
    unsigned int * numMiawsResult, 
    unsigned int maxMiaws );
  float getBaseWeight();
  int getMorphTargetWeightCount();
  std::vector<float>& getVectorMorphTargetWeight();
  void clearMorphTargetScales();
  void clearMorphTargetState( std::string const & morphName );
  void blendMorphTargetScale( std::string const & morphName, 
    float scale, 
    float unrampedWeight, 
    float rampValue,
    bool replace );
  void setSubMorphTargetGroupAttenuatorArray( unsigned int len, int const * morphTargetIdArray );
  void setSubMorphTargetGroupAttenuationArray( unsigned int len, float const * attenuationArray );
};
