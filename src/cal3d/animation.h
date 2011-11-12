#pragma once

#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"

typedef boost::shared_ptr<class CalCoreAnimation> CalCoreAnimationPtr;

class CAL3D_API CalAnimation {
public:
    CalAnimation(const CalCoreAnimationPtr& pCoreAnimation, float weight, unsigned priority);

    const CalCoreAnimationPtr coreAnimation;

    float time; // current time
    float rampValue; // 0->1 fade in, 1->0 fade out
    const float weight;
    const unsigned priority; // 0 is lowest
};
typedef boost::shared_ptr<CalAnimation> CalAnimationPtr;
