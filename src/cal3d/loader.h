//****************************************************************************//
// loader.h                                                                   //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#pragma once

#include <boost/shared_ptr.hpp>
#include <math.h>
#include <string>
#include "cal3d/global.h"
#include "cal3d/datasource.h"
#include "cal3d/tinyxml.h"

CAL3D_PTR(CalCoreSkeleton);
CAL3D_PTR(CalCoreBone);
CAL3D_PTR(CalCoreAnimation);
CAL3D_PTR(CalCoreMorphAnimation);
CAL3D_PTR(CalCoreTrack);
CAL3D_PTR(CalCoreKeyframe);
CAL3D_PTR(CalCoreMorphTrack);
CAL3D_PTR(CalCoreMorphKeyframe);
CAL3D_PTR(CalCoreMesh);
CAL3D_PTR(CalCoreSubmesh);
CAL3D_PTR(CalCoreMaterial);
class CalVector;
class CalQuaternion;
class CalBufferSource;

namespace rapidxml {
    template<typename T>
    class xml_document;

    template<typename T>
    class xml_node;
}

bool CAL3D_API CalVectorFromDataSrc(CalBufferSource& dataSrc, CalVector* calVec);

class CAL3D_API CalLoader {
public:
    static unsigned int const keyframeBitsPerOriComponent;
    static unsigned int const keyframeBitsPerTime;

    static unsigned int const keyframeBitsPerUnsignedPosComponent;
    static unsigned int const keyframeBitsPerPosPadding;
    static float const keyframePosRange;
    static unsigned int const keyframePosBytes;

    static unsigned int const keyframeBitsPerUnsignedPosComponentSmall;
    static unsigned int const keyframeBitsPerPosPaddingSmall;
    static float const keyframePosRangeSmall;
    static unsigned int const keyframePosBytesSmall;

    static CalCoreAnimationPtr loadCoreAnimation(CalBufferSource& inputSrc);
    static CalCoreMorphAnimationPtr loadCoreMorphAnimation(CalBufferSource& inputSrc);
    static CalCoreMaterialPtr loadCoreMaterial(CalBufferSource& inputSrc);
    static CalCoreMeshPtr loadCoreMesh(CalBufferSource& inputSrc);
    static CalCoreSkeletonPtr loadCoreSkeleton(CalBufferSource& inputSrc);

private:
    static CalCoreAnimationPtr loadBinaryCoreAnimation(CalBufferSource& inputSrc);
    static CalCoreMorphAnimationPtr loadBinaryCoreMorphAnimation(CalBufferSource& inputSrc);
    static CalCoreMaterialPtr loadBinaryCoreMaterial(CalBufferSource& inputSrc);
    static CalCoreMeshPtr loadBinaryCoreMesh(CalBufferSource& inputSrc);
    static CalCoreSkeletonPtr loadBinaryCoreSkeleton(CalBufferSource& inputSrc);

    static CalCoreAnimationPtr loadXmlCoreAnimation(char*);
    static CalCoreAnimationPtr loadXmlCoreAnimationDoc(TiXmlDocument& doc);

    static CalCoreMorphAnimationPtr loadXmlCoreMorphAnimation(char*);
    static CalCoreMorphAnimationPtr loadXmlCoreMorphAnimationDoc(TiXmlDocument& doc);

    static CalCoreSkeletonPtr loadXmlCoreSkeleton(char*);
    static CalCoreSkeletonPtr loadXmlCoreSkeletonDoc(const rapidxml::xml_document<char>& doc);

    static CalCoreMeshPtr loadXmlCoreMesh(char*);
    static CalCoreMeshPtr loadXmlCoreMeshDoc(const rapidxml::xml_document<char>& doc);

    static CalCoreMaterialPtr loadXmlCoreMaterial(char*);
    static CalCoreMaterialPtr loadXmlCoreMaterialDoc(const rapidxml::xml_document<char>& doc);

    static bool isHeaderWellFormed(const TiXmlElement* header);
    static bool isHeaderWellFormed(const rapidxml::xml_node<char>* node);

    static CalCoreBonePtr loadCoreBones(CalBufferSource& dataSrc, int version);
    static CalCoreKeyframePtr loadCoreKeyframe(CalBufferSource& dataSrc,
            int version, CalCoreKeyframe* lastCoreKeyframe,
            bool translationRequired, bool highRangeRequired, bool translationIsDynamic,
            bool useAnimationCompression);
    static CalCoreMorphKeyframePtr loadCoreMorphKeyframe(CalBufferSource& dataSrc);
    static CalCoreSubmeshPtr loadCoreSubmesh(CalBufferSource& dataSrc, int version);
    static CalCoreTrackPtr loadCoreTrack(CalBufferSource& dataSrc, int version, bool useAnimationCompresssion);
    static CalCoreMorphTrackPtr loadCoreMorphTrack(CalBufferSource& dataSrc);

    static bool usesAnimationCompression(int version);
    static unsigned int compressedKeyframeRequiredBytes(CalCoreKeyframe* lastCoreKeyframe, bool translationRequired, bool highRangeRequired, bool translationIsDynamic);
    static unsigned int readCompressedKeyframe(
        unsigned char* buf,
        CalVector* vecResult, CalQuaternion* quatResult, float* timeResult,
        CalCoreKeyframe* lastCoreKeyframe,
        bool translationRequired, bool highRangeRequired, bool translationIsDynamic);

    CalLoader();
    ~CalLoader();
};


class BitReader {
public:
    BitReader(unsigned char const* source) {
        source_ = source;
        bitsInBuf_ = 0;
        buf_ = 0;
        bytesRead_ = 0;
    }
    inline void read(unsigned int* data, unsigned int numBits);
    inline unsigned int bytesRead() {
        return bytesRead_;
    }
private:
    unsigned int buf_;
    unsigned int bitsInBuf_;
    unsigned int bytesRead_;
    unsigned char const* source_;
};

inline void
BitReader::read(unsigned int* data, unsigned int numBits) {

    // Fill up the buffer with enough bits.  I load 8 at a time, only when
    // needed, so I can never have more than 7 left over.
    assert(bitsInBuf_ < 8);
    while (bitsInBuf_ < numBits) {
        buf_ |= (source_[ bytesRead_ ] << bitsInBuf_);
        bitsInBuf_ += 8;
        bytesRead_++;
    }

    // Read the data.
    * data = buf_ & ((1 << numBits) - 1);
    buf_ >>= numBits;
    bitsInBuf_ -= numBits;
}

inline float
FixedPointToFloatZeroToOne(unsigned int fixedPoint, unsigned int numBits) {
    unsigned int maxVal = (1 << numBits) - 1;
    return (float) fixedPoint / maxVal;
}



// Return the number of bytes read.
inline unsigned int
ReadQuatAndExtra(unsigned char const* data, float* valsResult, unsigned int* extraResult,
                 unsigned int bitsPerComponent, unsigned int bitsPerExtra) {
    BitReader br(data);
    unsigned int bigi;
    br.read(& bigi, 2);
    unsigned int i;
    float sum = 0.0f;
    for (i = 0; i < 4; i++) {
        if (i != bigi) {
            unsigned int sign;
            br.read(& sign, 1);
            unsigned int val;
            br.read(& val, bitsPerComponent);
            float fval = FixedPointToFloatZeroToOne(val, bitsPerComponent);
            valsResult[ i ] = sign ? - fval : fval;
            sum += valsResult[ i ] * valsResult[ i ];
        }
    }
    if (sum > 1.0f) {
        sum = 1.0f;    // Safety for sqrt.
    }
    valsResult[ bigi ] = sqrtf(1.0f - sum);
    br.read(extraResult, bitsPerExtra);
    return br.bytesRead();
}

extern CalVector InvalidTranslation;
