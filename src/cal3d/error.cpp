//****************************************************************************//
// error.cpp                                                                  //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#include "cal3d/error.h"

#ifdef _MSC_VER
#include <windows.h>
#else
#include <pthread.h>
#endif

struct CalError::ErrorState {
    ErrorState()
        : lastErrorCode(CalError::OK)
        , lastErrorLine(-1)
    {}

    Code lastErrorCode;
    std::string strLastErrorFile;
    int lastErrorLine;
    std::string strLastErrorText;
};

#ifdef _MSC_VER

static DWORD s_errorStateKey = TlsAlloc();

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_THREAD_DETACH) {
        delete reinterpret_cast<CalError::ErrorState*>(TlsGetValue(s_errorStateKey));
    }
    return TRUE;
}

CalError::ErrorState& CalError::getErrorState() {
    void* state = TlsGetValue(s_errorStateKey);
    if (!state) {
        state = new ErrorState;
        TlsSetValue(s_errorStateKey, state);
    }
    return *reinterpret_cast<ErrorState*>(state);
}

#else

static void destroyErrorState(void* state) {
    delete reinterpret_cast<CalError::ErrorState*>(state);
}

static pthread_key_t s_errorStateKey;
static int s_errorStateKeyResult = pthread_key_create(&s_errorStateKey, &destroyErrorState);

CalError::ErrorState& CalError::getErrorState() {
    void* state = pthread_getspecific(s_errorStateKey);
    if (!state) {
        state = new ErrorState;
        pthread_setspecific(s_errorStateKey, state);
    }
    return *reinterpret_cast<ErrorState*>(state);
}

#endif

CalError::Code CalError::getLastErrorCode() {
    return getErrorState().lastErrorCode;
}

const char* CalError::getLastErrorDescriptionInternal() {
    switch (getLastErrorCode()) {
        case OK:
            return "No error found";
            break;
        case INTERNAL:
            return "Internal error";
            break;
        case INVALID_HANDLE:
            return "Invalid handle as argument";
            break;
        case MEMORY_ALLOCATION_FAILED:
            return "Memory allocation failed";
            break;
        case FILE_NOT_FOUND:
            return "File not found";
            break;
        case INVALID_FILE_FORMAT:
            return "Invalid file format";
            break;
        case FILE_PARSER_FAILED:
            return "Parser failed to process file";
            break;
        case INDEX_BUILD_FAILED:
            return "Building of the index failed";
            break;
        case NO_PARSER_DOCUMENT:
            return "There is no document to parse";
            break;
        case INVALID_ANIMATION_DURATION:
            return "The duration of the animation is invalid";
            break;
        case BONE_NOT_FOUND:
            return "Bone not found";
            break;
        case INVALID_ATTRIBUTE_VALUE:
            return "Invalid attribute value";
            break;
        case INVALID_KEYFRAME_COUNT:
            return "Invalid number of keyframes";
            break;
        case INVALID_ANIMATION_TYPE:
            return "Invalid animation type";
            break;
        case FILE_CREATION_FAILED:
            return "Failed to create file";
            break;
        case FILE_WRITING_FAILED:
            return "Failed to write to file";
            break;
        case INCOMPATIBLE_FILE_VERSION:
            return "Incompatible file version";
            break;
        case NO_MESH_IN_MODEL:
            return "No mesh attached to the model";
            break;
        case BAD_DATA_SOURCE:
            return "Cannot read from data source";
            break;
        case NULL_BUFFER:
            return "Memory buffer is null";
            break;
        case INVALID_MIXER_TYPE:
            return "The CalModel mixer is not a CalMixer instance";
            break;
        default:
            break;
    }

    return "Unknown error";
}

const std::string& CalError::getLastErrorFileInternal() {
    return getErrorState().strLastErrorFile;
}

int CalError::getLastErrorLine() {
    return getErrorState().lastErrorLine;
}

const std::string& CalError::getLastErrorTextInternal() {
    return getErrorState().strLastErrorText;
}

char const* CalError::getLastErrorDescription() {
    return getLastErrorDescriptionInternal();
}

char const* CalError::getLastErrorFile() {
    return getLastErrorFileInternal().c_str();
}

char const* CalError::getLastErrorText() {
    return getLastErrorTextInternal().c_str();
}

void CalError::setLastError(Code code, const std::string& strFile, int line, const std::string& strText) {
    if (code >= MAX_ERROR_CODE) {
        code = INTERNAL;
    }

    ErrorState& es = getErrorState();

    es.lastErrorCode = code;
    es.strLastErrorFile = strFile.c_str();
    es.lastErrorLine = line;
    es.strLastErrorText = strText.c_str();
}
