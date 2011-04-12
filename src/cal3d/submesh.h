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
#include "cal3d/coresubmesh.h"
#include "cal3d/global.h"
#include "cal3d/vector.h"

typedef boost::shared_ptr<class CalCoreMaterial> CalCoreMaterialPtr;

// Structure used to return an array of the morphs that have non-zero weights.
struct MorphIdAndWeight {
    int morphId_;
    float weight_;
};

class CAL3D_API CalSubmesh : public Cal::UserDataHolder {
public:
    typedef CalCoreSubmesh::Face Face;

    const CalCoreSubmeshPtr coreSubmesh;
    CalCoreMaterialPtr material;

    CalSubmesh(const CalCoreSubmeshPtr& pCoreSubmesh);

    const std::vector<Face>& getVectorFace() const {
        return m_vectorFace;
    }

    std::vector<CalVector>& getVectorNormal() {
        return m_vectorNormal;
    }
    int getFaceCount() const {
        return m_faceCount;
    }
    std::vector<CalVector>& getVectorVertex() {
        return m_vectorVertex;
    }
    int getVertexCount() const {
        return m_vertexCount;
    }
private:
    void setLodLevel(float lodLevel);
public:
    std::vector<float>& getVectorWeight();
    float getMorphTargetWeight(int blendId) const;
    void setMorphTargetWeight(std::string const& morphName, float weight);
    bool getMorphTargetWeight(std::string const& morphName, float* weightOut) const;
    void getMorphIdAndWeightArray(
        MorphIdAndWeight* arrayResult,
        unsigned int* numMiawsResult,
        unsigned int maxMiaws) const;
    float getBaseWeight() const;
    int getMorphTargetWeightCount() const {
        return m_vectorMorphTargetWeight.size();
    }
    const std::vector<float>& getVectorMorphTargetWeight() {
        return m_vectorMorphTargetWeight;
    }
    void clearMorphTargetScales();
    void clearMorphTargetState(std::string const& morphName);
    void blendMorphTargetScale(std::string const& morphName,
                               float scale,
                               float unrampedWeight,
                               float rampValue,
                               bool replace);
    void setSubMorphTargetGroupAttenuatorArray(unsigned int len, int const* morphTargetIdArray);
    void setSubMorphTargetGroupAttenuationArray(unsigned int len, float const* attenuationArray);

private:
    std::vector<float> m_vectorMorphTargetWeight;
    std::vector<float> m_vectorAccumulatedWeight;
    std::vector<float> m_vectorReplacementAttenuation;
    std::vector<CalVector> m_vectorVertex;
    std::vector<CalVector> m_vectorNormal;
    std::vector<Face> m_vectorFace;
    std::vector<int> m_vectorSubMorphTargetGroupAttenuator;
    std::vector<float> m_vectorSubMorphTargetGroupAttenuation;
    int m_vertexCount;
    int m_faceCount;
};
