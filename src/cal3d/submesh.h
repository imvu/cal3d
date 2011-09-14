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

namespace cal3d {
    struct CAL3D_API MorphTarget {
        MorphTarget(const CalCoreMorphTargetPtr& coreMorphTarget);
        void resetState();

        const CalCoreMorphTargetPtr coreMorphTarget;

        float weight;
        float accumulatedWeight;
        float replacementAttenuation;
    };
}


class CAL3D_API CalSubmesh : public cal3d::UserDataHolder {
public:
    const CalCoreSubmeshPtr coreSubmesh;
    CalCoreMaterialPtr material;
    std::vector<cal3d::MorphTarget> morphTargets; // index maps to CoreSubMorphTarget in CoreSubmesh

    CalSubmesh(const CalCoreSubmeshPtr& pCoreSubmesh);

    void setMorphTargetWeight(std::string const& morphName, float weight);
    void clearMorphTargetScales();
    void clearMorphTargetState(std::string const& morphName);
    void blendMorphTargetScale(
        std::string const& morphName,
        float scale,
        float unrampedWeight,
        float rampValue,
        bool replace);
};
