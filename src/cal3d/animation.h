#pragma once

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

protected:
  CalCoreAnimation *m_pCoreAnimation;
  Type m_type;
  State m_state;
  float m_time;
  float m_timeFactor;
  float m_weight;

protected:
  CalAnimation();
public:
  virtual ~CalAnimation();

  virtual bool create(CalCoreAnimation *pCoreAnimation) = 0;
  virtual void destroy();
  CalCoreAnimation *getCoreAnimation();
  State getState();
  float getTime();
  Type getType();
  float getWeight();
  void setTime(float time);
  void setTimeFactor(float timeFactor);
  float getTimeFactor();
};
