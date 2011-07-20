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

struct MorphIdAndWeight;

class CAL3D_API CalSubmesh : public Cal::UserDataHolder {
public:
    const CalCoreSubmeshPtr coreSubmesh;
    CalCoreMaterialPtr material;
    std::vector<float> morphTargetWeights;

    CalSubmesh(const CalCoreSubmeshPtr& pCoreSubmesh);

    std::vector<float>& getVectorWeight();
    void setMorphTargetWeight(std::string const& morphName, float weight);
    void getMorphIdAndWeightArray(
        MorphIdAndWeight* arrayResult,
        size_t* numMiawsResult,
        size_t maxMiaws) const;
    float getBaseWeight() const;
    void clearMorphTargetScales();
    void clearMorphTargetState(std::string const& morphName);
    void blendMorphTargetScale(std::string const& morphName,
                               float scale,
                               float unrampedWeight,
                               float rampValue,
                               bool replace);

private:
    std::vector<float> m_vectorAccumulatedWeight;
    std::vector<float> m_vectorReplacementAttenuation;
};
