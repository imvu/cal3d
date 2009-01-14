//****************************************************************************//
// coremodel.h                                                                //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <map>
#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"

class CalCoreSkeleton;
class CalCoreAnimation;
class CalCoreAnimatedMorph;
class CalCoreMesh;
class CalCoreMaterial;

class CAL3D_API CalCoreModel
{
// member variables
protected:
  std::string m_strName;
  CalCoreSkeleton *m_pCoreSkeleton;
  std::vector<CalCoreAnimation *> m_vectorCoreAnimation;
  std::vector<CalCoreAnimatedMorph *> m_vectorCoreAnimatedMorph;
  std::vector<CalCoreMesh *> m_vectorCoreMesh;
  std::vector< boost::shared_ptr<CalCoreMaterial> > m_vectorCoreMaterial;
  std::map<int, std::map<int, int> > m_mapmapCoreMaterialThread;
  bool m_coreMeshManagement;
  bool m_coreAnimationManagement;
  unsigned int m_magic;

// constructors/destructor
public:
  CalCoreModel();
  ~CalCoreModel();

// member functions
public:

  int getNumCoreAnimations();

  int getNumCoreAnimatedMorphs();
  int addCoreAnimation(CalCoreAnimation *pCoreAnimation);
  bool removeCoreAnimation( int id );

  bool removeCoreAnimatedMorph( int id );

  int addCoreAnimatedMorph(CalCoreAnimatedMorph *pCoreAnimatedMorph);
  int addCoreMaterial(boost::shared_ptr<CalCoreMaterial> pCoreMaterial);
  int addCoreMesh(CalCoreMesh *pCoreMesh);
  bool createInternal(const std::string& strName);
  bool createWithName( char const * strName);
  bool createCoreMaterialThread(int coreMaterialThreadId);
  void destroy();
  CalCoreAnimation *getCoreAnimation(int coreAnimationId);
  CalCoreAnimatedMorph *getCoreAnimatedMorph(int coreAnimatedMorphId);
  int getCoreAnimationMaxId();
  boost::shared_ptr<CalCoreMaterial> getCoreMaterial(int coreMaterialId);
  int getCoreMaterialCount();
  int getCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId);
  CalCoreMesh *getCoreMesh(int coreMeshId);
  int getCoreMeshCount();
  CalCoreSkeleton *getCoreSkeleton();
  int loadCoreAnimation(const std::string& strFilename);
  int loadCoreAnimatedMorph(const std::string& strFilename);
  bool loadCoreSkeleton(const std::string& strFilename);
  bool saveCoreAnimation(const std::string& strFilename, int coreAnimtionId);
  bool saveCoreMaterial(const std::string& strFilename, int coreMaterialId);
  bool saveCoreMesh(const std::string& strFilename, int coreMeshId);
  bool saveCoreSkeleton(const std::string& strFilename);
  bool setCoreMaterialId(int coreMaterialThreadId, int coreMaterialSetId, int coreMaterialId);
  void setCoreSkeleton(CalCoreSkeleton *pCoreSkeleton);
  void scale(float factor);
  void setCoreMeshManagementOn( bool p ) { m_coreMeshManagement = p; }
  bool getCoreMeshManagementOn() { return m_coreMeshManagement; }
  void setCoreAnimationManagementOn( bool p ) { m_coreAnimationManagement = p; }
  bool getCoreAnimationManagementOn() { return m_coreAnimationManagement; }

};
