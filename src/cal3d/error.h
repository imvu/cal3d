//****************************************************************************//
// error.h                                                                    //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <string>
#include "cal3d/global.h"

#ifdef _MSC_VER
#define IMVU_THREAD_LOCAL __declspec(thread)
#else
#define IMVU_THREAD_LOCAL __thread
#endif

class CAL3D_API CalError {
public:
    enum Code {
        OK = 0,
        INTERNAL,
        INVALID_HANDLE,
        MEMORY_ALLOCATION_FAILED,
        FILE_NOT_FOUND,
        INVALID_FILE_FORMAT,
        FILE_PARSER_FAILED,
        INDEX_BUILD_FAILED,
        NO_PARSER_DOCUMENT,
        INVALID_ANIMATION_DURATION,
        BONE_NOT_FOUND,
        INVALID_ATTRIBUTE_VALUE,
        INVALID_KEYFRAME_COUNT,
        INVALID_ANIMATION_TYPE,
        FILE_CREATION_FAILED,
        FILE_WRITING_FAILED,
        INCOMPATIBLE_FILE_VERSION,
        NO_MESH_IN_MODEL,
        BAD_DATA_SOURCE,
        NULL_BUFFER,
        INVALID_MIXER_TYPE,
        MAX_ERROR_CODE
    };

    static Code getLastErrorCode();
    static char const* getLastErrorDescription();
    static char const* getLastErrorFile();
    static char const* getLastErrorText();

    static const char* getLastErrorDescriptionInternal();
    static const std::string& getLastErrorFileInternal();
    static const std::string& getLastErrorTextInternal();
    static int getLastErrorLine();
    static void setLastError(Code code, const std::string& strFile, int line, const std::string& strText = "");

private:
    struct ErrorState;
    static ErrorState& getErrorState();
};
