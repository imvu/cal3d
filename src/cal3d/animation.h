#pragma once

#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"

class CalCoreAnimation;

class CAL3D_API CalAnimation : public Cal::Object
{
// misc
public:
  enum Type
  {
    TYPE_NONE = 0,
    TYPE_CYCLE,
    TYPE_POSE,
    TYPE_ACTION
  };

  enum State
  {
    STATE_NONE = 0,
    STATE_SYNC,
    STATE_ASYNC,
    STATE_IN,
    STATE_STEADY,
    STATE_OUT,
	STATE_STOPPED
  };
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
  State getState() { return m_state; }
  Type getType() { return m_type; }
  float getWeight() { return m_weight; }

  CalAnimation(const boost::shared_ptr<CalCoreAnimation>& pCoreAnimation);

  bool execute(float delayIn, float delayOut, float weightTarget = 1.0f,bool autoLock=false);
  bool update(float deltaTime);
  void setManual();
  bool setManualAnimationActionOn( bool p );
  bool setManualAnimationActionWeight( float );
  bool setScale( float );
  float getScale();
  bool setCompositionFunction( CompositionFunction );
  CompositionFunction getCompositionFunction();
  bool setRampValue( float );
  float getRampValue();
  bool manual();
  bool on();

  float time;
  float timeFactor;

private:
  float m_delayIn;
  float m_delayOut;
  float m_delayTarget;
  float m_weightTarget;
  bool  m_autoLock; 
  float m_scale;
  float m_rampValue;
  CompositionFunction m_compositionFunction;
  enum SequencingMode {
    SequencingModeAutomatic,
    SequencingModeManual
  } m_sequencingMode;
  bool m_manualOn;

private:
  boost::shared_ptr<CalCoreAnimation> m_pCoreAnimation;
  Type m_type;
  State m_state;
  float m_weight;
};
