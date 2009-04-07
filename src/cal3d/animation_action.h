//****************************************************************************//
// animation_action.h                                                         //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"
#include "cal3d/animation.h"

class CalCoreAnimation;

class CAL3D_API CalAnimationAction : public CalAnimation
{
public:
  CalAnimationAction();
  virtual ~CalAnimationAction();

  bool create(CalCoreAnimation *pCoreAnimation);
  void destroy();
  bool execute(float delayIn, float delayOut, float weightTarget = 1.0f,bool autoLock=false);
  bool update(float deltaTime);
  bool setManual();
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
    SequencingModeNull = 0,
    SequencingModeAutomatic,
    SequencingModeManual
  } m_sequencingMode;
  bool m_manualOn;
};
