//****************************************************************************//
// global.h                                                                   //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifndef CAL_GLOBAL_H
#define CAL_GLOBAL_H


// platform dependent includes
#include "cal3d/platform.h"

//****************************************************************************//
// Define options
//****************************************************************************//

//Uncomment this if you want to use 16bit indices or configure the compiler

//#define CAL_16BIT_INDICES

#ifdef CAL_16BIT_INDICES 
typedef unsigned short CalIndex; 
#else 
typedef int CalIndex; 
#endif


//****************************************************************************//
// Global Cal3D namespace for constants, ...                                  //
//****************************************************************************//

namespace Cal
{  
  // global typedefs
  typedef void *UserData;

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

};

struct VertexComponentReceiver {
    void* data;
    unsigned int stride;
};

template<typename T>
const T* pointerFromVector(const std::vector<T>& v) {
    if (v.empty()) {
        return 0;
    } else {
        return &v[0];
    }
}

inline bool float_epsilonEquals(float a, float b, float epsilon=0.001) {
    float d = a-b;    
    if(d<0) d =-d;
    return d < epsilon;
}

template<typename T>
void enlargeStdVectorCache(std::vector<T>& v, size_t size) {
    size_t dest_size = size;
    if(v.capacity() < size) {
        dest_size = (size_t)(size * 1.414f);
    }
    if(v.size() < dest_size) {
        v.resize(dest_size);
    }
}

struct CalHeader {
    int version;
    char const* magic;
};

#endif
