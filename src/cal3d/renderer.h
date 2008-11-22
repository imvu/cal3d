//****************************************************************************//
// renderer.h                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"

class CalModel;
class CalSubmesh;

class CAL3D_API CalRenderer
{
private:
  CalModel *m_pModel;
  CalSubmesh *m_pSelectedSubmesh;

public:
  CalRenderer();
  CalRenderer(CalRenderer* pRenderer); 
  ~CalRenderer();

public:
  bool beginRendering();
  void create(CalModel *pModel);
  void destroy();
  void endRendering();
  int getMapCount();
  Cal::UserData* getMaterialUserData();
  void getSpecularColor(unsigned char *pColorBuffer);
  int getSubmeshCount(int meshId);
  int getTextureCoordinates(int mapId, float *pTextureCoordinateBuffer);
  int getVertColorsAsStandardPixels( unsigned long *pVertexBuffer);
  int getVerticesAndNormals(float *pVertexBuffer);
  CalSubmesh* selectMeshSubmesh(int meshId, int submeshId);
  void setNormalization(bool normalize);
  bool textureCoordinatesForMapValid( int mapId );
};
