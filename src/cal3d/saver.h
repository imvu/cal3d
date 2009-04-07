//****************************************************************************//
// saver.h                                                                    //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"

class CalCoreModel;
class CalCoreSkeleton;
class CalCoreBone;
class CalCoreAnimation;
class CalCoreAnimatedMorph;
class CalCoreTrack;
class CalCoreKeyframe;
class CalCoreMorphTrack;
class CalCoreMorphKeyframe;
class CalCoreMesh;
class CalCoreSubmesh;
class CalCoreMaterial;
class CalVector;
class CalQuaternion;

class CAL3D_API CalSaver : public Cal::Object
{
public:
  static bool saveCoreAnimation(const std::string& strFilename, CalCoreAnimation *pCoreAnimation);
  static bool saveCoreAnimatedMorph(const std::string& strFilename, CalCoreAnimatedMorph *pCoreAnimatedMorph);
  static bool saveCoreMaterial(const std::string& strFilename, CalCoreMaterial *pCoreMaterial);
  static bool saveCoreMesh(const std::string& strFilename, CalCoreMesh *pCoreMesh);
  static bool saveCoreSkeleton(const std::string& strFilename, CalCoreSkeleton *pCoreSkeleton);

  static bool saveXmlCoreSkeleton(const std::string& strFilename, CalCoreSkeleton *pCoreSkeleton);
  static bool saveXmlCoreAnimation(const std::string& strFilename, CalCoreAnimation *pCoreAnimation);
  static bool saveXmlCoreAnimation(std::ostream& os, CalCoreAnimation* pCoreAnimation);
  static bool saveXmlCoreAnimatedMorph(const std::string& strFilename, CalCoreAnimatedMorph *pCoreAnimatedMorph);
  static bool saveXmlCoreMesh(const std::string& strFilename, CalCoreMesh *pCoreMesh);
  static bool saveXmlCoreMaterial(const std::string& strFilename, CalCoreMaterial *pCoreMaterial);  

protected:
  static bool saveCoreBones(std::ofstream& file, const std::string& strFilename, CalCoreBone *pCoreBone);
  static bool saveCoreKeyframe(std::ofstream& file, const std::string& strFilename, CalCoreKeyframe *pCoreKeyframe, int version, 
    bool needTranslation, bool highRangeRequired, bool useAnimationCompression );
  static bool saveCoreSubmesh(std::ofstream& file, const std::string& strFilename, CalCoreSubmesh *pCoreSubmesh);
  static bool saveCoreTrack(std::ofstream& file, const std::string& strFilename, CalCoreTrack *pCoreTrack, bool useAnimationCompression, int version);
  static bool saveCoreMorphKeyframe(std::ofstream& file, const std::string& strFilename, CalCoreMorphKeyframe *pCoreMorphKeyframe);
  static bool saveCoreMorphTrack(std::ofstream& file, const std::string& strFilename, CalCoreMorphTrack *pCoreMorphTrack);
};
