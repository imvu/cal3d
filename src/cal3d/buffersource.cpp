//****************************************************************************//
// buffersource.h                                                            //
// Copyright (C) 2001-2003 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include "cal3d/buffersource.h"
#include "cal3d/error.h"

CalBufferSource::CalBufferSource(const void* inputBuffer, size_t length)
  : mInputBuffer(inputBuffer)
  , mOffset(0)
  , mLength(length)
{
  assert(inputBuffer || length == 0);
}


void CalBufferSource::setError() const
{
   CalError::setLastError(CalError::NULL_BUFFER, __FILE__, __LINE__);
}

bool CalBufferSource::readBytes(void* pBuffer, int length)
{
   //Check that the buffer and the target are usable
   if (!ok() || (pBuffer == NULL)) return false;
   if (mOffset + length > mLength) return false;
   
   bool result = CalPlatform::readBytes( ((char*)mInputBuffer+mOffset), pBuffer, length );
   mOffset += length;

   return result;
}

bool CalBufferSource::readFloat(float& value)
{
   //Check that the buffer is usable
   if (!ok()) return false;
   if (mOffset + 4 > mLength) return false;

   bool result = CalPlatform::readFloat( ((char*)mInputBuffer+mOffset), value );
   mOffset += 4;

   return result;
}

bool CalBufferSource::readInteger(int& value)
{
   //Check that the buffer is usable
   if (!ok()) return false;
   if (mOffset + 4 > mLength) return false;

   bool result = CalPlatform::readInteger( ((char*)mInputBuffer+mOffset), value );
   mOffset += 4;

   return result;
}

bool CalBufferSource::readString(std::string& strValue)
{
   //Check that the buffer is usable
   if (!ok()) return false;

   bool result = CalPlatform::readString( ((char*)mInputBuffer+mOffset), strValue );

   mOffset += (strValue.length() + 4 + 1); // +1 is for Null-terminator

   // May still have read the string that read beyond the end.
   if(mOffset > mLength) return false;   
   return result;
}
