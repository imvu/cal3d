//****************************************************************************//
// loader.cpp                                                                 //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fstream>
#include "cal3d/loader.h"
#include "cal3d/error.h"
#include "cal3d/vector.h"
#include "cal3d/quaternion.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h"
#include "cal3d/coreanimation.h"
#include "cal3d/coreanimatedmorph.h"
#include "cal3d/coretrack.h"
#include "cal3d/corekeyframe.h"
#include "cal3d/coremesh.h"
#include "cal3d/coremorphtrack.h"
#include "cal3d/coremorphkeyframe.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/coresubmorphtarget.h"
#include "cal3d/corematerial.h"
#include "cal3d/tinyxml.h"
#include "cal3d/streamsource.h"
#include "cal3d/buffersource.h"
#include "cal3d/xmlformat.h"
#include "cal3d/calxmlbindings.h"

double CalLoader::translationTolerance = 0.25;
double CalLoader::rotationToleranceDegrees = 0.1;

// Quat format: 
//
//  axis selection (2), 
//  asign (1), afixed (n), 
//  bsign (1), bfixed (n), 
//  csign (1), cfixed (n), 
//  time (in 1/30th second steps)
unsigned int const CalLoader::keyframeBitsPerOriComponent = 11;
unsigned int const CalLoader::keyframeBitsPerTime = 10;
//unsigned int const keyframeTimeMax = ( 1 << CalLoader::keyframeBitsPerTime );

// My resolution is in 1/4 units which is 1/2mm if 1 unit is 2mm.
// My large range is 2^23 units, or 2^24mm, which is 2^4km, or 16km.
// My small range is 2^7 units, or 2^8mm, which is 2.56m.
// The large pos has X, Y, Z, having 25 bits plus a sign bit, totaling 78 bits, leaving two bits padding.
// The small pos has X, Y, Z, having 9 bits plus a sign bit, totaling 30 bits, leaving two bits padding.
unsigned int const CalLoader::keyframeBitsPerUnsignedPosComponent = 25;
unsigned int const CalLoader::keyframeBitsPerPosPadding = 2;
unsigned int const CalLoader::keyframePosBytes = 10;
float const CalLoader::keyframePosRange = ( 1 << ( CalLoader::keyframeBitsPerUnsignedPosComponent - 2 ) ); // In units.

unsigned int const CalLoader::keyframeBitsPerUnsignedPosComponentSmall = 9;
unsigned int const CalLoader::keyframeBitsPerPosPaddingSmall = 2;
unsigned int const CalLoader::keyframePosBytesSmall = 4;
float const CalLoader::keyframePosRangeSmall = ( 1 << ( CalLoader::keyframeBitsPerUnsignedPosComponentSmall - 2 ) );



template<typename T>
void allocateVectorWhereSizeIsGuarded(size_t n, std::vector<T> &o_ret, int lineNumMacroVal){
    if( n > o_ret.max_size())
    {
        CalError calerr;
        calerr.setLastError(CalError::FILE_PARSER_FAILED, __FILE__, lineNumMacroVal);
        throw calerr;
    }
    o_ret.resize(n);
}

bool CAL3D_API CalVectorFromDataSrc( CalDataSource & dataSrc, CalVector * calVec )
{
  return dataSrc.readFloat(calVec->x) &&
         dataSrc.readFloat(calVec->y) &&
         dataSrc.readFloat(calVec->z);
}


bool
TranslationWritten( CalCoreKeyframe * lastCoreKeyframe, bool translationRequired, bool translationIsDynamic )
{
  return ( translationRequired && ( !lastCoreKeyframe || translationIsDynamic ) );
}


CalCoreMesh *CalLoader::loadCoreMesh(const std::string& strFilename)
{

  if(strFilename.size()>= 3 && _stricmp(strFilename.substr(strFilename.size()-3,3).c_str(),Cal::MESH_XMLFILE_EXTENSION)==0)
    return loadXmlCoreMesh(strFilename);

  // open the file
  std::ifstream file;
  file.open(strFilename.c_str(), std::ios::in | std::ios::binary);

  // make sure it opened properly
  if(!file)
  {
    CalError::setLastError(CalError::FILE_NOT_FOUND, __FILE__, __LINE__, strFilename);
    return 0;
  }

  //make a new stream data source and use it to load the mesh
  CalStreamSource streamSrc( file );
  
  CalCoreMesh* coremesh = loadCoreMesh( streamSrc );

  //close the file
  file.close();

  return coremesh;

}

CalCoreSkeleton *CalLoader::loadCoreSkeleton(const std::string& strFilename)
{

  if(strFilename.size()>= 3 && _stricmp(strFilename.substr(strFilename.size()-3,3).c_str(),Cal::SKELETON_XMLFILE_EXTENSION)==0)
    return loadXmlCoreSkeletonFromFile(strFilename);

  // open the file
  std::ifstream file;
  file.open(strFilename.c_str(), std::ios::in | std::ios::binary);

  //make sure it opened properly
  if(!file)
  {
    CalError::setLastError(CalError::FILE_NOT_FOUND, __FILE__, __LINE__, strFilename);
    return 0;
  }

  //make a new stream data source and use it to load the skeleton
  CalStreamSource streamSrc( file );
  
  CalCoreSkeleton* coreskeleton = loadCoreSkeleton( streamSrc );

  //close the file
  file.close();

  return coreskeleton;

}


CalCoreAnimationPtr CalLoader::loadCoreAnimation(std::istream& inputStream, CalCoreSkeleton *skel)
{
   CalStreamSource streamSrc(inputStream);
   return loadCoreAnimation(streamSrc, skel);
}

CalCoreAnimatedMorphPtr CalLoader::loadCoreAnimatedMorph(std::istream& inputStream)
{
   CalStreamSource streamSrc(inputStream);
   return loadCoreAnimatedMorph(streamSrc);
}


CalCoreMaterial *CalLoader::loadCoreMaterial(std::istream& inputStream)
{
   CalStreamSource streamSrc(inputStream);
   return loadCoreMaterial(streamSrc);
}

CalCoreMesh *CalLoader::loadCoreMesh(std::istream& inputStream)
{
   CalStreamSource streamSrc(inputStream);
   return loadCoreMesh(streamSrc);
}

CalCoreSkeleton *CalLoader::loadCoreSkeleton(std::istream& inputStream)
{
   //Create a new istream data source and pass it on
   CalStreamSource streamSrc(inputStream);
   return loadCoreSkeleton(streamSrc);
}

CalCoreAnimationPtr CalLoader::loadCoreAnimationFromBuffer(const void* inputBuffer, unsigned int len, CalCoreSkeleton *skel)
{
   // Create a new buffer data source and pass it on
   CalBufferSource bufferSrc(inputBuffer, len);
   CalCoreAnimationPtr result = loadCoreAnimation(bufferSrc,skel);
   if( result ) {
     return result;
   } else {
     if (CalError::getLastErrorCode() == CalError::INVALID_FILE_FORMAT) {
       // Assumes inputBuffer is zero-terminated, which may not be the case.
       std::string nullTerm((const char*)inputBuffer, len);
       return loadXmlCoreAnimation(nullTerm.c_str(), skel);
     } else {
       return CalCoreAnimationPtr();
     }
   }
}

CalCoreAnimatedMorphPtr CalLoader::loadCoreAnimatedMorphFromBuffer(const void* inputBuffer, unsigned int len )
{
   //Create a new buffer data source and pass it on
   CalBufferSource bufferSrc(inputBuffer, len);
   CalCoreAnimatedMorphPtr result = loadCoreAnimatedMorph(bufferSrc);
   if( result ) {
     return result;
   } else {
       std::string nullTerm((const char*)inputBuffer, len);
     return loadXmlCoreAnimatedMorph(nullTerm.c_str());
   }

}

CalCoreMaterial *CalLoader::loadCoreMaterialFromBuffer(const void* inputBuffer, unsigned int len)
{
   //Create a new buffer data source and pass it on
   CalBufferSource bufferSrc(inputBuffer, len);
   CalCoreMaterial * result = loadCoreMaterial(bufferSrc);
   if( result ) {
     return result;
   } else {
       std::string nullTerm((const char*)inputBuffer, len);
     return loadXmlCoreMaterial(nullTerm.c_str());
   }
}

CalCoreMesh *CalLoader::loadCoreMeshFromBuffer(const void* inputBuffer, unsigned int len)
{
   //Create a new buffer data source and pass it on
    CalCoreMesh * result = NULL;
    try {
        CalBufferSource bufferSrc(inputBuffer, len);
        result = loadCoreMesh(bufferSrc);
        if( result ) {
            return result;
        } else {
            std::string nullTerm((const char*)inputBuffer, len);
            return loadXmlCoreMesh(nullTerm.c_str());
        }
    } catch (const CalError& ){        
    }
    return result;
}

CalCoreSkeleton *CalLoader::loadCoreSkeletonFromBuffer(const void* inputBuffer, unsigned int len)
{
   //Create a new buffer data source and pass it on
   CalBufferSource bufferSrc(inputBuffer, len);
   CalCoreSkeleton * result = loadCoreSkeleton(bufferSrc);
   if( result ) {
     return result;
   } else {
       std::string nullTerm((const char*)inputBuffer, len);
     return loadXmlCoreSkeleton(nullTerm.c_str());
   }
}

CalCoreAnimationPtr CalLoader::loadCoreAnimation(CalDataSource& dataSrc, CalCoreSkeleton *skel)
{
  const CalCoreAnimationPtr null;

  // check if this is a valid file
  char magic[4];
  if(!dataSrc.readBytes(&magic[0], 4) || (memcmp(&magic[0], Cal::ANIMATION_FILE_MAGIC, 4) != 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return null;
  }

  // check if the version is compatible with the library
  int version;
  if(!dataSrc.readInteger(version) || (version < Cal::EARLIEST_COMPATIBLE_FILE_VERSION) || (version > Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__);
    return null;
  }

  bool useAnimationCompression = CalLoader::usesAnimationCompression(version);
  if (Cal::versionHasCompressionFlag(version)) {
    int compressionFlag = 0;
    if (!dataSrc.readInteger(compressionFlag)) {
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      return null;
    }
    // Only really need the first bit.
    useAnimationCompression = (compressionFlag != 0);
  }


  CalCoreAnimationPtr pCoreAnimation(new CalCoreAnimation);

  float duration;
  if(!dataSrc.readFloat(duration))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return null;
  }

  if(duration <= 0.0f)
  {
    CalError::setLastError(CalError::INVALID_ANIMATION_DURATION, __FILE__, __LINE__);
    return null;
  }

  // set the duration in the core animation instance
  pCoreAnimation->duration = duration;

  // read the number of tracks
  int trackCount;
  if(!dataSrc.readInteger(trackCount) || (trackCount <= 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return null;
  }

  // load all core bones
  for(int trackId = 0; trackId < trackCount; ++trackId)
  {
    // load the core track
    CalCoreTrackPtr pCoreTrack(loadCoreTrack(dataSrc,skel, version, useAnimationCompression));
    if(!pCoreTrack)
    {
      return null;
    }

    // add the core track to the core animation instance
    pCoreAnimation->tracks.push_back(*pCoreTrack);
  }

  return pCoreAnimation;
}



CalCoreAnimatedMorphPtr CalLoader::loadCoreAnimatedMorph(CalDataSource& dataSrc)
{
  const CalCoreAnimatedMorphPtr null;

  char magic[4];
  if(!dataSrc.readBytes(&magic[0], 4) || (memcmp(&magic[0], Cal::ANIMATEDMORPH_FILE_MAGIC, 4) != 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return null;
  }

  // check if the version is compatible with the library
  int version;
  if(!dataSrc.readInteger(version) || (version < Cal::EARLIEST_COMPATIBLE_FILE_VERSION) || (version > Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__);
    return null;
  }

  // allocate a new core animatedMorph instance
  CalCoreAnimatedMorphPtr pCoreAnimatedMorph(new CalCoreAnimatedMorph);

  // get the duration of the core animatedMorph
  float duration;
  if(!dataSrc.readFloat(duration))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return null;
  }

  // check for a valid duration
  if(duration <= 0.0f)
  {
    CalError::setLastError(CalError::INVALID_ANIMATION_DURATION, __FILE__, __LINE__);
    return null;
  }

  // set the duration in the core animatedMorph instance
  pCoreAnimatedMorph->setDuration(duration);

  // read the number of tracks
  int trackCount;
  if(!dataSrc.readInteger(trackCount) || (trackCount <= 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return null;
  }

  // load all core bones
  int trackId;
  for(trackId = 0; trackId < trackCount; ++trackId)
  {
    // load the core track
    CalCoreMorphTrack *pCoreTrack;
    pCoreTrack = loadCoreMorphTrack(dataSrc);
    if(pCoreTrack == 0)
    {
      return null;
    }

    // add the core track to the core animatedMorph instance
    pCoreAnimatedMorph->addCoreTrack(pCoreTrack);
  }

  return pCoreAnimatedMorph;
}


 /*****************************************************************************/
/** Loads a core material instance.
  *
  * This function loads a core material instance from a data source.
  *
  * @param dataSrc The data source to load the core material instance from.
  *
  * @return One of the following values:
  *         \li a pointer to the core material
  *         \li \b 0 if an error happened
  *****************************************************************************/

CalCoreMaterial *CalLoader::loadCoreMaterial(CalDataSource& dataSrc)
{

  // check if this is a valid file
  char magic[4];
  if(!dataSrc.readBytes(&magic[0], 4) || (memcmp(&magic[0], Cal::MATERIAL_FILE_MAGIC, 4) != 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // check if the version is compatible with the library
  int version;
  if(!dataSrc.readInteger(version) || (version < Cal::EARLIEST_COMPATIBLE_FILE_VERSION) || (version > Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__);
    return 0;
  }

  bool hasMaterialTypes = (version >= Cal::FIRST_FILE_VERSION_WITH_MATERIAL_TYPES);
  
  // allocate a new core material instance
  CalCoreMaterial *pCoreMaterial;
  pCoreMaterial = new CalCoreMaterial();
  if(pCoreMaterial == 0)
  {
    CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__);
    return 0;
  }

  // get the ambient color of the core material
  CalCoreMaterial::Color ambientColor;
  if( !dataSrc.readBytes(&ambientColor, sizeof(ambientColor)) ) {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return NULL;
  }

  // get the diffuse color of the core material
  CalCoreMaterial::Color diffuseColor;
  if( !dataSrc.readBytes(&diffuseColor, sizeof(diffuseColor)) ) {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return NULL;
  }

  // get the specular color of the core material
  CalCoreMaterial::Color specularColor;
  if( !dataSrc.readBytes(&specularColor, sizeof(specularColor)) ) {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return NULL;
  }

  // get the shininess factor of the core material
  float shininess;
  dataSrc.readFloat(shininess);

  // check if an error happened
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    delete pCoreMaterial;
    return 0;
  }

  // set the colors and the shininess
  pCoreMaterial->ambientColor = ambientColor;
  pCoreMaterial->diffuseColor = diffuseColor;
  pCoreMaterial->specularColor = specularColor;
  pCoreMaterial->shininess = shininess;

  // read the number of maps
  int mapCount;
  if(!dataSrc.readInteger(mapCount) || (mapCount < 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // load all maps
  for(int mapId = 0; mapId < mapCount; ++mapId)
  {
    CalCoreMaterial::Map map;

    dataSrc.readString(map.filename);

    if( hasMaterialTypes ) {
      dataSrc.readString(map.type);
    } else {
      map.type = "";
    }
    
    if(!dataSrc.ok())
    {
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      delete pCoreMaterial;
      return 0;
    }

    pCoreMaterial->maps.push_back(map);
  }

  return pCoreMaterial;
}

 /*****************************************************************************/
/** Loads a core mesh instance.
  *
  * This function loads a core mesh instance from a data source.
  *
  * @param dataSrc The data source to load the core mesh instance from.
  *
  * @return One of the following values:
  *         \li a pointer to the core mesh
  *         \li \b 0 if an error happened
  *****************************************************************************/

CalCoreMesh *CalLoader::loadCoreMesh(CalDataSource& dataSrc)
{

  // check if this is a valid file
  char magic[4];
  if(!dataSrc.readBytes(&magic[0], 4) || (memcmp(&magic[0], Cal::MESH_FILE_MAGIC, 4) != 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // check if the version is compatible with the library
  int version;
  if(!dataSrc.readInteger(version) || (version < Cal::EARLIEST_COMPATIBLE_FILE_VERSION) || (version > Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__);
    return 0;
  }

  // get the number of submeshes
  int submeshCount;
  if(!dataSrc.readInteger(submeshCount))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // allocate a new core mesh instance
  CalCoreMesh *pCoreMesh;
  pCoreMesh = new CalCoreMesh();
  if(pCoreMesh == 0)
  {
    CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__);
    return 0;
  }

  // load all core submeshes
  int submeshId;
  for(submeshId = 0; submeshId < submeshCount; ++submeshId)
  {
    // load the core submesh
    boost::shared_ptr<CalCoreSubmesh> pCoreSubmesh(loadCoreSubmesh(dataSrc, version));
    if(!pCoreSubmesh)
    {
      delete pCoreMesh;
      return 0;
    }

    // add the core submesh to the core mesh instance
    pCoreMesh->addCoreSubmesh(pCoreSubmesh);
  }

  return pCoreMesh;
}

 /*****************************************************************************/
/** Loads a core skeleton instance.
  *
  * This function loads a core skeleton instance from a data source.
  *
  * @param dataSrc The data source to load the core skeleton instance from.
  *
  * @return One of the following values:
  *         \li a pointer to the core skeleton
  *         \li \b 0 if an error happened
  *****************************************************************************/

CalCoreSkeleton *CalLoader::loadCoreSkeleton(CalDataSource& dataSrc)
{

  // check if this is a valid file
  char magic[4];
  if(!dataSrc.readBytes(&magic[0], 4) || (memcmp(&magic[0], Cal::SKELETON_FILE_MAGIC, 4) != 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // check if the version is compatible with the library
  int version;
  if(!dataSrc.readInteger(version) || (version < Cal::EARLIEST_COMPATIBLE_FILE_VERSION) || (version > Cal::CURRENT_FILE_VERSION))
  {
    CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__);
    return 0;
  }

  bool hasNodeLights = (version >= Cal::FIRST_FILE_VERSION_WITH_NODE_LIGHTS);
  
  // read the number of bones
  int boneCount;
  if(!dataSrc.readInteger(boneCount) || (boneCount <= 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // allocate a new core skeleton instance
  CalCoreSkeleton *pCoreSkeleton = new CalCoreSkeleton();
  if(pCoreSkeleton == 0)
  {
    CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__);
    return 0;
  }

  // load the scene ambient
  if( hasNodeLights ) {
    CalVector sceneColor;
    CalVectorFromDataSrc( dataSrc, &sceneColor );
    pCoreSkeleton->setSceneAmbientColor(sceneColor);
  }
  
  // load all core bones
  for(int boneId = 0; boneId < boneCount; ++boneId)
  {
    // load the core bone
    boost::shared_ptr<CalCoreBone> pCoreBone(loadCoreBones(dataSrc, version));
    if(!pCoreBone)
    {
      delete pCoreSkeleton;
      return 0;
    }

    // add the core bone to the core skeleton instance
    pCoreSkeleton->addCoreBone(pCoreBone);

    // add a core skeleton mapping of the bone's name for quick reference later
    pCoreSkeleton->mapCoreBoneName(boneId, pCoreBone->getName());

  }

  // calculate state of the core skeleton
  pCoreSkeleton->calculateState();

  return pCoreSkeleton;
}



 /*****************************************************************************/
/** Loads a core bone instance.
  *
  * This function loads a core bone instance from a data source.
  *
  * @param dataSrc The data source to load the core bone instance from.
  *
  * @return One of the following values:
  *         \li a pointer to the core bone
  *         \li \b 0 if an error happened
  *****************************************************************************/

CalCoreBone *CalLoader::loadCoreBones(CalDataSource& dataSrc, int version)
{
  bool hasNodeLights = (version >= Cal::FIRST_FILE_VERSION_WITH_NODE_LIGHTS);

  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }

  // read the name of the bone
  std::string strName;
  dataSrc.readString(strName);

  // get the translation of the bone
  float tx, ty, tz;
  dataSrc.readFloat(tx);
  dataSrc.readFloat(ty);
  dataSrc.readFloat(tz);

  // get the rotation of the bone
  float rx, ry, rz, rw;
  dataSrc.readFloat(rx);
  dataSrc.readFloat(ry);
  dataSrc.readFloat(rz);
  dataSrc.readFloat(rw);

  // get the bone space translation of the bone
  float txBoneSpace, tyBoneSpace, tzBoneSpace;
  dataSrc.readFloat(txBoneSpace);
  dataSrc.readFloat(tyBoneSpace);
  dataSrc.readFloat(tzBoneSpace);

  // get the bone space rotation of the bone
  float rxBoneSpace, ryBoneSpace, rzBoneSpace, rwBoneSpace;
  dataSrc.readFloat(rxBoneSpace);
  dataSrc.readFloat(ryBoneSpace);
  dataSrc.readFloat(rzBoneSpace);
  dataSrc.readFloat(rwBoneSpace);

  // get the parent bone id
  int parentId;
  dataSrc.readInteger(parentId);

  // get the lgith type and light color
  int lightType = LIGHT_TYPE_NONE;
  CalVector lightColor;
  if(hasNodeLights){
    dataSrc.readInteger(lightType);
    CalVectorFromDataSrc(dataSrc, &lightColor);
  }

  
  CalQuaternion rot(rx,ry,rz,rw);
  CalQuaternion rotbs(rxBoneSpace, ryBoneSpace, rzBoneSpace, rwBoneSpace);
  CalVector trans(tx,ty,tz);

  // check if an error happened
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }

  // allocate a new core bone instance
  CalCoreBone *pCoreBone = new CalCoreBone(strName);

  // set the parent of the bone
  pCoreBone->setParentId(parentId);

  // set all attributes of the bone
  pCoreBone->setTranslation(trans);
  pCoreBone->setRotation(rot);
  pCoreBone->setTranslationBoneSpace(CalVector(txBoneSpace, tyBoneSpace, tzBoneSpace));
  pCoreBone->setRotationBoneSpace(rotbs);
  if( hasNodeLights ) {
    pCoreBone->setLightType( (CalLightType)lightType );
    pCoreBone->setLightColor( lightColor );
  }

  // read the number of children
  int childCount;
  if(!dataSrc.readInteger(childCount) || (childCount < 0))
  {
    delete pCoreBone;
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // load all children ids
  for(; childCount > 0; childCount--)
  {
    int childId;
    if(!dataSrc.readInteger(childId) || (childId < 0))
    {
      delete pCoreBone;
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      return 0;
    }

    pCoreBone->addChildId(childId);
  }

  return pCoreBone;
}




bool
CalLoader::usesAnimationCompression( int version )
{
  return (version >= Cal::FIRST_FILE_VERSION_WITH_ANIMATION_COMPRESSION);
}


CalCoreKeyframe *CalLoader::loadCoreKeyframe(
  CalDataSource& dataSrc, CalCoreBone * coreboneOrNull, int version, 
  CalCoreKeyframe * prevCoreKeyframe,
  bool translationRequired, bool highRangeRequired, bool translationIsDynamic,
  bool useAnimationCompression)
{
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }

  float time;
  float tx, ty, tz;
  float rx, ry, rz, rw;
  if( useAnimationCompression ) {
    unsigned int bytesRequired = compressedKeyframeRequiredBytes( prevCoreKeyframe, translationRequired, highRangeRequired, translationIsDynamic );
    assert( bytesRequired < 100 );
    unsigned char buf[ 100 ];
    if( !dataSrc.readBytes( buf, bytesRequired ) ) {
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      return NULL;
    }
    CalVector vec;
    CalQuaternion quat;
    unsigned int bytesRead = readCompressedKeyframe(
        buf, coreboneOrNull, 
        & vec, & quat, & time, prevCoreKeyframe,
        translationRequired, highRangeRequired, translationIsDynamic);
    if( bytesRead != bytesRequired ) {
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      return NULL;
    }
    tx = vec.x;
    ty = vec.y;
    tz = vec.z;
    rx = quat.x;
    ry = quat.y;
    rz = quat.z;
    rw = quat.w;
    if(version < Cal::FIRST_FILE_VERSION_WITH_ANIMATION_COMPRESSION6 ) {
      if(version >= Cal::FIRST_FILE_VERSION_WITH_ANIMATION_COMPRESSION4 ) {
        if( version >= Cal::FIRST_FILE_VERSION_WITH_ANIMATION_COMPRESSION5 ) {
          if( TranslationWritten( prevCoreKeyframe, translationRequired, translationIsDynamic ) ) {
            dataSrc.readFloat(tx);
            dataSrc.readFloat(ty);
            dataSrc.readFloat(tz);
          }
        }
        
        // get the rotation of the bone
        dataSrc.readFloat(rx);
        dataSrc.readFloat(ry);
        dataSrc.readFloat(rz);
        dataSrc.readFloat(rw);
      }
    }
  } else {
    dataSrc.readFloat(time);
    
    // get the translation of the bone
    dataSrc.readFloat(tx);
    dataSrc.readFloat(ty);
    dataSrc.readFloat(tz);

    if (coreboneOrNull && TranslationInvalid(CalVector(tx, ty, tz))) {
      CalVector tv = coreboneOrNull->getTranslation();
      tx = tv.x;
      ty = tv.y;
      tz = tv.z;
    }
    
    // get the rotation of the bone
    dataSrc.readFloat(rx);
    dataSrc.readFloat(ry);
    dataSrc.readFloat(rz);
    dataSrc.readFloat(rw);
  }  


  // check if an error happened
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }

  // allocate a new core keyframe instance
  CalCoreKeyframe *pCoreKeyframe;
  pCoreKeyframe = new CalCoreKeyframe();
  if(pCoreKeyframe == 0)
  {
    CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__);
    return 0;
  }

  // set all attributes of the keyframe
  pCoreKeyframe->time = time;
  pCoreKeyframe->translation = CalVector(tx, ty, tz);
  pCoreKeyframe->rotation = CalQuaternion(rx, ry, rz, rw);

  return pCoreKeyframe;
}


// Return the number of bytes required by the compressed binary format of a keyframe with these attributes.
unsigned int
CalLoader::compressedKeyframeRequiredBytes( CalCoreKeyframe * lastCoreKeyframe, bool translationRequired, bool highRangeRequired, bool translationIsDynamic )
{
  unsigned int bytes = 0;
  if( translationRequired ) {
    
    // If I am not the first keyframe in the track, and the translations are not dynamic (meaning
    // they are the same for all the keyframes in the track, though different from the skeleton), then
    // just use the translation from the last keyframe.
    if( lastCoreKeyframe && !translationIsDynamic ) {
    } else {
      if( highRangeRequired ) {
        bytes += keyframePosBytes;
      } else {
        bytes += keyframePosBytesSmall;
      }
    }
  }
  bytes += 6;
  return bytes;
}



static float const InvalidCoord = 1e10;

void
SetTranslationInvalid( float * xResult, float * yResult, float * zResult )
{
  * xResult = InvalidCoord;
  * yResult = InvalidCoord;
  * zResult = InvalidCoord;
}

void
SetTranslationInvalid( CalVector * result )
{
  result->set( InvalidCoord, InvalidCoord, InvalidCoord );
}

bool
TranslationInvalid(float x, float y, float z)
{
  return x == InvalidCoord
      && y == InvalidCoord
      && z == InvalidCoord;
}

bool
TranslationInvalid( CalVector const & result )
{
  return result.x == InvalidCoord
      && result.y == InvalidCoord
      && result.z == InvalidCoord;
}


// Pass in the number of bytes that are valid.
// Returns number of byts read.
unsigned int
CalLoader::readCompressedKeyframe(
  unsigned char * buf, CalCoreBone * coreboneOrNull, 
  CalVector * vecResult, CalQuaternion * quatResult, float * timeResult,
  CalCoreKeyframe * lastCoreKeyframe,
  bool translationRequired, bool highRangeRequired, bool translationIsDynamic)
{
  unsigned char * bufStart = buf;
  
  // Read in the translation or get it from the skeleton or zero it out as a last resort.
  if( translationRequired ) {
    
    // If I am not the first keyframe in the track, and the translations are not dynamic (meaning
    // they are the same for all the keyframes in the track, though different from the skeleton), then
    // just use the translation from the last keyframe.
    if( lastCoreKeyframe && !translationIsDynamic ) {
      * vecResult = lastCoreKeyframe->translation;
    } else {
      unsigned int data;
      float tx, ty, tz;
      if( highRangeRequired ) {
        BitReader br( buf );
        
        // Read x.
        br.read( & data, keyframeBitsPerUnsignedPosComponent );
        tx = FixedPointToFloatZeroToOne( data, keyframeBitsPerUnsignedPosComponent ) * keyframePosRange;
        br.read( & data, 1 );
        if( data ) tx = - tx;
        
        // Read y.
        br.read( & data, keyframeBitsPerUnsignedPosComponent );
        ty = FixedPointToFloatZeroToOne( data, keyframeBitsPerUnsignedPosComponent ) * keyframePosRange;
        br.read( & data, 1 );
        if( data ) ty = - ty;
        
        // Read z.
        br.read( & data, keyframeBitsPerUnsignedPosComponent );
        tz = FixedPointToFloatZeroToOne( data, keyframeBitsPerUnsignedPosComponent ) * keyframePosRange;
        br.read( & data, 1 );
        if( data ) tz = - tz;
        
        // Now even it off to n bytes.
        br.read( & data, keyframeBitsPerPosPadding );
        assert( br.bytesRead() == keyframePosBytes );
        buf += keyframePosBytes;
      } else {
        BitReader br( buf );
        
        // Read x.
        br.read( & data, keyframeBitsPerUnsignedPosComponentSmall );
        tx = FixedPointToFloatZeroToOne( data, keyframeBitsPerUnsignedPosComponentSmall ) * keyframePosRangeSmall;
        br.read( & data, 1 );
        if( data ) tx = - tx;
        
        // Read y.
        br.read( & data, keyframeBitsPerUnsignedPosComponentSmall );
        ty = FixedPointToFloatZeroToOne( data, keyframeBitsPerUnsignedPosComponentSmall ) * keyframePosRangeSmall;
        br.read( & data, 1 );
        if( data ) ty = - ty;
        
        // Read z.
        br.read( & data, keyframeBitsPerUnsignedPosComponentSmall );
        tz = FixedPointToFloatZeroToOne( data, keyframeBitsPerUnsignedPosComponentSmall ) * keyframePosRangeSmall;
        br.read( & data, 1 );
        if( data ) tz = - tz;
        
        // Now even it off to n bytes.
        br.read( & data, keyframeBitsPerPosPaddingSmall );
        assert( br.bytesRead() == keyframePosBytesSmall );
        buf += keyframePosBytesSmall;
      }
      vecResult->set( tx, ty, tz );
    }
  } else {
    SetTranslationInvalid( vecResult );
    if( coreboneOrNull ) {
      *vecResult = coreboneOrNull->getTranslation();
    }
  }
  
  // Read in the quat and time.
  float quat[ 4 ];
  unsigned int steps;
  unsigned int bytesRead = ReadQuatAndExtra( buf, quat, & steps, keyframeBitsPerOriComponent, keyframeBitsPerTime );
  buf += 6;

  (void)bytesRead;
  assert( bytesRead == 6 );

  quatResult->set( quat[ 0 ], quat[ 1 ], quat[ 2 ], quat[ 3 ] );
  * timeResult = steps / 30.0f;
  return buf - bufStart;
}



 /*****************************************************************************/
/** Loads a core morphKeyframe instance.
  *
  * This function loads a core morphKeyframe instance from a data source.
  *
  * @param dataSrc The data source to load the core morphKeyframe instance from.
  *
  * @return One of the following values:
  *         \li a pointer to the core morphKeyframe
  *         \li \b 0 if an error happened
  *****************************************************************************/

CalCoreMorphKeyframe *CalLoader::loadCoreMorphKeyframe(CalDataSource& dataSrc)
{
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }

  // get the time of the morphKeyframe
  float time;
  dataSrc.readFloat(time);

  // get the translation of the bone
  float weight;
  dataSrc.readFloat(weight);

  // check if an error happened
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }

  // allocate a new core morphKeyframe instance
  CalCoreMorphKeyframe *pCoreMorphKeyframe;
  pCoreMorphKeyframe = new CalCoreMorphKeyframe();
  if(pCoreMorphKeyframe == 0)
  {
    CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__);
    return 0;
  }

  // create the core morphKeyframe instance
  if(!pCoreMorphKeyframe->create())
  {
    delete pCoreMorphKeyframe;
    return 0;
  }

  // set all attributes of the morphKeyframe
  pCoreMorphKeyframe->setTime(time);
  pCoreMorphKeyframe->setWeight(weight);

  return pCoreMorphKeyframe;
}


 /*****************************************************************************/
/** Loads a core submesh instance.
  *
  * This function loads a core submesh instance from a data source.
  *
  * @param dataSrc The data source to load the core submesh instance from.
  *
  * @return One of the following values:
  *         \li a pointer to the core submesh
  *         \li \b 0 if an error happened
  *****************************************************************************/

CalCoreSubmesh *CalLoader::loadCoreSubmesh(CalDataSource& dataSrc, int version)
{
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }

  bool hasVertexColors = (version >= Cal::FIRST_FILE_VERSION_WITH_VERTEX_COLORS);
  bool hasMorphTargetsInMorphFiles = (version >= Cal::FIRST_FILE_VERSION_WITH_MORPH_TARGETS_IN_MORPH_FILES);

  // get the material thread id of the submesh
  int coreMaterialThreadId;
  dataSrc.readInteger(coreMaterialThreadId);

  // get the number of vertices, faces, level-of-details and springs
  int vertexCount;
  dataSrc.readInteger(vertexCount);

  int faceCount;
  dataSrc.readInteger(faceCount);

  int lodCount;
  dataSrc.readInteger(lodCount);

  int springCount;
  dataSrc.readInteger(springCount);

  // get the number of texture coordinates per vertex
  int textureCoordinateCount;
  dataSrc.readInteger(textureCoordinateCount);

  int morphCount = 0;
  if( hasMorphTargetsInMorphFiles ) {
    dataSrc.readInteger(morphCount);
  }

  // check if an error happened
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }

  // allocate a new core submesh instance
  CalCoreSubmesh* pCoreSubmesh = new CalCoreSubmesh(vertexCount, textureCoordinateCount, faceCount);

  // set the LOD step count
  pCoreSubmesh->setLodCount(lodCount);

  // set the core material id
  pCoreSubmesh->setCoreMaterialThreadId(coreMaterialThreadId);

  // load all vertices and their influences
  pCoreSubmesh->setHasNonWhiteVertexColors( false );
  int vertexId;
  for(vertexId = 0; vertexId < vertexCount; ++vertexId)
  {
    CalCoreSubmesh::Vertex vertex;
    CalCoreSubmesh::LodData lodData;
    CalColor32 vertexColor;

    // load data of the vertex
    dataSrc.readFloat(vertex.position.x);
    dataSrc.readFloat(vertex.position.y);
    dataSrc.readFloat(vertex.position.z);
    dataSrc.readFloat(vertex.normal.x);
    dataSrc.readFloat(vertex.normal.y);
    dataSrc.readFloat(vertex.normal.z);
    vertexColor = CalMakeColor(CalVector(1.0f, 1.0f, 1.0f));
    if( hasVertexColors ) {
      CalVector vc;
      dataSrc.readFloat(vc.x);
      dataSrc.readFloat(vc.y);
      dataSrc.readFloat(vc.z);
      if( vc.x != 1.0f
        || vc.y != 1.0f
        || vc.z != 1.0f ) {
        pCoreSubmesh->setHasNonWhiteVertexColors( true );
      }
      vertexColor = CalMakeColor(vc);
    }
    dataSrc.readInteger(lodData.collapseId);
    dataSrc.readInteger(lodData.faceCollapseCount);

    // check if an error happened
    if(!dataSrc.ok())
    {
      dataSrc.setError();
      delete pCoreSubmesh;
      return 0;
    }

    // load all texture coordinates of the vertex
    int textureCoordinateId;
    for(textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; ++textureCoordinateId)
    {
      CalCoreSubmesh::TextureCoordinate textureCoordinate;

      // load data of the influence
      dataSrc.readFloat(textureCoordinate.u);
      dataSrc.readFloat(textureCoordinate.v);

      // check if an error happened
      if(!dataSrc.ok())
      {
        dataSrc.setError();
        delete pCoreSubmesh;
        return 0;
      }

      // set texture coordinate in the core submesh instance
      pCoreSubmesh->setTextureCoordinate(vertexId, textureCoordinateId, textureCoordinate);
    }

    // get the number of influences
    int influenceCount;
    if(!dataSrc.readInteger(influenceCount) || (influenceCount < 0))
    {
      dataSrc.setError();
      delete pCoreSubmesh;
      return 0;
    }

    std::vector<CalCoreSubmesh::Influence> influences;
    allocateVectorWhereSizeIsGuarded(influenceCount, influences, __LINE__);

    for(int influenceId = 0; influenceId < influenceCount; ++influenceId)
    {
      dataSrc.readInteger(influences[influenceId].boneId),
      dataSrc.readFloat(influences[influenceId].weight);

      if(!dataSrc.ok())
      {
        dataSrc.setError();
        delete pCoreSubmesh;
        return 0;
      }
    }

    // set vertex in the core submesh instance
    pCoreSubmesh->addVertex(vertex, vertexColor, lodData, influences);

    // load the physical property of the vertex if there are springs in the core submesh
    if(springCount > 0)
    {
      float f;
      // load data of the physical property
      dataSrc.readFloat(f);

      // check if an error happened
      if(!dataSrc.ok())
      {
        dataSrc.setError();
        delete pCoreSubmesh;
        return 0;
      }
    }
  }

  // load all springs
  int springId;
  for(springId = 0; springId < springCount; ++springId)
  {
    int i;
    float f;

    // load data of the spring
    dataSrc.readInteger(i);
    dataSrc.readInteger(i);
    dataSrc.readFloat(f);
    dataSrc.readFloat(f);

    // check if an error happened
    if(!dataSrc.ok())
    {
      dataSrc.setError();
      delete pCoreSubmesh;
      return 0;
    }
  }

  for( int morphId = 0; morphId < morphCount; morphId++ ) {
    std::string morphName;
    dataSrc.readString(morphName);

    boost::shared_ptr<CalCoreSubMorphTarget> morphTarget(new CalCoreSubMorphTarget(morphName));
    morphTarget->reserve(vertexCount);

    int blendVertId;
    dataSrc.readInteger(blendVertId);
    
    for( int blendVertI = 0; blendVertI < vertexCount; blendVertI++ )
    {
      CalCoreSubMorphTarget::BlendVertex Vertex;
      Vertex.textureCoords.clear();
      Vertex.textureCoords.reserve(textureCoordinateCount);

      bool copyOrig;

      if( blendVertI < blendVertId ) {
        copyOrig = true;
      } else {
        copyOrig = false;
      }
      
      if( !copyOrig ) {
        CalVectorFromDataSrc(dataSrc, &Vertex.position);
        CalVectorFromDataSrc(dataSrc, &Vertex.normal);
        int textureCoordinateId;
        for(textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; ++textureCoordinateId)
        {
          CalCoreSubmesh::TextureCoordinate textureCoordinate;
          dataSrc.readFloat(textureCoordinate.u);
          dataSrc.readFloat(textureCoordinate.v);

          Vertex.textureCoords.push_back(textureCoordinate);
        }
        if( ! dataSrc.ok() ) {
          delete pCoreSubmesh;
          dataSrc.setError();
          return false;
        }

        morphTarget->setBlendVertex(blendVertI, Vertex);
        dataSrc.readInteger(blendVertId);
     } 
    }
    pCoreSubmesh->addCoreSubMorphTarget(morphTarget);
  }
    
  // load all faces
  int faceId;
  bool flipModel = false;
  for(faceId = 0; faceId < faceCount; ++faceId)
  {
    CalCoreSubmesh::Face face;

    // load data of the face

	int tmp[4];
	dataSrc.readInteger(tmp[0]);
	dataSrc.readInteger(tmp[1]);
	dataSrc.readInteger(tmp[2]);

	if(sizeof(CalIndex)==2)
	{
		if(tmp[0]>65535 || tmp[1]>65535 || tmp[2]>65535)
		{			
			CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
			delete pCoreSubmesh;
			return 0;
		}
	}
	face.vertexId[0]=tmp[0];
	face.vertexId[1]=tmp[1];
	face.vertexId[2]=tmp[2];
	
    // check if an error happened
    if(!dataSrc.ok())
    {
      dataSrc.setError();
      delete pCoreSubmesh;
      return 0;
    }

    // flip if needed
    if (flipModel) {
      tmp[3] = face.vertexId[1];
      face.vertexId[1]=face.vertexId[2];
	  face.vertexId[2]=tmp[3];
    }

    // set face in the core submesh instance
    pCoreSubmesh->setFace(faceId, face);
  }

  return pCoreSubmesh;
}

CalCoreTrack* CalLoader::loadCoreTrack(
  CalDataSource& dataSrc,
  CalCoreSkeleton *skel,
  int version,
  bool useAnimationCompression
) {
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }

  // Read the bone id.
  int coreBoneId;
  bool translationRequired = true;
  bool highRangeRequired = true;
  bool translationIsDynamic = true;
  int keyframeCount;
  static unsigned char buf[ 4 ];

  // If this file version supports animation compression, then I store the boneId in 15 bits,
  // and use the 16th bit to record if translation is required.
  if( useAnimationCompression ) {
    if( !dataSrc.readBytes( buf, 4 ) ) {
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      return NULL;
    }
    
    // Stored low byte first.  Top 3 bits of coreBoneId are compression flags.
    coreBoneId = buf[ 0 ] + ( unsigned int ) ( buf[ 1 ] & 0x1f ) * 256;
    translationRequired = ( buf[ 1 ] & 0x80 ) ? true : false;
    highRangeRequired = ( buf[ 1 ] & 0x40 ) ? true : false;
    translationIsDynamic = ( buf[ 1 ] & 0x20 ) ? true : false;
    keyframeCount = buf[ 2 ] + ( unsigned int ) buf[ 3 ] * 256;
    //if( keyframeCount > keyframeTimeMax ) {
    //  CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    //  return NULL;
    //}
  } else {
    if(!dataSrc.readInteger(coreBoneId) || (coreBoneId < 0)) {
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      return 0;
    }

    // Read the number of keyframes.
    if(!dataSrc.readInteger(keyframeCount) || (keyframeCount <= 0))
    {
      CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
      return 0;
    }
  }

  CalCoreTrack::KeyframeList keyframes;

  CalCoreBone * cb = NULL;
  if( skel ) {
    cb = skel->getCoreBone( coreBoneId );
  }


  // load all core keyframes
  bool hasLastKeyframe = false;
  CalCoreKeyframe lastCoreKeyframe;
  for(int keyframeId = 0; keyframeId < keyframeCount; ++keyframeId)
  {
    // load the core keyframe
    CalCoreKeyframe *pCoreKeyframe = loadCoreKeyframe(
      dataSrc,
      cb,
      version,
      (hasLastKeyframe ? &lastCoreKeyframe : 0),
      translationRequired,
      highRangeRequired,
      translationIsDynamic,
      useAnimationCompression);
    if(pCoreKeyframe == 0)
    {
      return 0;
    }
    // add the core keyframe to the core track instance
    CalCoreKeyframe kf = *pCoreKeyframe;
    lastCoreKeyframe = kf;
    hasLastKeyframe = true;
    delete pCoreKeyframe;
    keyframes.push_back(kf);
  }

  CalCoreTrack* pCoreTrack = new CalCoreTrack(coreBoneId, keyframes);
  pCoreTrack->setTranslationRequired( translationRequired );
  pCoreTrack->setTranslationIsDynamic( translationIsDynamic );
  return pCoreTrack;
}


 /*****************************************************************************/
/** Loads a core morphTrack instance.
  *
  * This function loads a core morphTrack instance from a data source.
  *
  * @param dataSrc The data source to load the core morphTrack instance from.
  *
  * @return One of the following values:
  *         \li a pointer to the core morphTrack
  *         \li \b 0 if an error happened
  *****************************************************************************/

CalCoreMorphTrack *CalLoader::loadCoreMorphTrack(CalDataSource& dataSrc)
{
  if(!dataSrc.ok())
  {
    dataSrc.setError();
    return 0;
  }

  // read the morph name
  std::string morphName;
  if(!dataSrc.readString(morphName))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // allocate a new core morphTrack instance
  CalCoreMorphTrack *pCoreMorphTrack;
  pCoreMorphTrack = new CalCoreMorphTrack();
  if(pCoreMorphTrack == 0)
  {
    CalError::setLastError(CalError::MEMORY_ALLOCATION_FAILED, __FILE__, __LINE__);
    return 0;
  }

  // create the core morphTrack instance
  if(!pCoreMorphTrack->create())
  {
    delete pCoreMorphTrack;
    return 0;
  }

  // link the core morphTrack to the appropriate morph name
  pCoreMorphTrack->setMorphName(morphName);

  // read the number of keyframes
  int keyframeCount;
  if(!dataSrc.readInteger(keyframeCount) || (keyframeCount <= 0))
  {
    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    return 0;
  }

  // load all core keyframes
  int keyframeId;
  for(keyframeId = 0; keyframeId < keyframeCount; ++keyframeId)
  {
    // load the core keyframe
    CalCoreMorphKeyframe *pCoreKeyframe;
    pCoreKeyframe = loadCoreMorphKeyframe(dataSrc);
    if(pCoreKeyframe == 0)
    {
      pCoreMorphTrack->destroy();
      delete pCoreMorphTrack;
      return 0;
    }

    // add the core keyframe to the core morphTrack instance
    pCoreMorphTrack->addCoreMorphKeyframe(pCoreKeyframe);
  }

  return pCoreMorphTrack;
}

void 
CalLoader::setAnimationTranslationTolerance( double p )
{ 
  translationTolerance = p; 
}
void 
CalLoader::setAnimationRotationToleranceDegrees( double p ) 
{ 
  rotationToleranceDegrees = p; 
}
