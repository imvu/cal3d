//****************************************************************************//
// buffersource.h                                                            //
// Copyright (C) 2001-2003 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/global.h"
#include "cal3d/datasource.h"

class CAL3D_API CalBufferSource {
public:
    CalBufferSource(const void* inputBuffer, size_t length);

    bool readInteger(unsigned& value) {
        int i;
        bool rv = readInteger(i);
        value = i;
        return rv;
    }

    bool readBytes(void* pBuffer, int length);
    bool readFloat(float& value);
    bool readInteger(int& value);
    bool readString(std::string& strValue);

    const void* data() const {
        return mInputBuffer;
    }
    size_t size() const {
        return mLength;
    }

private:
    const void* mInputBuffer;
    size_t mOffset;
    size_t mLength;

    CalBufferSource(); //Can't use this
};
