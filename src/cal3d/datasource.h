//****************************************************************************//
// datasource.h                                                              //
// Copyright (C) 2001-2003 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <iosfwd>
#include <string>
#include "cal3d/global.h"

namespace CalPlatform {
    CAL3D_API bool readBytes(char* input, void* pBuffer, int length);
    CAL3D_API bool readFloat(char* input, float& value);
    CAL3D_API bool readInteger(char* input, int& value);
    CAL3D_API bool readString(char* input, std::string& strValue);

    CAL3D_API bool writeBytes(std::ostream& output, const void* pBuffer, int length);
    CAL3D_API bool writeFloat(std::ostream& output, float value);
    CAL3D_API bool writeInteger(std::ostream& output, int value);
    CAL3D_API bool writeString(std::ostream& output, const std::string& strValue);
};
