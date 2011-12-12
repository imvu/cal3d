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

#include <stdexcept>
#include <math.h>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <list>
#include <map>
#include "cal3d/platform.h"

typedef unsigned short CalIndex;

const float EPSILON = 1e-5f;

#define CAL3D_PTR(c) typedef boost::shared_ptr<class c> c##Ptr

namespace cal3d {
    inline bool close(float a, float b) {
        return fabs(a - b) < EPSILON;
    }

    inline float clamp(float v, float min, float max) {
        if (v > max) {
            return max;
        } else if (v < min) {
            return min;
        } else {
            return v;
        }
    }
}

namespace cal3d {

    const char SKELETON_FILE_MAGIC[4]  = { 'C', 'S', 'F', '\0' };
    const char ANIMATION_FILE_MAGIC[4] = { 'C', 'A', 'F', '\0' };
    const char ANIMATEDMORPH_FILE_MAGIC[4] = { 'C', 'P', 'F', '\0' };
    const char MESH_FILE_MAGIC[4]      = { 'C', 'M', 'F', '\0' };
    const char MATERIAL_FILE_MAGIC[4]  = { 'C', 'R', 'F', '\0' };

    const int LIBRARY_VERSION = 919;

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

// Boo OS X
#ifdef verify
#undef verify
#endif

    inline void verify(bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }
};

namespace cal3d {
    template<typename V, typename K>
    V map_get(const std::map<K, V>& m, const K& key) {
        typename std::map<K, V>::const_iterator i = m.find(key);
        if (i == m.end()) {
            return V();
        } else {
            return i->second;
        }
    }

    template<typename V, typename K>
    V* map_get_ptr(std::map<K, V>& m, const K& key) {
        typename std::map<K, V>::iterator i = m.find(key);
        if (i == m.end()) {
            return 0;
        } else {
            return &i->second;
        }
    }

    template<typename V, typename K>
    const V* map_get_ptr(const std::map<K, V>& m, const K& key) {
        typename std::map<K, V>::const_iterator i = m.find(key);
        if (i == m.end()) {
            return 0;
        } else {
            return &i->second;
        }
    }
}

