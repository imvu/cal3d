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

public:
  CalRenderer();

public:
  void create(CalModel *pModel);
  void destroy();
  int getMapCount(CalSubmesh* submesh);
  Cal::UserData* getMaterialUserData(CalSubmesh* submesh);
  void getSpecularColor(CalSubmesh* submesh, unsigned char *pColorBuffer);
  int getTextureCoordinates(CalSubmesh* submesh, int mapId, float *pTextureCoordinateBuffer);
  int getVertColorsAsStandardPixels( CalSubmesh* submesh, unsigned long *pVertexBuffer);
  int getVerticesAndNormals(CalSubmesh* submesh, float *pVertexBuffer);
  void setNormalization(bool normalize);
  bool textureCoordinatesForMapValid(CalSubmesh* submesh, int mapId );
};
