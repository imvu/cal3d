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

typedef boost::shared_ptr<class CalCoreSkeleton> CalCoreSkeletonPtr;
class CalCoreBone;
typedef boost::shared_ptr<class CalCoreAnimation> CalCoreAnimationPtr;
typedef boost::shared_ptr<class CalCoreAnimatedMorph> CalCoreAnimatedMorphPtr;
class CalCoreTrack;
class CalCoreKeyframe;
class CalCoreMorphTrack;
class CalCoreMorphKeyframe;
typedef boost::shared_ptr<class CalCoreMesh> CalCoreMeshPtr;
class CalCoreSubmesh;
typedef boost::shared_ptr<class CalCoreMaterial> CalCoreMaterialPtr;
class CalVector;
class CalQuaternion;

class CAL3D_API CalSaver {
public:
    static std::string saveCoreAnimationToBuffer(CalCoreAnimationPtr pCoreAnimation);
    static std::string saveCoreAnimatedMorphToBuffer(CalCoreAnimatedMorphPtr pCoreAnimatedMorph);
    static std::string saveCoreMaterialToBuffer(CalCoreMaterialPtr pCoreMaterial);
    static std::string saveCoreMeshToBuffer(CalCoreMeshPtr pCoreMesh);
    static std::string saveCoreSkeletonToBuffer(CalCoreSkeletonPtr pCoreSkeleton);

    static bool saveCoreAnimation(const std::string& strFilename, CalCoreAnimation* pCoreAnimation);
    static bool saveCoreAnimation(std::ostream& stream, CalCoreAnimation* coreAnimation);

    static bool saveCoreAnimatedMorph(const std::string& strFilename, CalCoreAnimatedMorph* pCoreAnimatedMorph);
    static bool saveCoreAnimatedMorph(std::ostream& file, CalCoreAnimatedMorph* pCoreAnimatedMorph);

    static bool saveCoreMaterial(const std::string& strFilename, CalCoreMaterial* pCoreMaterial);
    static bool saveCoreMaterial(std::ostream& file, CalCoreMaterial* pCoreMaterial);

    static bool saveCoreMesh(const std::string& strFilename, CalCoreMesh* pCoreMesh);
    static bool saveCoreMesh(std::ostream& os, CalCoreMesh* pCoreMesh);

    static bool saveCoreSkeleton(const std::string& strFilename, CalCoreSkeleton* pCoreSkeleton);
    static bool saveCoreSkeleton(std::ostream& os, CalCoreSkeleton* pCoreSkeleton);

    static bool saveXmlCoreSkeleton(const std::string& strFilename, CalCoreSkeleton* pCoreSkeleton);
    static bool saveXmlCoreAnimation(const std::string& strFilename, CalCoreAnimation* pCoreAnimation);
    static bool saveXmlCoreAnimation(std::ostream& os, CalCoreAnimation* pCoreAnimation);
    static bool saveXmlCoreAnimatedMorph(const std::string& strFilename, CalCoreAnimatedMorph* pCoreAnimatedMorph);
    static bool saveXmlCoreMesh(const std::string& strFilename, CalCoreMesh* pCoreMesh);
    static bool saveXmlCoreMaterial(const std::string& strFilename, CalCoreMaterial* pCoreMaterial);

private:
    static bool saveCoreBones(std::ostream& file, CalCoreBone* pCoreBone);
    static bool saveCoreKeyframe(std::ostream& file, const std::string& strFilename, const CalCoreKeyframe* pCoreKeyframe);
    static bool saveCoreSubmesh(std::ostream& file, CalCoreSubmesh* pCoreSubmesh);
    static bool saveCoreTrack(std::ostream& file, const std::string& strFilename, CalCoreTrack* pCoreTrack);
    static bool saveCoreMorphKeyframe(std::ostream& file, const std::string& strFilename, CalCoreMorphKeyframe* pCoreMorphKeyframe);
    static bool saveCoreMorphTrack(std::ostream& file, const std::string& strFilename, CalCoreMorphTrack* pCoreMorphTrack);

private:
    CalSaver();
    ~CalSaver();
};
