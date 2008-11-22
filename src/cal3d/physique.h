//****************************************************************************//
// physique.h                                                                 //
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

class CAL3D_API CalPhysique
{
// member variables
public:
  CalModel *m_pModel;

// constructors/destructor
public:
  CalPhysique();
  ~CalPhysique();

// member functions	
public:
  int calculateTangentSpaces(CalSubmesh *pSubmesh, int mapId, float *pTangentSpaceBuffer);
  int calculateNormals(CalSubmesh *pSubmesh, float *pNormalBuffer);
  int calculateVertices(CalSubmesh *pSubmesh, float *pVertexBuffer);
  int calculateVerticesAndNormals(CalSubmesh *pSubmesh, float *pVertexBuffer);
  void create(CalModel *pModel);
  void destroy();
};
