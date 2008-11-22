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

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "cal3d/error.h"
#include "cal3d/renderer.h"
#include "cal3d/coremodel.h"
#include "cal3d/model.h"
#include "cal3d/coremesh.h"
#include "cal3d/mesh.h"
#include "cal3d/submesh.h"
#include "cal3d/skeleton.h"
#include "cal3d/bone.h"
#include "cal3d/corematerial.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/physique.h"

 /*****************************************************************************/
/** Returns the number of maps.
  *
  * This function returns the number of maps in the selected mesh/submesh.
  *
  * @return The number of maps.
  *****************************************************************************/

int CalRenderer::getMapCount(CalSubmesh* submesh)
{
  // get the core material
  boost::shared_ptr<CalCoreMaterial> pCoreMaterial = submesh->getMaterial();
  if(!pCoreMaterial) return 0;

  return pCoreMaterial->getMapCount();
}

Cal::UserData* CalRenderer::getMaterialUserData(CalSubmesh* submesh)
{
  // get the core material
  boost::shared_ptr<CalCoreMaterial> pCoreMaterial = submesh->getMaterial();
  if(!pCoreMaterial) return 0;
  return pCoreMaterial->getUserData();
}

 /*****************************************************************************/
/** Provides access to the specular color.
  *
  * This function returns the specular color of the material of the selected
  * mesh/submesh.
  *
  * @param pColorBuffer A pointer to the user-provided buffer where the color
  *                     data is written to.
  *****************************************************************************/

void CalRenderer::getSpecularColor(CalSubmesh* submesh, unsigned char *pColorBuffer)
{
  // get the core material
  boost::shared_ptr<CalCoreMaterial> pCoreMaterial = submesh->getMaterial();
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
  CalCoreMaterial::Color& color = pCoreMaterial->getSpecularColor();

  // write it to the color buffer
  pColorBuffer[0] = color.red;
  pColorBuffer[1] = color.green;
  pColorBuffer[2] = color.blue;
  pColorBuffer[3] = color.alpha;
}

 /*****************************************************************************/
/** Provides access to the texture coordinate data.
  *
  * This function returns the texture coordinate data for a given map of the
  * selected mesh/submesh.
  *
  * @param mapId The ID of the map to get the texture coordinate data from.
  * @param pTextureCoordinateBuffer A pointer to the user-provided buffer where
  *                    the texture coordinate data is written to.
  *
  * @return The number of texture coordinates written to the buffer.
  *****************************************************************************/

int CalRenderer::getTextureCoordinates(CalSubmesh* submesh, int mapId, float *pTextureCoordinateBuffer)
{
  // get the texture coordinate vector vector
  std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> >& vectorvectorTextureCoordinate = submesh->getCoreSubmesh()->getVectorVectorTextureCoordinate();

  // check if the map id is valid
  if((mapId < 0) || (mapId >= (int)vectorvectorTextureCoordinate.size()))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    return -1;
  }

  // get the number of texture coordinates to return
  int textureCoordinateCount = submesh->getVertexCount();

  // copy the texture coordinate vector to the face buffer
  if (textureCoordinateCount) {
    memcpy(pTextureCoordinateBuffer, &vectorvectorTextureCoordinate[mapId][0], textureCoordinateCount * sizeof(CalCoreSubmesh::TextureCoordinate));
  }

  return textureCoordinateCount;
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
CalRenderer::textureCoordinatesForMapValid(CalSubmesh* submesh, int mapId )
{
  std::vector<std::vector<CalCoreSubmesh::TextureCoordinate> >& vectorvectorTextureCoordinate = submesh->getCoreSubmesh()->getVectorVectorTextureCoordinate();
  if((mapId < 0) || (mapId >= (int)vectorvectorTextureCoordinate.size())) {
    return false;
  }
  return true;
}


// Returns the in-memory 32 bit representation of a color ("StandardPixel"),
// which may be different on different machines depending on endedness and
// on graphics format convention.  Alpha is 0xff.
//
//    Win32 is low byte first, ARGB8 (i.e., the first byte is B, the second is G).
//
int CalRenderer::getVertColorsAsStandardPixels(CalSubmesh* submesh, unsigned long *pVertexBuffer)
{
  // get the number of vertices
  int vertexCount;
  vertexCount = submesh->getVertexCount();

  // get vertex vector of the core submesh
  std::vector<CalCoreSubmesh::Vertex>& vectorVertex = submesh->getCoreSubmesh()->getVectorVertex();

  int vertexId;
  for(vertexId = 0; vertexId < vertexCount; ++vertexId)
  {
    // get the vertex
    CalCoreSubmesh::Vertex& vertex = vectorVertex[vertexId];
    unsigned long color;

#ifdef WIN32
    // Win32 StandardPixels are ARGB8 with low byte first, which means BGRA byte order.
    color =
      ( unsigned int ) ( vertex.vertexColor.z * 0xff )
      + ( ( ( unsigned int ) ( vertex.vertexColor.y * 0xff ) ) << 8 )
      + ( ( ( unsigned int ) ( vertex.vertexColor.x * 0xff ) ) << 16 )
      + 0xff000000;
#else
    color = 
        ( ( ( unsigned int ) ( vertex.vertexColor.x * 0xff ) ) << 24)
      | ( ( ( unsigned int ) ( vertex.vertexColor.y * 0xff ) ) << 16)
      | ( ( ( unsigned int ) ( vertex.vertexColor.z * 0xff ) ) << 8)
      | 0x000000ff;
#endif

    *pVertexBuffer++ = color;
  }
  return vertexCount;
}
