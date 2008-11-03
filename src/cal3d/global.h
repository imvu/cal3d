//****************************************************************************//
// global.h                                                                   //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include "cal3d/platform.h"

typedef unsigned short CalIndex;

namespace Cal {

    // global typedefs
    struct UserData {
        virtual ~UserData() {}
    };

    // file magic cookies
    const char SKELETON_FILE_MAGIC[4]  = { 'C', 'S', 'F', '\0' };
    const char ANIMATION_FILE_MAGIC[4] = { 'C', 'A', 'F', '\0' };
    const char ANIMATEDMORPH_FILE_MAGIC[4] = { 'C', 'P', 'F', '\0' };
    const char MESH_FILE_MAGIC[4]      = { 'C', 'M', 'F', '\0' };
    const char MATERIAL_FILE_MAGIC[4]  = { 'C', 'R', 'F', '\0' };


    // library version
    const int LIBRARY_VERSION = 919;

    // file versions
    const int CURRENT_FILE_VERSION = LIBRARY_VERSION;
    const int EARLIEST_COMPATIBLE_FILE_VERSION = 699;

    const int FIRST_FILE_VERSION_WITH_ANIMATION_COMPRESSION6 = 918;
    const int FIRST_FILE_VERSION_WITH_ANIMATION_COMPRESSION5 = 917;
    const int FIRST_FILE_VERSION_WITH_ANIMATION_COMPRESSION4 = 916;
    const int FIRST_FILE_VERSION_WITH_ANIMATION_COMPRESSION = 913;
    const int FIRST_FILE_VERSION_WITH_VERTEX_COLORS = 911;
    const int FIRST_FILE_VERSION_WITH_NODE_LIGHTS = 911;
    const int FIRST_FILE_VERSION_WITH_MATERIAL_TYPES = 911;
    const int FIRST_FILE_VERSION_WITH_MORPH_TARGETS_IN_MORPH_FILES = 911;

    inline bool versionHasCompressionFlag(int version) {
        return version >= 919;
    }

    template<typename T>
    const T* pointerFromVector(const std::vector<T>& v) {
        if (v.empty()) {
            return 0;
        } else {
            return &v[0];
        }
    }

};

struct CalHeader {
    int version;
    char const* magic;
};
