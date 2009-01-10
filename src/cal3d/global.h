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

#include <boost/noncopyable.hpp>
#include <vector>
#include "cal3d/platform.h"

typedef unsigned short CalIndex;


void* allocate_aligned_data(size_t size);

// Can't use std::vector w/ __declspec(align(16)) :(
// http://ompf.org/forum/viewtopic.php?f=11&t=686
// http://social.msdn.microsoft.com/Forums/en-US/vclanguage/thread/0adabdb5-f732-4db7-a8de-e3e83af0e147/
template<typename T>
struct SSEArray : boost::noncopyable {
  SSEArray()
    : data(0)
    , _size(0)
  {}

  ~SSEArray() {
    _aligned_free(data);
  }

  // destructive
  void resize(size_t new_size) {
    if (_size != new_size) {
      T* new_data = reinterpret_cast<T*>(allocate_aligned_data(sizeof(T) * new_size));

      if (data) {
        _aligned_free(data);
      }
      _size = new_size;
      data = new_data;
    }
  }

  T& operator[](size_t idx) {
    return data[idx];
  }

  const T& operator[](size_t idx) const {
    return data[idx];
  }

  size_t size() const {
    return _size;
  }

  T* data;

private:
  size_t _size;
};


namespace Cal {

  struct UserData {
    virtual ~UserData() {}
  };

  struct CAL3D_API UserDataHolder {
  public:
    UserDataHolder() 
    : m_userData(0)
    { }

    ~UserDataHolder() {
        delete m_userData;
    }

    void setUserData(UserData* userData) {
        delete m_userData;
        m_userData = userData;
    }

    UserData* getUserData() const {
        return m_userData;
    }

  private:
      UserData* m_userData;
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

  template<typename T, typename A>
  T* pointerFromVector(std::vector<T, A>& v) {
      if (v.empty()) {
          return 0;
      } else {
          return &v[0];
      }
  }

  template<typename T, typename A>
  const T* pointerFromVector(const std::vector<T, A>& v) {
      if (v.empty()) {
          return 0;
      } else {
          return &v[0];
      }
  }

  template<typename T>
  const T* pointerFromVector(const SSEArray<T>& v) {
    return v.data;
  }

};

struct CalHeader {
  int version;
  char const* magic;
};
