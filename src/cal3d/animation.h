#pragma once

#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"

class CalCoreAnimation;

class CAL3D_API CalAnimation : public Cal::Object
{
public:
  enum CompositionFunction {
    CompositionFunctionNull = 0,
    CompositionFunctionReplace,
    CompositionFunctionAverage,
    CompositionFunctionCrossFade
  };

public:
  CalAnimation();

  const boost::shared_ptr<CalCoreAnimation>& getCoreAnimation() const {
      return m_pCoreAnimation;
  }

  CalAnimation(const boost::shared_ptr<CalCoreAnimation>& pCoreAnimation);

  bool execute(float delayIn, float delayOut, float weightTarget = 1.0f,bool autoLock=false);
  void setManual();
  void setScale(float p) { m_scale = p; }
  float getScale() { return m_scale; }
  bool setCompositionFunction( CompositionFunction );
  CompositionFunction getCompositionFunction();

  float time;
  float timeFactor;
  float weight;
  float rampValue;

private:
  float m_scale;
  CompositionFunction m_compositionFunction;

  boost::shared_ptr<CalCoreAnimation> m_pCoreAnimation;
};
