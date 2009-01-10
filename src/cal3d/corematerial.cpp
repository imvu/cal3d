//****************************************************************************//
// corematerial.cpp                                                           //
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

#include "cal3d/error.h"
#include "cal3d/corematerial.h"

 /*****************************************************************************/
/** Returns the ambient color.
  *
  * This function returns the ambient color of the core material instance.
  *
  * @return A reference to the ambient color.
  *****************************************************************************/

CalCoreMaterial::Color& CalCoreMaterial::getAmbientColor()
{
  return m_ambientColor;
}

 /*****************************************************************************/
/** Returns the diffuse color.
  *
  * This function returns the diffuse color of the core material instance.
  *
  * @return A reference to the diffuse color.
  *****************************************************************************/

CalCoreMaterial::Color& CalCoreMaterial::getDiffuseColor()
{
  return m_diffuseColor;
}

 /*****************************************************************************/
/** Returns the number of maps.
  *
  * This function returns the number of mapss in the core material instance.
  *
  * @return The number of maps.
  *****************************************************************************/

int CalCoreMaterial::getMapCount() const
{
  return m_vectorMap.size();
}

 /*****************************************************************************/
/** Returns a specified map texture filename.
  *
  * This function returns the texture filename for a specified map ID of the
  * core material instance.
  *
  * @param mapId The ID of the map.
  *
  * @return One of the following values:
  *         \li the filename of the map texture
  *         \li an empty string if an error happend
  *****************************************************************************/

const std::string& CalCoreMaterial::getMapFilename(int mapId) const
{
  // check if the map id is valid
  if((mapId < 0) || (mapId >= (int)m_vectorMap.size()))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    static std::string null;
    return null;
  }

  return m_vectorMap[mapId].strFilename;
}

 /*****************************************************************************/
/** Returns a specified map type
  *
  * This function returns the map type for a specified map ID of the
  * core material instance. The type will be an exporter-specific string which
  * explains what the Map is meant to be used for, such as "Opacity"
  *
  * @param mapId The ID of the map.
  *
  * @return One of the following values:
  *         \li the type of the map 
  *         \li an empty string if an error happend
  *****************************************************************************/

const std::string& CalCoreMaterial::getMapType(int mapId) const
{
  // check if the map id is valid
  if((mapId < 0) || (mapId >= (int)m_vectorMap.size()))
  {
    CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__);
    static std::string null;
    return null;
  }

  return m_vectorMap[mapId].mapType;
}

 /*****************************************************************************/
/** Returns the shininess factor.
  *
  * This function returns the shininess factor of the core material instance.
  *
  * @return The shininess factor.
  *****************************************************************************/

float CalCoreMaterial::getShininess()
{
  return m_shininess;
}

 /*****************************************************************************/
/** Returns the specular color.
  *
  * This function returns the specular color of the core material instance.
  *
  * @return A reference to the specular color.
  *****************************************************************************/

CalCoreMaterial::Color& CalCoreMaterial::getSpecularColor()
{
  return m_specularColor;
}

 /*****************************************************************************/
/** Returns the map vector.
  *
  * This function returns the vector that contains all maps of the core material
  * instance.
  *
  * @return A reference to the map vector.
  *****************************************************************************/

std::vector<CalCoreMaterial::Map>& CalCoreMaterial::getVectorMap()
{
  return m_vectorMap;
}

 /*****************************************************************************/
/** Reserves memory for the maps.
  *
  * This function reserves memory for the maps of the core material instance.
  *
  * @param mapCount The number of maps that this core material instance should
  *                 be able to hold.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

void CalCoreMaterial::reserve(int mapCount)
{
  m_vectorMap.resize(mapCount);
}

 /*****************************************************************************/
/** Sets the ambient color.
  *
  * This function sets the ambient color of the core material instance.
  *
  * @param ambientColor The ambient color that should be set.
  *****************************************************************************/

void CalCoreMaterial::setAmbientColor(const CalCoreMaterial::Color& ambientColor)
{
  m_ambientColor = ambientColor;
}

 /*****************************************************************************/
/** Sets the diffuse color.
  *
  * This function sets the diffuse color of the core material instance.
  *
  * @param ambientColor The diffuse color that should be set.
  *****************************************************************************/

void CalCoreMaterial::setDiffuseColor(const CalCoreMaterial::Color& diffuseColor)
{
  m_diffuseColor = diffuseColor;
}

 /*****************************************************************************/
/** Sets a specified map.
  *
  * This function sets a specified map in the core material instance.
  *
  * @param mapId  The ID of the map.
  * @param map The map that should be set.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalCoreMaterial::setMap(int mapId, const Map& map)
{
  if((mapId < 0) || (mapId >= (int)m_vectorMap.size())) return false;

  m_vectorMap[mapId] = map;

  return true;
}

 /*****************************************************************************/
/** Sets the shininess factor.
  *
  * This function sets the shininess factor of the core material instance.
  *
  * @param shininess The shininess factor that should be set.
  *****************************************************************************/

void CalCoreMaterial::setShininess(float shininess)
{
  m_shininess = shininess;
}

 /*****************************************************************************/
/** Sets the specular color.
  *
  * This function sets the specular color of the core material instance.
  *
  * @param ambientColor The specular color that should be set.
  *****************************************************************************/

void CalCoreMaterial::setSpecularColor(const CalCoreMaterial::Color& specularColor)
{
  m_specularColor = specularColor;
}
