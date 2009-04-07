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

#include <iostream>
#include <string>
#include "cal3d/global.h"

/**
 * CalDataSource abstract interface class.
 *
 * This is an abstract class designed to represent a source of Cal3d data,
 * whether it is an ifstream, istream, or even a memory buffer. Inheriting
 * classes must implement the 'read' functions below.
 */

class CAL3D_API CalDataSource : public Cal::Object
{
public:
  // So...  I started making some refactorings to the implementations of DataSource.
  // It turns out all of the implementations of ok() would only return true.  Since they
  // all did the same thing, I lifted the implementation into the base class.  Now, since
  // it is always true, we can eliminate it.  I think that it is better to always check the
  // return values of the read*() functions, anyway, as ok()'s meaning is unclear (can we
  // read in the future?  did a read fail in the past?)
  //
  // When this ok() call is removed, make sure to check that any nearby read*() calls are
  // actually testing the return value.
  bool ok() { return true; }
  virtual void setError() const = 0;
  virtual bool readBytes(void* pBuffer, int length) = 0;
  virtual bool readFloat(float& value) = 0;
  virtual bool readInteger(int& value) = 0;
  virtual bool readString(std::string& strValue) = 0;

  bool readInteger(unsigned& value) {
    int i;
    bool rv = readInteger(i);
    value = i;
    return rv;
  }
};

namespace CalPlatform
{
  CAL3D_API bool readBytes(std::istream& input, void *pBuffer, int length);
  CAL3D_API bool readFloat(std::istream& input, float& value);
  CAL3D_API bool readInteger(std::istream& input, int& value);
  CAL3D_API bool readString(std::istream& input, std::string& strValue);

  CAL3D_API bool readBytes(char* input, void *pBuffer, int length);
  CAL3D_API bool readFloat(char* input, float& value);
  CAL3D_API bool readInteger(char* input, int& value);
  CAL3D_API bool readString(char* input, std::string& strValue);

  CAL3D_API bool writeBytes(std::ostream& output, const void *pBuffer, int length);
  CAL3D_API bool writeFloat(std::ostream& output, float value);
  CAL3D_API bool writeInteger(std::ostream& output, int value);
  CAL3D_API bool writeString(std::ostream& output, const std::string& strValue);
};
