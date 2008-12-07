//****************************************************************************//
// skeleton.h                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <boost/noncopyable.hpp>
#include "cal3d/global.h"
#include "cal3d/matrix.h"
#include "cal3d/vector.h"

class CalCoreSkeleton;
class CalCoreModel;
class CalBone;

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
      T* new_data = reinterpret_cast<T*>(_aligned_malloc(sizeof(T) * new_size, 16));
      if (!new_data) {
        throw std::bad_alloc();
      }

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

  size_t size() const {
    return _size;
  }

  T* data;

private:
  size_t _size;
};

class CAL3D_API CalSkeleton
{
public:
  // Cal3D uses a 3x3 transform matrix and a translation 3-vector. We don't
  // use w in any of these, but we need to be 16-byte aligned for SSE.
  struct BoneTransform {
    CalVector4 colx;
    CalVector4 coly;
    CalVector4 colz;
    CalVector4 translation;
  };

  CalSkeleton(CalCoreSkeleton* pCoreSkeleton);
  ~CalSkeleton();

  void calculateState();
  void clearState();
  CalBone *getBone(int boneId);
  CalCoreSkeleton *getCoreSkeleton();
  std::vector<CalBone *>& getVectorBone();
  void lockState();

  SSEArray<BoneTransform> boneTransforms;

private:
  CalCoreSkeleton *m_pCoreSkeleton;
  std::vector<CalBone *> m_vectorBone;
};
