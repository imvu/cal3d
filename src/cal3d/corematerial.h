//****************************************************************************//
// corematerial.h                                                             //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"

class CAL3D_API CalCoreMaterial : public Cal::UserDataHolder
{
// misc
public:
  /// The core material Color.
  typedef struct
  {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
  } Color;

  /// The core material Map.
  typedef struct
  {
    std::string strFilename;
    std::string mapType;
  } Map;

private:
  Color m_ambientColor;
  Color m_diffuseColor;
  Color m_specularColor;
  float m_shininess;
  std::vector<Map> m_vectorMap;

public:
  Color& getAmbientColor();
  Color& getDiffuseColor();
  int getMapCount() const;
  const std::string& getMapFilename(int mapId) const;
  const std::string& getMapType(int mapId) const;
  float getShininess();
  Color& getSpecularColor();
  std::vector<Map>& getVectorMap();
  void reserve(int mapCount);
  void setAmbientColor(const Color& ambientColor);
  void setDiffuseColor(const Color& diffuseColor);
  bool setMap(int mapId, const Map& map);
  void setShininess(float shininess);
  void setSpecularColor(const Color& specularColor);
  bool getTwoSided() const { return getMapCount() > 1; } // Should come from check box.
};
