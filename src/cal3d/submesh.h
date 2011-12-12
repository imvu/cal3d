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

CAL3D_PTR(CalCoreMaterial);

namespace cal3d {
    class CAL3D_API MorphTarget {
    public:
        MorphTarget(const CalCoreMorphTargetPtr& coreMorphTarget);
        void resetState();

        CalCoreMorphTargetPtr coreMorphTarget;

        float weight;
        float accumulatedWeight;
        float replacementAttenuation;
    };
    CAL3D_PTR(MorphTarget);
}


class CAL3D_API CalSubmesh {
public:
    const CalCoreSubmeshPtr coreSubmesh;
    std::vector<cal3d::MorphTarget> morphTargets; // index maps to CoreSubMorphTarget in CoreSubmesh

    CalSubmesh(const CalCoreSubmeshPtr& coreSubmesh);

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
