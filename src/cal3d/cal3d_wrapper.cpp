//****************************************************************************//
// cal3d_wrapper.cpp                                                          //
// Copyright (C) 2002 Bruno 'Beosil' Heidelberger                             //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//****************************************************************************//
// Defines                                                                    //
//****************************************************************************//

#define CAL3D_WRAPPER_EXPORTS

//****************************************************************************//
// Includes                                                                   // 
//****************************************************************************//

#include "cal3d/cal3d.h"
#include "cal3d/cal3d_wrapper.h"
#include "cal3d/coremodel.h"
#include "cal3d/corematerial.h"
#include "cal3d/coresubmesh.h"

//****************************************************************************//
// CalCoreAnimation wrapper functions definition                              //
//****************************************************************************//

Boolean CalCoreAnimation_AddCoreTrack(CalCoreAnimation *self, CalCoreTrack *pCoreTrack)
{
  return self->addCoreTrack(pCoreTrack) ? True : False;
}

void CalCoreAnimation_Delete(CalCoreAnimation *self)
{
  delete self;
}

CalCoreTrack *CalCoreAnimation_GetCoreTrack(CalCoreAnimation *self, int coreBoneId)
{
  return self->getCoreTrack(coreBoneId);
}

float CalCoreAnimation_GetDuration(CalCoreAnimation *self)
{
  return self->getDuration();
}

/*
std::list<CalCoreTrack *>& CalCoreAnimation_GetListCoreTrack(CalCoreAnimation *self)
{
  return self->getListCoreTrack();
}
*/

CalCoreAnimation *CalCoreAnimation_New(CalCoreAnimation *self)
{
  return new CalCoreAnimation();
}

void CalCoreAnimation_SetDuration(CalCoreAnimation *self, float duration)
{
  self->setDuration(duration);
}

//****************************************************************************//
// CalCoreKeyframe wrapper functions definition                               //
//****************************************************************************//

void CalCoreKeyframe_Delete(CalCoreKeyframe *self)
{
  delete self;
}

CalQuaternion *CalCoreKeyframe_GetRotation(CalCoreKeyframe *self)
{
  return &(const_cast<CalQuaternion&>(self->getRotation()));
}

float CalCoreKeyframe_GetTime(CalCoreKeyframe *self)
{
  return self->getTime();
}

CalVector *CalCoreKeyframe_GetTranslation(CalCoreKeyframe *self)
{
  return &(const_cast<CalVector&>(self->getTranslation()));
}

CalCoreKeyframe *CalCoreKeyframe_New()
{
  return new CalCoreKeyframe();
}

void CalCoreKeyframe_SetRotation(CalCoreKeyframe *self, CalQuaternion *pRotation)
{
  self->setRotation(*pRotation);
}

void CalCoreKeyframe_SetTime(CalCoreKeyframe *self, float time)
{
  self->setTime(time);
}

void CalCoreKeyframe_SetTranslation(CalCoreKeyframe *self, CalVector *pTranslation)
{
  self->setTranslation(*pTranslation);
}

//****************************************************************************//
// CalCoreMaterial wrapper functions definition                               //
//****************************************************************************//

void CalCoreMaterial_Delete(CalCoreMaterial *self)
{
  delete self;
}

int CalCoreMaterial_GetMapCount(CalCoreMaterial *self)
{
  return self->getMapCount();
}

char *CalCoreMaterial_GetMapFilename(CalCoreMaterial *self, int mapId)
{
  return const_cast<char *>(self->getMapFilename(mapId).c_str());
}

char *CalCoreMaterial_GetMapType(CalCoreMaterial *self, int mapId)
{
  return const_cast<char *>(self->getMapType(mapId).c_str());
}

float CalCoreMaterial_GetShininess(CalCoreMaterial *self)
{
  return self->getShininess();
}

CalCoreMaterial *CalCoreMaterial_New()
{
  return new CalCoreMaterial();
}

void CalCoreMaterial_SetShininess(CalCoreMaterial *self, float shininess)
{
  self->setShininess(shininess);
}

//****************************************************************************//
// CalCoreMesh wrapper functions definition                                   //
//****************************************************************************//

int CalCoreMesh_AddCoreSubmesh(CalCoreMesh *self, CalCoreSubmesh *pCoreSubmesh)
{
  return self->addCoreSubmesh(pCoreSubmesh);
}

void CalCoreMesh_Delete(CalCoreMesh *self)
{
  delete self;
}

CalCoreSubmesh *CalCoreMesh_GetCoreSubmesh(CalCoreMesh *self, int id)
{
  return self->getCoreSubmesh(id);
}

int CalCoreMesh_GetCoreSubmeshCount(CalCoreMesh *self)
{
  return self->getCoreSubmeshCount();
}

/*
std::vector<CalCoreSubmesh *>& CalCoreMesh_GetVectorCoreSubmesh(CalCoreMesh *self)
{
  return self->getVectorCoreSubmesh();
}
*/

CalCoreMesh *CalCoreMesh_New()
{
  return new CalCoreMesh();
}

Boolean CalCoreModel_CreateCoreMaterialThread(CalCoreModel *self, int coreMaterialThreadId)
{
  return True;
}

void CalCoreModel_Delete(CalCoreModel *self)
{
  delete self;
}

CalCoreModel *CalCoreModel_New()
{
  return new CalCoreModel();
}

//****************************************************************************//
// CalCoreSkeleton wrapper functions definition                               //
//****************************************************************************//

int CalCoreSkeleton_AddCoreBone(CalCoreSkeleton *self, CalCoreBone *pCoreBone)
{
  return self->addCoreBone(pCoreBone);
}

Boolean CalCoreSkeleton_Create(CalCoreSkeleton *self)
{
  return True;
}

void CalCoreSkeleton_Delete(CalCoreSkeleton *self)
{
  delete self;
}

CalCoreBone *CalCoreSkeleton_GetCoreBone(CalCoreSkeleton *self, int coreBoneId)
{
  return self->getCoreBone(coreBoneId);
}

int CalCoreSkeleton_GetCoreBoneId(CalCoreSkeleton *self, char *strName)
{
  return self->getCoreBoneId(strName);
}

/*
std::list<int>& CalCoreSkeleton_GetListRootCoreBoneId(CalCoreSkeleton *self)
{
  return self->getlistRootCoreBoneId();
}
*/


/*
std::vector<CalCoreBone *>& CalCoreSkeleton_GetVectorCoreBone(CalCoreSkeleton *self)
{
  return self->getVectorCoreBone();
}
*/

CalCoreSkeleton *CalCoreSkeleton_New()
{
  return new CalCoreSkeleton();
}

//****************************************************************************//
// CalCoreSubmesh wrapper functions definition                                //
//****************************************************************************//

void CalCoreSubmesh_Delete(CalCoreSubmesh *self)
{
  delete self;
}

int CalCoreSubmesh_GetCoreMaterialThreadId(CalCoreSubmesh *self)
{
  return self->getCoreMaterialThreadId();
}

int CalCoreSubmesh_GetFaceCount(CalCoreSubmesh *self)
{
  return self->getFaceCount();
}

int CalCoreSubmesh_GetLodCount(CalCoreSubmesh *self)
{
  return self->getLodCount();
}

int CalCoreSubmesh_GetVertexCount(CalCoreSubmesh *self)
{
  return self->getVertexCount();
}

void CalCoreSubmesh_SetCoreMaterialThreadId(CalCoreSubmesh *self, int coreMaterialThreadId)
{
  self->setCoreMaterialThreadId(coreMaterialThreadId);
}

void CalCoreSubmesh_SetLodCount(CalCoreSubmesh *self, int lodCount)
{
  self->setLodCount(lodCount);
}


//****************************************************************************//
// CalCoreTrack wrapper functions definition                                  //
//****************************************************************************//

Boolean CalCoreTrack_AddCoreKeyframe(CalCoreTrack *self, CalCoreKeyframe *pCoreKeyframe)
{
  return self->addCoreKeyframe(pCoreKeyframe) ? True : False;
}

Boolean CalCoreTrack_Create(CalCoreTrack *self)
{
  self->create();
  return True;
}

void CalCoreTrack_Delete(CalCoreTrack *self)
{
  delete self;
}

void CalCoreTrack_Destroy(CalCoreTrack *self)
{
  self->destroy();
}

int CalCoreTrack_GetCoreBoneId(CalCoreTrack *self)
{
  return self->getCoreBoneId();
}

/*
std::map<float, CalCoreKeyframe *>& CalCoreTrack_GetMapCoreKeyframe(CalCoreTrack *self)
{
  return self->getMapCoreKeyframe();
}
*/

Boolean CalCoreTrack_GetState(CalCoreTrack *self, float time, CalVector *pTranslation, CalQuaternion *pRotation)
{
  return self->getState(time, *pTranslation, *pRotation) ? True : False;
}

CalCoreTrack *CalCoreTrack_New()
{
  return new CalCoreTrack();
}

Boolean CalCoreTrack_SetCoreBoneId(CalCoreTrack *self, int coreBoneId)
{
  return self->setCoreBoneId(coreBoneId) ? True : False;
}

//****************************************************************************//
// CalError wrapper functions definition                                      //
//****************************************************************************//

CalErrorCode CalError_GetLastErrorCode()
{
  return (CalErrorCode)CalError::getLastErrorCode();
}

char *CalError_GetLastErrorDescription()
{
  static std::string strDescription;
  strDescription = CalError::getLastErrorDescription();

  return const_cast<char *>(strDescription.c_str());
}

char *CalError_GetLastErrorFile()
{
  return const_cast<char *>(CalError::getLastErrorFile());
}

int CalError_GetLastErrorLine()
{
  return CalError::getLastErrorLine();
}

char *CalError_GetLastErrorText()
{
  return const_cast<char *>(CalError::getLastErrorText());
}

void CalError_PrintLastError()
{
  CalError::printLastError();
}

void CalError_SetLastError(CalErrorCode code, char *strFile, int line, char *strText)
{
  CalError::setLastError((CalError::Code)code, strFile, line, strText);
}

//****************************************************************************//
// CalLoader wrapper functions definition                                     //
//****************************************************************************//

CalCoreAnimation *CalLoader_LoadCoreAnimationFromBuffer(CalLoader*, char *buf, unsigned int len)
{
  return CalLoader::loadCoreAnimationFromBuffer(buf, len, NULL);
}

CalCoreAnimatedMorph *CalLoader_LoadCoreAnimatedMorphFromBuffer(CalLoader*, char *buf, unsigned int len)
{
  return CalLoader::loadCoreAnimatedMorphFromBuffer(buf, len);
}

CalCoreMaterial *CalLoader_LoadCoreMaterialFromBuffer(CalLoader*, char *buf, unsigned int len)
{
  return CalLoader::loadCoreMaterialFromBuffer(buf, len);
}

CalCoreMesh *CalLoader_LoadCoreMeshFromBuffer(CalLoader*, char *buf, unsigned int len)
{
  return CalLoader::loadCoreMeshFromBuffer(buf, len);
}

CalCoreSkeleton *CalLoader_LoadCoreSkeletonFromBuffer(CalLoader*, char *buf, unsigned int len)
{
  return CalLoader::loadCoreSkeletonFromBuffer(buf, len);
}

void CalLoader_CompressCoreAnimation(CalLoader*, CalCoreAnimation * anim, CalCoreSkeleton * skelOrNull)
{
  CalLoader::compressCoreAnimation( anim, skelOrNull );
}


Boolean CalSaver_SaveCoreAnimation(CalSaver*, char *strFilename, CalCoreAnimation *pCoreAnimation)
{
  return CalSaver::saveCoreAnimation(strFilename, pCoreAnimation) ? True : False;
}

Boolean CalSaver_SaveCoreAnimatedMorph(CalSaver*, char *strFilename, CalCoreAnimatedMorph *pCoreAnimatedMorph)
{
  return CalSaver::saveCoreAnimatedMorph(strFilename, pCoreAnimatedMorph) ? True : False;
}

Boolean CalSaver_SaveCoreMaterial(CalSaver*, char *strFilename, CalCoreMaterial *pCoreMaterial)
{
  return CalSaver::saveCoreMaterial(strFilename, pCoreMaterial) ? True : False;
}

Boolean CalSaver_SaveCoreMesh(CalSaver*, char *strFilename, CalCoreMesh *pCoreMesh)
{
  return CalSaver::saveCoreMesh(strFilename, pCoreMesh) ? True : False;
}

Boolean CalSaver_SaveCoreSkeleton(CalSaver*, char *strFilename, CalCoreSkeleton *pCoreSkeleton)
{
  return CalSaver::saveCoreSkeleton(strFilename, pCoreSkeleton) ? True : False;
}

//****************************************************************************//
// CalVector wrapper functions definition                                     //
//****************************************************************************//

void CalVector_Add(CalVector *self, CalVector *pV)
{
  *self += *pV;
}

void CalVector_Blend(CalVector *self, float d, CalVector *pV)
{
  self->blend(d, *pV);
}

void CalVector_Clear(CalVector *self)
{
  self->clear();
}

void CalVector_Delete(CalVector *self)
{
  delete self;
}

void CalVector_Equal(CalVector *self, CalVector *pV)
{
  *self = *pV;
}

void CalVector_InverseScale(CalVector *self, float d)
{
  *self /= d;
}

float *CalVector_Get(CalVector *self)
{
  return &(self->x);
}

float CalVector_Length(CalVector *self)
{
  return self->length();
}

CalVector *CalVector_New()
{
  return new CalVector();
}

float CalVector_Normalize(CalVector *self)
{
  return self->normalize();
}

void CalVector_Op_Add(CalVector *pResult, CalVector *pV, CalVector *pU)
{
  *pResult = *pV + *pU;
}

void CalVector_Op_Subtract(CalVector *pResult, CalVector *pV, CalVector *pU)
{
  *pResult = *pV - *pU;
}

void CalVector_CalVector_Op_Scale(CalVector *pResult, CalVector *pV, float d)
{
  *pResult = *pV * d;
}

void CalVector_CalVector_Op_InverseScale(CalVector *pResult, CalVector *pV, float d)
{
  *pResult = *pV / d;
}

float CalVector_Op_Scalar(CalVector *pV, CalVector *pU)
{
  return *pV * *pU;
}

void CalVector_Op_Cross(CalVector *pResult, CalVector *pV, CalVector *pU)
{
  *pResult = *pV % *pU;
}

void CalVector_Scale(CalVector *self, float d)
{
  *self *= d;
}

void CalVector_Set(CalVector *self, float vx, float vy, float vz)
{
  self->set(vx, vy, vz);
}

void CalVector_Subtract(CalVector *self, CalVector *pV)
{
  *self = *pV;
}

void CalVector_Transform(CalVector *self, CalQuaternion *pQ)
{
  *self *= *pQ;
}

//****************************************************************************//
