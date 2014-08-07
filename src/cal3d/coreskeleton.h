//****************************************************************************//
// coreskeleton.h                                                             //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <set>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"
#include "cal3d/vector.h"
#include "cal3d/transform.h"

CAL3D_PTR(CalCoreBone);
class CalQuaternion;

class CAL3D_API CalCoreSkeleton : private boost::noncopyable {
public:
    CalCoreSkeleton(const std::vector<CalCoreBonePtr>& bones = std::vector<CalCoreBonePtr>());

    size_t addCoreBone(const CalCoreBonePtr& coreBone);
    int getBoneId(const CalCoreBone* coreBone) const;

    void scale(float factor);

    cal3d::RotateTranslate getAdjustedRootTransform(size_t boneIndex) const;

    const std::vector<CalCoreBonePtr>& coreBones; // points to the internal m_coreBones
    const std::vector<CalCoreBonePtr>& getCoreBones() const {
        return coreBones;
    }

    void rotateTranslate(cal3d::RotateTranslate &rt);
    void rotate(CalQuaternion& rot);

    std::vector<unsigned> boneIdTranslation; // mapping from exported-to-file ID to the in-memory index

    std::vector<int> getChildIds(const CalCoreBone* coreBone) const;

    CalVector sceneAmbientColor;

private:
    cal3d::RotateTranslate inverseOriginalRootTransform;
    std::set<size_t> adjustedRoots;
    std::vector<CalCoreBonePtr> m_coreBones;
};

CAL3D_PTR(CalCoreSkeleton);
