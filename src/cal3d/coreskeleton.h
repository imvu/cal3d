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

#include <list>
#include <map>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "cal3d/global.h"
#include "cal3d/vector.h"

typedef boost::shared_ptr<class CalCoreBone> CalCoreBonePtr;

class CAL3D_API CalCoreSkeleton : private boost::noncopyable {
public:
    CalCoreSkeleton(const std::vector<CalCoreBonePtr>& bones = std::vector<CalCoreBonePtr>());

    size_t addCoreBone(const CalCoreBonePtr& coreBone);

    void scale(float factor);
    void zeroRootTransforms();

    const std::vector<CalCoreBonePtr>& coreBones; // points to the internal m_coreBones
    const std::vector<CalCoreBonePtr>& getCoreBones() const {
        return coreBones;
    }
    std::vector<unsigned> boneIdTranslation; // mapping from exported-to-file ID to the in-memory index

    std::vector<int> getChildIds(const CalCoreBone* coreBone) const;

    CalVector sceneAmbientColor;

private:
    std::vector<CalCoreBonePtr> m_coreBones;
};

typedef boost::shared_ptr<CalCoreSkeleton> CalCoreSkeletonPtr;
