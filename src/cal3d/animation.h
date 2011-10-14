#pragma once

#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"

typedef boost::shared_ptr<class CalCoreAnimation> CalCoreAnimationPtr;

class CAL3D_API CalAnimation {
public:
    enum CompositionFunction {
        CompositionFunctionReplace,
        CompositionFunctionAverage,
        CompositionFunctionCrossFade
    };

    CalAnimation(const CalCoreAnimationPtr& pCoreAnimation);

    const CalCoreAnimationPtr coreAnimation;

    float time;
    float weight;
    float rampValue;

    CompositionFunction compositionFunction;
};
typedef boost::shared_ptr<CalAnimation> CalAnimationPtr;
