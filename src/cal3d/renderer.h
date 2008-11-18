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
  void getAmbientColor(unsigned char *pColorBuffer);
  void getDiffuseColor(unsigned char *pColorBuffer);
  CalSubmesh* getSelectedSubmesh() const {
      return m_pSelectedSubmesh;
  }
  CalModel * getModel() { return m_pModel; }
  int getMapCount();
  Cal::UserData* getMaterialUserData();
  int getMeshCount();
  int getNormals(float *pNormalBuffer);
  float getShininess();
  void getSpecularColor(unsigned char *pColorBuffer);
  int getSubmeshCount(int meshId);
  int getTextureCoordinates(int mapId, float *pTextureCoordinateBuffer);
  int getVertexCount();
  int getVertices(float *pVertexBuffer);
  int getVertColors(float *pVertexBuffer);
  int getVertColorsAsStandardPixels( unsigned long *pVertexBuffer);
  int getTangentSpaces(int mapId, float *pTangentSpaceBuffer);
  int getVerticesAndNormals(float *pVertexBuffer);
  int getVerticesNormalsAndTexCoords(float *pVertexBuffer,int NumTexCoords=1);
  bool isTangentsEnabled(int mapId);
  bool selectMeshSubmesh(int meshId, int submeshId);
  void setNormalization(bool normalize);
  bool textureCoordinatesForMapValid( int mapId );
  bool hasNonWhiteVertexColors();
};
