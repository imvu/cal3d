#pragma once

#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"

class CalCoreAnimation;

class CAL3D_API CalAnimation
{
public:
  enum CompositionFunction {
    CompositionFunctionReplace,
    CompositionFunctionAverage,
    CompositionFunctionCrossFade
  };

  CalAnimation(const boost::shared_ptr<CalCoreAnimation>& pCoreAnimation);

  const boost::shared_ptr<CalCoreAnimation>& getCoreAnimation() const {
      return m_pCoreAnimation;
  }

  float time;
  float timeFactor;
  float weight;
  float rampValue;

  /*****************************************************************************
   * Scale is different from weight.  Weight
   * is really relative weight.  All the weights are combined into a sum, and
   * each animation contributes according to the ratio of its weight to the sum.
   * In other words, the total influence of the weights is normalized to 1.0.
   * In contrast, scale factors apply to animation actions independently.  Scaling
   * one animation action up does not decrease the scale of other actions applying
   * to the same bones.  Whereas the weights are used to compose animations onto
   * a skeleton, scales are used to adjust the absolute, non-relative magnitude
   * of an animation.
   *****************************************************************************/
  float scale;

  CompositionFunction compositionFunction;

private:
  boost::shared_ptr<CalCoreAnimation> m_pCoreAnimation;
};
