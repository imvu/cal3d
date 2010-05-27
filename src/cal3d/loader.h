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
#include <iosfwd>
#include "cal3d/global.h"
#include "cal3d/datasource.h"
#include "cal3d/tinyxml.h"

class CalCoreSkeleton;
class CalCoreBone;
typedef boost::shared_ptr<class CalCoreAnimation> CalCoreAnimationPtr;
typedef boost::shared_ptr<class CalCoreAnimatedMorph> CalCoreAnimatedMorphPtr;
class CalCoreTrack;
class CalCoreKeyframe;
class CalCoreMorphTrack;
class CalCoreMorphKeyframe;
class CalCoreMesh;
class CalCoreSubmesh;
class CalCoreMaterial;
class CalVector;
class CalQuaternion;

bool CAL3D_API CalVectorFromDataSrc( CalDataSource & dataSrc, CalVector * calVec );

class CAL3D_API CalLoader
{
// member functions
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

  static CalCoreAnimationPtr loadCoreAnimation(const std::string& strFilename, CalCoreSkeleton *skel);
  static CalCoreAnimatedMorphPtr loadCoreAnimatedMorph(const std::string& strFilename);
  static CalCoreMaterial *loadCoreMaterial(const std::string& strFilename);
  static CalCoreMesh *loadCoreMesh(const std::string& strFilename);
  static CalCoreSkeleton *loadCoreSkeleton(const std::string& strFilename);

  static CalCoreAnimationPtr loadCoreAnimation(std::istream& inputStream, CalCoreSkeleton *skel);
  static CalCoreAnimatedMorphPtr loadCoreAnimatedMorph(std::istream& inputStream);
  static CalCoreMaterial *loadCoreMaterial(std::istream& inputStream);
  static CalCoreMesh *loadCoreMesh(std::istream& inputStream);
  static CalCoreSkeleton *loadCoreSkeleton(std::istream& inputStream);

  static CalCoreAnimationPtr loadCoreAnimationFromBuffer(const void* inputBuffer, unsigned int len, CalCoreSkeleton *skel);
  static CalCoreAnimatedMorphPtr loadCoreAnimatedMorphFromBuffer(const void* inputBuffer, unsigned int len);
  static CalCoreMaterial* loadCoreMaterialFromBuffer(const void* inputBuffer, unsigned int len);
  static CalCoreMesh *loadCoreMeshFromBuffer(const void* inputBuffer, unsigned int len);
  static CalCoreSkeleton *loadCoreSkeletonFromBuffer(const void* inputBuffer, unsigned int len);

  static CalCoreAnimationPtr loadCoreAnimation(CalDataSource& inputSrc, CalCoreSkeleton *skel);
  static CalCoreAnimatedMorphPtr loadCoreAnimatedMorph(CalDataSource& inputSrc);
  static CalCoreMaterial *loadCoreMaterial(CalDataSource& inputSrc);
  static CalCoreMesh *loadCoreMesh(CalDataSource& inputSrc);
  static CalCoreSkeleton *loadCoreSkeleton(CalDataSource& inputSrc);

  static void setAnimationTranslationTolerance( double p );
  static void setAnimationRotationToleranceDegrees( double p );

  static double getAnimationTranslationTolerance() { return translationTolerance; }
  static double getAnimationRotationToleranceDegrees() { return rotationToleranceDegrees; }
  static int getAnimationNumEliminatedKeyframes() { return numEliminatedKeyframes; }
  static int getAnimationNumKeptKeyframes() { return numKeptKeyframes; }
  static int getAnimationNumRoundedKeyframes() { return numRoundedKeyframes; }
  static int getAnimationNumCompressedAnimations() { return numCompressedAnimations; }
  static void addAnimationCompressionStatistic( int totalKeyframes, int eliminatedKeyframes, int numRounded ) {
    numEliminatedKeyframes += eliminatedKeyframes;
    numKeptKeyframes += totalKeyframes - eliminatedKeyframes;
    numRoundedKeyframes += numRounded;
    numCompressedAnimations++;
  }
  static void resetCompressionStatistics() {
    numEliminatedKeyframes = 0;
    numKeptKeyframes = 0;
    numCompressedAnimations = 0;
  }
  static bool usesAnimationCompression( int version );
  static unsigned int compressedKeyframeRequiredBytes( CalCoreKeyframe * lastCoreKeyframe, bool translationRequired, bool highRangeRequired, bool translationIsDynamic );
  static unsigned int readCompressedKeyframe(
      unsigned char * buf, CalCoreBone * coreboneOrNull,
      CalVector * vecResult, CalQuaternion * quatResult, float * timeResult,
      CalCoreKeyframe * lastCoreKeyframe,
      bool translationRequired, bool highRangeRequired, bool translationIsDynamic);

  // xmlformat.cpp
  static CalCoreAnimationPtr loadXmlCoreAnimation(const std::string& strFilename, CalCoreSkeleton* skel);
  static CalCoreAnimationPtr loadXmlCoreAnimation(const char*, CalCoreSkeleton* skel);
  static CalCoreAnimationPtr loadXmlCoreAnimation(TiXmlDocument & doc, CalCoreSkeleton* skel);

  static CalCoreAnimatedMorphPtr loadXmlCoreAnimatedMorph(const std::string& strFilename);
  static CalCoreAnimatedMorphPtr loadXmlCoreAnimatedMorph(const void *);
  static CalCoreAnimatedMorphPtr loadXmlCoreAnimatedMorph(TiXmlDocument & doc);

  static CalCoreSkeleton *loadXmlCoreSkeletonFromFile(const std::string& strFilename);
  static CalCoreSkeleton *loadXmlCoreSkeleton(const void *);
  static CalCoreSkeleton *loadXmlCoreSkeleton(TiXmlDocument & doc);

  static CalCoreMesh *loadXmlCoreMesh(const std::string& strFilename);
  static CalCoreMesh *loadXmlCoreMesh(const void *);
  static CalCoreMesh *loadXmlCoreMesh(TiXmlDocument & doc);

  static CalCoreMaterial *loadXmlCoreMaterial(const std::string& strFilename);
  static CalCoreMaterial *loadXmlCoreMaterial(const void *);
  static CalCoreMaterial *loadXmlCoreMaterial(TiXmlDocument & doc);

private:
  static CalCoreBone *loadCoreBones(CalDataSource& dataSrc, int version);
  static CalCoreKeyframe *loadCoreKeyframe(CalDataSource& dataSrc, CalCoreBone * coreboneOrNull, 
                                             int version, CalCoreKeyframe * lastCoreKeyframe, 
                                             bool translationRequired, bool highRangeRequired, bool translationIsDynamic,
                                             bool useAnimationCompression);
  static CalCoreMorphKeyframe *loadCoreMorphKeyframe(CalDataSource& dataSrc);
  static CalCoreSubmesh *loadCoreSubmesh(CalDataSource& dataSrc, int version);
  static CalCoreTrack *loadCoreTrack(CalDataSource & dataSrc, CalCoreSkeleton * skel, int version, bool useAnimationCompresssion);
  static CalCoreMorphTrack *loadCoreMorphTrack(CalDataSource& dataSrc);

  static double translationTolerance;
  static double rotationToleranceDegrees;

  static int numEliminatedKeyframes;
  static int numKeptKeyframes;
  static int numCompressedAnimations;
  static int numRoundedKeyframes;

  CalLoader();
  ~CalLoader();
};


class BitReader {
public:
  BitReader( unsigned char const * source ) {
    source_ = source;
    bitsInBuf_ = 0;
    buf_ = 0;
    bytesRead_ = 0;
  }
  inline void read( unsigned int * data, unsigned int numBits );
  inline unsigned int bytesRead() { return bytesRead_; }
private:
  unsigned int buf_;
  unsigned int bitsInBuf_;
  unsigned int bytesRead_;
  unsigned char const * source_;
};

inline void
BitReader::read( unsigned int * data, unsigned int numBits )
{

  // Fill up the buffer with enough bits.  I load 8 at a time, only when
  // needed, so I can never have more than 7 left over.
  assert( bitsInBuf_ < 8 );
  while( bitsInBuf_ < numBits ) {
    buf_ |= ( source_[ bytesRead_ ] << bitsInBuf_ );
    bitsInBuf_ += 8;
    bytesRead_++;
  }

  // Read the data.
  * data = buf_ & ( ( 1 << numBits ) - 1 );
  buf_ >>= numBits;
  bitsInBuf_ -= numBits;
}

inline float
FixedPointToFloatZeroToOne( unsigned int fixedPoint, unsigned int numBits )
{
  unsigned int maxVal = ( 1 << numBits ) - 1;
  return ( float ) fixedPoint / maxVal;
}



// Return the number of bytes read.
inline unsigned int
ReadQuatAndExtra( unsigned char const * data, float * valsResult, unsigned int * extraResult,
                 unsigned int bitsPerComponent, unsigned int bitsPerExtra )
{
  BitReader br( data );
  unsigned int bigi;
  br.read( & bigi, 2 ); 
  unsigned int i;
  float sum = 0.0f;
  for( i = 0; i < 4; i++ ) {
    if( i != bigi ) {
      unsigned int sign;
      br.read( & sign, 1 );
      unsigned int val;
      br.read( & val, bitsPerComponent );
      float fval = FixedPointToFloatZeroToOne( val, bitsPerComponent );
      valsResult[ i ] = sign ? - fval : fval;
      sum += valsResult[ i ] * valsResult[ i ];
    }
  }
  if( sum > 1.0f ) sum = 1.0f; // Safety for sqrt.
  valsResult[ bigi ] = sqrtf( 1.0f - sum );
  br.read( extraResult, bitsPerExtra );
  return br.bytesRead();
}

void SetTranslationInvalid( float * xResult, float * yResult, float * zResult );
void SetTranslationInvalid( CalVector * result );
bool TranslationInvalid( CalVector const & result );
