//****************************************************************************//
// coretrack.h                                                                //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <boost/shared_ptr.hpp>
#include "cal3d/corekeyframe.h"
#include "cal3d/global.h"
#include "cal3d/transform.h"

CAL3D_PTR(CalCoreBone);
CAL3D_PTR(CalCoreTrack);
class CalCoreSkeleton;
class CalQuaternion;

class CAL3D_API CalCoreTrack {
public:
    typedef std::vector<CalCoreKeyframe> KeyframeList;

    // would be const, except we store a std::vector of these
    unsigned coreBoneId;
    KeyframeList keyframes;

    bool translationRequired;
    bool translationIsDynamic;

    CalCoreTrack(int coreBoneId, const KeyframeList& keyframes);

    size_t sizeInBytes() const;
    void scale(float factor);
    void optimize();
    void zeroTransforms();
    void fixup(
        const CalCoreBone& bone,
        const cal3d::RotateTranslate& adjustedRootTransform);
    void rotateTranslate(cal3d::RotateTranslate &rt);

    cal3d::RotateTranslate getCurrentTransform(float time) const;

    CalCoreTrackPtr compress(double translationTolerance, double rotationToleranceDegrees, CalCoreSkeleton* skelOrNull) const;
    void translationCompressibility(
        bool* transRequiredResult, bool* transDynamicResult,
        float threshold, CalCoreSkeleton* skel
    ) const;

private:
    KeyframeList::const_iterator getUpperBound(float time) const;
};
CAL3D_PTR(CalCoreTrack);

inline bool operator==(const CalCoreTrack& lhs, const CalCoreTrack& rhs) {
    return lhs.coreBoneId == rhs.coreBoneId &&
           lhs.keyframes == rhs.keyframes;
}
