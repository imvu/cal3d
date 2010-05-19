//****************************************************************************//
// renderer.cpp                                                               //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cal3d/color.h"
#include "cal3d/error.h"
#include "cal3d/renderer.h"
#include "cal3d/model.h"
#include "cal3d/coremesh.h"
#include "cal3d/mesh.h"
#include "cal3d/submesh.h"
#include "cal3d/skeleton.h"
#include "cal3d/bone.h"
#include "cal3d/corematerial.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/physique.h"

int CalRenderer::getMapCount(const CalSubmesh* submesh)
{
  // get the core material
  const boost::shared_ptr<CalCoreMaterial>& pCoreMaterial = submesh->getMaterial();
  if(!pCoreMaterial) return 0;

  return pCoreMaterial->maps.size();
}

Cal::UserData* CalRenderer::getMaterialUserData(const CalSubmesh* submesh)
{
  // get the core material
  const boost::shared_ptr<CalCoreMaterial>& pCoreMaterial = submesh->getMaterial();
  if(!pCoreMaterial) return 0;
  return pCoreMaterial->getUserData();
}

void CalRenderer::getSpecularColor(const CalSubmesh* submesh, unsigned char *pColorBuffer)
{
  // get the core material
  const boost::shared_ptr<CalCoreMaterial>& pCoreMaterial = submesh->getMaterial();
  if(pCoreMaterial == 0)
  {
    // write default values to the color buffer
    pColorBuffer[0] = 255;
    pColorBuffer[1] = 255;
    pColorBuffer[2] = 255;
    pColorBuffer[3] = 0;

    return;
  }

  // get the specular color of the material
  CalCoreMaterial::Color& color = pCoreMaterial->specularColor;

  // write it to the color buffer
  pColorBuffer[0] = color.red;
  pColorBuffer[1] = color.green;
  pColorBuffer[2] = color.blue;
  pColorBuffer[3] = color.alpha;
}

 /*****************************************************************************/
/** Returns true if texture coordinates exist for the given map.
  *
  *
  * @param mapId The ID of the map to test for texture coordinate data.
  *
  * @return true if texture coordinates for the given map are valid.
  *****************************************************************************/
bool 
CalRenderer::textureCoordinatesForMapValid(const CalSubmesh* submesh, int mapId )
{
  const std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> >& vectorvectorTextureCoordinate = submesh->getCoreSubmesh()->getVectorVectorTextureCoordinate();
  if((mapId < 0) || (mapId >= (int)vectorvectorTextureCoordinate.size())) {
    return false;
  }
  return true;
}
