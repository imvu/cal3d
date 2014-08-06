//****************************************************************************//
// saver.cpp                                                                  //
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
#include <sstream>
#include "cal3d/loader.h"
#include "cal3d/saver.h"
#include "cal3d/error.h"
#include "cal3d/vector.h"
#include "cal3d/quaternion.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h"
#include "cal3d/coreanimation.h"
#include "cal3d/coremorphanimation.h"
#include "cal3d/coretrack.h"
#include "cal3d/coremorphtrack.h"
#include "cal3d/corekeyframe.h"
#include "cal3d/coremorphkeyframe.h"
#include "cal3d/coremesh.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/coremorphtarget.h"
#include "cal3d/corematerial.h"
#include "cal3d/tinyxml.h"
#include "cal3d/xmlformat.h"

template<typename T>
std::string save(const boost::shared_ptr<T>& t, bool (saveBinary)(std::ostream&, T*)) {
    std::ostringstream os;
    if (saveBinary(os, t.get())) {
        return os.str();
    } else {
        return "";
    }
}

template<typename T>
std::string save(const boost::shared_ptr<T>& t, bool (saveBinary)(std::ostream&, T*, bool), bool negateW) {
    std::ostringstream os;
    if (saveBinary(os, t.get(), negateW)) {
        return os.str();
    } else {
        return "";
    }
}

std::string CalSaver::saveCoreAnimationToBuffer(CalCoreAnimationPtr pCoreAnimation, bool negateW) {
    return save(pCoreAnimation, &saveCoreAnimation, negateW);
}

std::string CalSaver::saveCoreAnimationXmlToBuffer(CalCoreAnimationPtr pCoreAnimation, bool negateW) {
    std::ostringstream os;
    if (saveXmlCoreAnimation(os, pCoreAnimation.get(), negateW)) {
        return os.str();
    } else {
        return "";
    }
}

std::string CalSaver::saveCoreMorphAnimationToBuffer(CalCoreMorphAnimationPtr pCoreMorphAnimation) {
    return save(pCoreMorphAnimation, &saveCoreMorphAnimation);
}

std::string CalSaver::saveCoreMorphAnimationXmlToBuffer(CalCoreMorphAnimationPtr pCoreMorphAnimation) {
    std::ostringstream os;
    if (saveXmlCoreMorphAnimation(os, pCoreMorphAnimation.get())) {
        return os.str();
    } else {
        return "";
    }
}

std::string CalSaver::saveCoreMaterialToBuffer(CalCoreMaterialPtr pCoreMaterial) {
    return save(pCoreMaterial, &saveCoreMaterial);
}

std::string CalSaver::saveCoreMeshToBuffer(CalCoreMeshPtr pCoreMesh) {
    return save(pCoreMesh, &saveCoreMesh);
}

std::string CalSaver::saveCoreSkeletonToBuffer(CalCoreSkeletonPtr pCoreSkeleton) {
    return save(pCoreSkeleton, &saveCoreSkeleton, false);
}

bool CalSaver::saveCoreAnimation(const std::string& strFilename, CalCoreAnimation* pCoreAnimation, bool negateW) {
    if (
        strFilename.size() >= 3 &&
        cal3d_stricmp(strFilename.substr(strFilename.size() - 3, 3).c_str(), cal3d::ANIMATION_XMLFILE_EXTENSION) == 0
    ) {
        return saveXmlCoreAnimation(strFilename, pCoreAnimation, negateW);
    }

    // open the file
    std::ofstream file;
    file.open(strFilename.c_str(), std::ios::out | std::ios::binary);
    if (!file) {
        CalError::setLastError(CalError::FILE_CREATION_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    return saveCoreAnimation(file, pCoreAnimation, negateW);
}

bool CalSaver::saveCoreAnimation(std::ostream& file, CalCoreAnimation* pCoreAnimation, bool negateW) {
    const char* strFilename = ""; // do we care?

    // write magic tag
    if (!CalPlatform::writeBytes(file, &cal3d::ANIMATION_FILE_MAGIC, sizeof(cal3d::ANIMATION_FILE_MAGIC))) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    // write version info
    int version = cal3d::CURRENT_FILE_VERSION;
    if (!CalPlatform::writeInteger(file, version)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    if (cal3d::versionHasCompressionFlag(cal3d::CURRENT_FILE_VERSION)) {
        int useCompressionFlag = 0; // no compression
        if (!CalPlatform::writeInteger(file, useCompressionFlag)) {
            CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
            return false;
        }
    }

    // write the duration of the core animation
    if (!CalPlatform::writeFloat(file, pCoreAnimation->duration)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    // get core track list
    CalCoreAnimation::TrackList& listCoreTrack = pCoreAnimation->tracks;

    // write the number of tracks
    if (!CalPlatform::writeInteger(file, listCoreTrack.size())) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return 0;
    }

    // write all core bones
    CalCoreAnimation::TrackList::iterator iteratorCoreTrack;
    for (iteratorCoreTrack = listCoreTrack.begin(); iteratorCoreTrack != listCoreTrack.end(); ++iteratorCoreTrack) {
        // save core track
        if (!saveCoreTrack(file, strFilename, &*iteratorCoreTrack, negateW)) {
            return false;
        }
    }

    return true;
}


bool CalSaver::saveCoreMorphAnimation(const std::string& strFilename, CalCoreMorphAnimation* pCoreMorphAnimation) {
    if (strFilename.size() >= 3 && cal3d_stricmp(strFilename.substr(strFilename.size() - 3, 3).c_str(),
            cal3d::ANIMATEDMORPH_XMLFILE_EXTENSION) == 0) {
        return saveXmlCoreMorphAnimation(strFilename, pCoreMorphAnimation);
    }


    // open the file
    std::ofstream file;
    file.open(strFilename.c_str(), std::ios::out | std::ios::binary);
    if (!file) {
        CalError::setLastError(CalError::FILE_CREATION_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    return saveCoreMorphAnimation(file, pCoreMorphAnimation);
}

bool CalSaver::saveCoreMorphAnimation(std::ostream& file, CalCoreMorphAnimation* pCoreMorphAnimation) {
    const char* strFilename = "";

    // write magic tag
    if (!CalPlatform::writeBytes(file, &cal3d::ANIMATEDMORPH_FILE_MAGIC, sizeof(cal3d::ANIMATEDMORPH_FILE_MAGIC))) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    // write version info
    if (!CalPlatform::writeInteger(file, cal3d::CURRENT_FILE_VERSION)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    // write the duration of the core animatedMorph
    if (!CalPlatform::writeFloat(file, pCoreMorphAnimation->duration)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    // get core track list
    std::vector<CalCoreMorphTrack>& listCoreMorphTrack = pCoreMorphAnimation->tracks;

    // write the number of tracks
    if (!CalPlatform::writeInteger(file, listCoreMorphTrack.size())) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return 0;
    }

    std::vector<CalCoreMorphTrack>::iterator iteratorCoreMorphTrack;
    for (iteratorCoreMorphTrack = listCoreMorphTrack.begin(); iteratorCoreMorphTrack != listCoreMorphTrack.end(); ++iteratorCoreMorphTrack) {
        // save coreMorph track
        if (!saveCoreMorphTrack(file, strFilename, &(*iteratorCoreMorphTrack))) {
            return false;
        }
    }

    return true;
}

bool CalSaver::saveCoreBone(std::ostream& file, const CalCoreSkeleton* coreSkeleton, const CalCoreBone* pCoreBone, bool negateW) {
    if (!file) {
        CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, "");
        return false;
    }

    // write the name of the bone
    if (!CalPlatform::writeString(file, pCoreBone->name)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    CalPlatform::writeVector(file, pCoreBone->relativeTransform.translation);
    CalQuaternion rq = pCoreBone->relativeTransform.rotation; 
    if (negateW) {
        rq.w = -rq.w;
    }
    CalPlatform::writeQuat(file, rq);
    CalPlatform::writeVector(file, pCoreBone->inverseBindPoseTransform.translation);
    CalQuaternion iq = pCoreBone->relativeTransform.rotation; 
    if (negateW) {
        iq.w = -iq.w;
    }
    CalPlatform::writeQuat(file, iq);

    // write the parent bone id
    if (!CalPlatform::writeInteger(file, pCoreBone->parentId)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // write lighting data
    CalPlatform::writeInteger(file, pCoreBone->lightType);
    CalPlatform::writeVector(file, pCoreBone->lightColor);

    std::vector<int> children(coreSkeleton->getChildIds(pCoreBone));

    if (!CalPlatform::writeInteger(file, children.size())) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    std::vector<int>::const_iterator iteratorChildId;
    for (iteratorChildId = children.begin(); iteratorChildId != children.end(); ++iteratorChildId) {
        if (!CalPlatform::writeInteger(file, *iteratorChildId)) {
            CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
            return false;
        }
    }

    return true;
}

/*****************************************************************************/
/** Saves a core keyframe instance.
  *
  * This function saves a core keyframe instance to a file stream.
  *
  * @param file The file stream to save the core keyframe instance to.
  * @param strFilename The name of the file stream.
  * @param pCoreKeyframe A pointer to the core keyframe instance that should be
  *                      saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreKeyframe(
    std::ostream& file,
    const std::string& strFilename,
    const CalCoreKeyframe* pCoreKeyframe,
    bool negateW
) {
    if (!file) {
        CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, strFilename);
        return false;
    }

    CalPlatform::writeFloat(file, pCoreKeyframe->time);
    CalPlatform::writeVector(file, pCoreKeyframe->transform.translation);
    CalQuaternion rt = pCoreKeyframe->transform.rotation;
    if (negateW) {
        rt.w = -rt.w;
    }
    CalPlatform::writeQuat(file, rt);

    // check if an error happend
    if (!file) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    return true;
}


/*****************************************************************************/
/** Saves a core morphKeyframe instance.
  *
  * This function saves a core morphKeyframe instance to a file stream.
  *
  * @param file The file stream to save the core morphKeyframe instance to.
  * @param strFilename The name of the file stream.
  * @param pCoreMorphKeyframe A pointer to the core morphKeyframe instance that should be
  *                      saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreMorphKeyframe(std::ostream& file, const std::string& strFilename, CalCoreMorphKeyframe* pCoreMorphKeyframe) {
    if (!file) {
        CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, strFilename);
        return false;
    }

    CalPlatform::writeFloat(file, pCoreMorphKeyframe->time);
    CalPlatform::writeFloat(file, pCoreMorphKeyframe->weight);

    // check if an error happend
    if (!file) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    return true;
}

/*****************************************************************************/
/** Saves a core material instance.
  *
  * This function saves a core material instance to a file.
  *
  * @param strFilename The name of the file to save the core material instance
  *                    to.
  * @param pCoreMaterial A pointer to the core material instance that should
  *                      be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreMaterial(const std::string& strFilename, CalCoreMaterial* pCoreMaterial) {
    if (strFilename.size() >= 3 && cal3d_stricmp(strFilename.substr(strFilename.size() - 3, 3).c_str(), cal3d::MATERIAL_XMLFILE_EXTENSION) == 0) {
        return saveXmlCoreMaterial(strFilename, pCoreMaterial);
    }

    // open the file
    std::ofstream file;
    file.open(strFilename.c_str(), std::ios::out | std::ios::binary);
    if (!file) {
        CalError::setLastError(CalError::FILE_CREATION_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    bool result = saveCoreMaterial(file, pCoreMaterial);
    file.close();
    return result;
}

bool CalSaver::saveCoreMaterial(std::ostream& file, CalCoreMaterial* pCoreMaterial) {
    // write magic tag
    if (!CalPlatform::writeBytes(file, &cal3d::MATERIAL_FILE_MAGIC, sizeof(cal3d::MATERIAL_FILE_MAGIC))) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // write version info
    if (!CalPlatform::writeInteger(file, cal3d::CURRENT_FILE_VERSION)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    unsigned char ambientColor[4] = {0, 0, 0, 0};
    CalPlatform::writeBytes(file, &ambientColor, sizeof(ambientColor));

    unsigned char diffusetColor[4] = {0, 0, 0, 0};
    CalPlatform::writeBytes(file, &diffusetColor, sizeof(diffusetColor));

    unsigned char specularColor[4] = {0, 0, 0, 0};
    CalPlatform::writeBytes(file, &specularColor, sizeof(specularColor));

    CalPlatform::writeFloat(file, 0.0f);

    // check if an error happend
    if (!file) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // get the map vector
    std::vector<CalCoreMaterial::Map>& vectorMap = pCoreMaterial->maps;

    // write the number of maps
    if (!CalPlatform::writeInteger(file, vectorMap.size())) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // write all maps
    int mapId;
    for (mapId = 0; mapId < (int)vectorMap.size(); ++mapId) {
        CalCoreMaterial::Map& map = vectorMap[mapId];

        // write the filename of the map
        bool ret = CalPlatform::writeString(file, map.filename);
        if (ret) {
            ret = CalPlatform::writeString(file, map.type);
        }
        if (!ret) {
            CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
            return false;
        }
    }

    return true;
}

/*****************************************************************************/
/** Saves a core mesh instance.
  *
  * This function saves a core mesh instance to a file.
  *
  * @param strFilename The name of the file to save the core mesh instance to.
  * @param pCoreMesh A pointer to the core mesh instance that should be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreMesh(const std::string& strFilename, CalCoreMesh* pCoreMesh) {
    if (strFilename.size() >= 3 && cal3d_stricmp(strFilename.substr(strFilename.size() - 3, 3).c_str(), cal3d::MESH_XMLFILE_EXTENSION) == 0) {
        return saveXmlCoreMesh(strFilename, pCoreMesh);
    }

    // open the file
    std::ofstream file;
    file.open(strFilename.c_str(), std::ios::out | std::ios::binary);
    if (!file) {
        CalError::setLastError(CalError::FILE_CREATION_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }
    return saveCoreMesh(file, pCoreMesh);
}


bool CalSaver::saveCoreMesh(std::ostream& os, CalCoreMesh* pCoreMesh) {

    // write magic tag
    if (!CalPlatform::writeBytes(os, &cal3d::MESH_FILE_MAGIC, sizeof(cal3d::MESH_FILE_MAGIC))) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // write version info
    if (!CalPlatform::writeInteger(os, cal3d::CURRENT_FILE_VERSION)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // get the submesh vector
    CalCoreMesh::CalCoreSubmeshVector& vectorCoreSubmesh = pCoreMesh->submeshes;

    // write the number of submeshes
    if (!CalPlatform::writeInteger(os, vectorCoreSubmesh.size())) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // write all core submeshes
    int submeshId;
    for (submeshId = 0; submeshId < (int)vectorCoreSubmesh.size(); ++submeshId) {
        // write the core submesh
        if (!saveCoreSubmesh(os, vectorCoreSubmesh[submeshId].get())) {
            return false;
        }
    }
    return true;
}
/*****************************************************************************/
/** Saves a core skeleton instance.
  *
  * This function saves a core skeleton instance to a file.
  *
  * @param strFilename The name of the file to save the core skeleton instance
  *                    to.
  * @param pCoreSkeleton A pointer to the core skeleton instance that should be
  *                      saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreSkeleton(const std::string& strFilename, CalCoreSkeleton* pCoreSkeleton, bool negateW) {
    if (strFilename.size() >= 3 && cal3d_stricmp(strFilename.substr(strFilename.size() - 3, 3).c_str(), cal3d::SKELETON_XMLFILE_EXTENSION) == 0) {
        return saveXmlCoreSkeleton(strFilename, pCoreSkeleton, negateW);
    }

    // open the file
    std::ofstream file;
    file.open(strFilename.c_str(), std::ios::out | std::ios::binary);
    if (!file) {
        CalError::setLastError(CalError::FILE_CREATION_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    return saveCoreSkeleton(file, pCoreSkeleton, negateW);
}

bool CalSaver::saveCoreSkeleton(std::ostream& file, CalCoreSkeleton* pCoreSkeleton, bool negateW) {
    // write magic tag
    if (!CalPlatform::writeBytes(file, &cal3d::SKELETON_FILE_MAGIC, sizeof(cal3d::SKELETON_FILE_MAGIC))) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // write version info
    if (!CalPlatform::writeInteger(file, cal3d::CURRENT_FILE_VERSION)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // write the number of bones
    if (!CalPlatform::writeInteger(file, pCoreSkeleton->coreBones.size())) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // write the sceneambient
    CalVector sceneColor = pCoreSkeleton->sceneAmbientColor;
    CalPlatform::writeFloat(file, sceneColor.x);
    CalPlatform::writeFloat(file, sceneColor.y);
    CalPlatform::writeFloat(file, sceneColor.z);


    for (size_t boneId = 0; boneId < pCoreSkeleton->coreBones.size(); ++boneId) {
        if (!saveCoreBone(file, pCoreSkeleton, pCoreSkeleton->coreBones[boneId].get(), negateW)) {
            return false;
        }
    }

    return true;
}

/*****************************************************************************/
/** Saves a core submesh instance.
  *
  * This function saves a core submesh instance to a file stream.
  *
  * @param file The file stream to save the core submesh instance to.
  * @param optionalFilename The name of the file stream.
  * @param pCoreSubmesh A pointer to the core submesh instance that should be
  *                     saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreSubmesh(std::ostream& os, CalCoreSubmesh* pCoreSubmesh) {
    if (!os) {
        CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, "");
        return false;
    }

    // write the core material thread id
    if (!CalPlatform::writeInteger(os, pCoreSubmesh->coreMaterialThreadId)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // get the vertex, face, physical property and spring vector
    const cal3d::SSEArray<CalCoreSubmesh::Vertex>& vectorVertex = pCoreSubmesh->getVectorVertex();
    const std::vector<CalColor32>& vertexColors = pCoreSubmesh->getVertexColors();
    const std::vector<CalCoreSubmesh::Face>& vectorFace = pCoreSubmesh->getFaces();

    // write the number of vertices, faces, level-of-details and springs
    CalPlatform::writeInteger(os, vectorVertex.size());
    CalPlatform::writeInteger(os, vectorFace.size());
    CalPlatform::writeInteger(os, 0); // lod count
    CalPlatform::writeInteger(os, 0); // spring count

    // get the texture coordinate vector vector
    std::vector<CalCoreSubmesh::VectorTextureCoordinate> vectorvectorTextureCoordinate;
    if (pCoreSubmesh->hasTextureCoordinates()) {
        vectorvectorTextureCoordinate.push_back(pCoreSubmesh->getTextureCoordinates());
    }

    // write the number of texture coordinates per vertex
    CalPlatform::writeInteger(os, vectorvectorTextureCoordinate.size());

    // write the number of morph targets
    const CalCoreSubmesh::MorphTargetArray& vectorMorphs = pCoreSubmesh->getMorphTargets();

    CalPlatform::writeInteger(os, vectorMorphs.size());

    // check if an error happend
    if (!os) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
        return false;
    }

    // write all vertices
    const CalCoreSubmesh::Influence* currentInfluence = cal3d::pointerFromVector(pCoreSubmesh->getInfluences());
    for (int vertexId = 0; vertexId < (int)vectorVertex.size(); ++vertexId) {
        const CalCoreSubmesh::Vertex& vertex = vectorVertex[vertexId];

        // write the vertex data
        CalPlatform::writeFloat(os, vertex.position.x);
        CalPlatform::writeFloat(os, vertex.position.y);
        CalPlatform::writeFloat(os, vertex.position.z);
        CalPlatform::writeFloat(os, vertex.normal.x);
        CalPlatform::writeFloat(os, vertex.normal.y);
        CalPlatform::writeFloat(os, vertex.normal.z);
        CalVector vc(CalVectorFromColor(vertexColors[vertexId]));
        CalPlatform::writeFloat(os, vc.x);
        CalPlatform::writeFloat(os, vc.y);
        CalPlatform::writeFloat(os, vc.z);
        CalPlatform::writeInteger(os, -1); // collapseId
        CalPlatform::writeInteger(os, 0); // faceCollapseCount

        // write all texture coordinates of this vertex
        int textureCoordinateId;
        for (textureCoordinateId = 0; textureCoordinateId < (int)vectorvectorTextureCoordinate.size(); ++textureCoordinateId) {
            const CalCoreSubmesh::TextureCoordinate& textureCoordinate = vectorvectorTextureCoordinate[textureCoordinateId][vertexId];

            // write the influence data
            CalPlatform::writeFloat(os, textureCoordinate.u);
            CalPlatform::writeFloat(os, textureCoordinate.v);

            // check if an error happend
            if (!os) {
                CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
                return false;
            }
        }

        const CalCoreSubmesh::Influence* nextVertex = currentInfluence;
        while (!nextVertex->lastInfluenceForThisVertex) {
            ++nextVertex;
        }
        ++nextVertex;

        if (!CalPlatform::writeInteger(os, size_t(nextVertex - currentInfluence))) {
            CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
            return false;
        }

        // write all influences of this vertex
        for (; currentInfluence != nextVertex; ++currentInfluence) {
            CalPlatform::writeInteger(os, currentInfluence->boneId);
            CalPlatform::writeFloat(os, currentInfluence->weight);

            // check if an error happend
            if (!os) {
                CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
                return false;
            }
        }
    }

    for (size_t morphId = 0; morphId < vectorMorphs.size(); morphId++) {
        CalCoreMorphTargetPtr morphTarget = vectorMorphs[morphId];
        CalPlatform::writeString(os, morphTarget->name);

        const CalCoreMorphTarget::VertexOffsetArray& vertices = morphTarget->vertexOffsets;
        for (size_t i = 0; i < vertices.size(); ++i) {
            VertexOffset const& bv = vertices[i];

            const size_t blendId = bv.vertexId;
            const CalCoreSubmesh::Vertex& Vertex = vectorVertex[blendId];
            const float differenceTolerance = 0.01f;
            if (bv.position.asCalVector().length() < differenceTolerance) {
                continue;
            }

            CalPlatform::writeInteger(os, blendId);
            CalPlatform::writeVector(os, Vertex.position + bv.position);
            CalPlatform::writeVector(os, Vertex.normal + bv.normal);

            for (size_t tcI = 0; tcI < vectorvectorTextureCoordinate.size(); tcI++) {
                CalPlatform::writeFloat(os, 0.f);
                CalPlatform::writeFloat(os, 0.f);
            }
        }
        CalPlatform::writeInteger(os, vectorVertex.size() + 1);
    }

    for (int faceId = 0; faceId < (int)vectorFace.size(); ++faceId) {
        const CalCoreSubmesh::Face& face = vectorFace[faceId];

        CalPlatform::writeInteger(os, face.vertexId[0]);
        CalPlatform::writeInteger(os, face.vertexId[1]);
        CalPlatform::writeInteger(os, face.vertexId[2]);

        if (!os) {
            CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, "");
            return false;
        }
    }

    return true;
}

/*****************************************************************************/
/** Saves a core track instance.
  *
  * This function saves a core track instance to a file stream.
  *
  * @param file The file stream to save the core track instance to.
  * @param strFilename The name of the file stream.
  * @param pCoreTrack A pointer to the core track instance that should be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveCoreTrack(std::ostream& file, const std::string& strFilename, CalCoreTrack* pCoreTrack, bool negateW) {
    if (!file) {
        CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, strFilename);
        return false;
    }

    // Write the coreBoneId.
    if (!CalPlatform::writeInteger(file, pCoreTrack->coreBoneId)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    // Write the number of keyframes
    if (!CalPlatform::writeInteger(file, pCoreTrack->keyframes.size())) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    for (size_t i = 0; i < pCoreTrack->keyframes.size(); ++i) {
        if (!saveCoreKeyframe(file, strFilename, &pCoreTrack->keyframes[i], negateW)) {
            return false;
        }
    }

    return true;
}

bool CalSaver::saveCoreMorphTrack(std::ostream& file, const std::string& strFilename, CalCoreMorphTrack* pCoreMorphTrack) {
    if (!file) {
        CalError::setLastError(CalError::INVALID_HANDLE, __FILE__, __LINE__, strFilename);
        return false;
    }

    // write the morph name
    if (!CalPlatform::writeString(file, pCoreMorphTrack->morphName)) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    // read the number of keyframes
    if (!CalPlatform::writeInteger(file, pCoreMorphTrack->keyframes.size())) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    // save all core keyframes
    for (size_t i = 0; i < pCoreMorphTrack->keyframes.size(); ++i) {
        // save the core keyframe
        if (!saveCoreMorphKeyframe(file, strFilename, &pCoreMorphTrack->keyframes[i])) {
            return false;
        }
    }

    return true;
}

/*****************************************************************************/
/** Saves a core skeleton instance to a XML file
  *
  * This function saves a core skeleton instance to a XML file.
  *
  * @param strFilename The name of the file to save the core skeleton instance
  *                    to.
  * @param pCoreSkeleton A pointer to the core skeleton instance that should be
  *                      saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveXmlCoreSkeleton(const std::string& strFilename, CalCoreSkeleton* pCoreSkeleton, bool negateW) {
    std::stringstream str;

    TiXmlDocument doc(strFilename);

    TiXmlElement header("HEADER");
    header.SetAttribute("MAGIC", cal3d::SKELETON_XMLFILE_EXTENSION);
    header.SetAttribute("VERSION", cal3d::LIBRARY_VERSION);

    doc.InsertEndChild(header);

    TiXmlElement skeleton("SKELETON");
    skeleton.SetAttribute("NUMBONES", pCoreSkeleton->coreBones.size());

    CalVector sceneColor = pCoreSkeleton->sceneAmbientColor;
    str << sceneColor.x << " " << sceneColor.y << " " << sceneColor.z;
    skeleton.SetAttribute("SCENEAMBIENTCOLOR", str.str());

    for (int boneId = 0; boneId < (int)pCoreSkeleton->coreBones.size(); ++boneId) {
        CalCoreBone* pCoreBone = pCoreSkeleton->coreBones[boneId].get();

        TiXmlElement bone("BONE");
        bone.SetAttribute("ID", boneId);
        bone.SetAttribute("NAME", pCoreBone->name);
        if (pCoreBone->hasLightingData()) {
            bone.SetAttribute("LIGHTTYPE", pCoreBone->lightType);
            str.str("");
            CalVector c = pCoreBone->lightColor;
            str << c.x << " " << c.y << " " << c.z;
            bone.SetAttribute("LIGHTCOLOR", str.str());
        }

        TiXmlElement translation("TRANSLATION");
        const CalVector& translationVector = pCoreBone->relativeTransform.translation;


        str.str("");
        str << translationVector.x << " "
            << translationVector.y << " "
            << translationVector.z;

        TiXmlText translationdata(str.str());

        translation.InsertEndChild(translationdata);
        bone.InsertEndChild(translation);


        TiXmlElement rotation("ROTATION");
        const CalQuaternion& rotationQuad = pCoreBone->relativeTransform.rotation;


        float w;
        if (negateW) {
            w = -rotationQuad.w;
        } else {
            w = rotationQuad.w;
        }
        str.str("");
        str << rotationQuad.x << " "
            << rotationQuad.y << " "
            << rotationQuad.z << " "
            << w;

        TiXmlText rotationdata(str.str());
        rotation.InsertEndChild(rotationdata);
        bone.InsertEndChild(rotation);


        TiXmlElement localtranslation("LOCALTRANSLATION");
        const CalVector& localtranslationVector = pCoreBone->inverseBindPoseTransform.translation;

        str.str("");
        str << localtranslationVector.x << " "
            << localtranslationVector.y << " "
            << localtranslationVector.z;

        TiXmlText localtranslationdata(str.str());

        localtranslation.InsertEndChild(localtranslationdata);
        bone.InsertEndChild(localtranslation);


        TiXmlElement localrotation("LOCALROTATION");
        const CalQuaternion& localrotationQuad = pCoreBone->inverseBindPoseTransform.rotation;

        float lw;
        if (negateW) {
            lw = -localrotationQuad.w;
        } else {
            lw = localrotationQuad.w;
        }
        str.str("");
        str << localrotationQuad.x << " "
            << localrotationQuad.y << " "
            << localrotationQuad.z << " "
            << lw;

        TiXmlText localrotationdata(str.str());
        localrotation.InsertEndChild(localrotationdata);
        bone.InsertEndChild(localrotation);


        TiXmlElement parent("PARENTID");
        str.str("");
        str << pCoreBone->parentId;
        TiXmlText parentid(str.str());
        parent.InsertEndChild(parentid);
        bone.InsertEndChild(parent);

        std::vector<int> listChildId(pCoreSkeleton->getChildIds(pCoreBone));

        std::vector<int>::const_iterator iteratorChildId;
        for (iteratorChildId = listChildId.begin(); iteratorChildId != listChildId.end(); ++iteratorChildId) {
            TiXmlElement child("CHILDID");
            str.str("");
            //int id=*iteratorChildId;
            str << *iteratorChildId;
            TiXmlText childid(str.str());
            child.InsertEndChild(childid);
            bone.InsertEndChild(child);
        }
        skeleton.InsertEndChild(bone);

    }
    doc.InsertEndChild(skeleton);

    if (!doc.SaveFile()) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }
    return true;
}


bool CalSaver::saveXmlCoreAnimation(const std::string& strFilename, CalCoreAnimation* pCoreAnimation, bool negateW) {
    std::ofstream of(strFilename.c_str());
    return saveXmlCoreAnimation(of, pCoreAnimation, negateW);
}


bool CalSaver::saveXmlCoreAnimation(std::ostream& os, CalCoreAnimation* pCoreAnimation, bool negateW) {

    std::stringstream str;

    TiXmlDocument doc;

    TiXmlElement animation("ANIMATION");

    TiXmlElement header("HEADER");
    header.SetAttribute("MAGIC", cal3d::ANIMATION_XMLFILE_EXTENSION);
    header.SetAttribute("VERSION", cal3d::LIBRARY_VERSION);

    doc.InsertEndChild(header);

    str.str("");
    str << pCoreAnimation->duration;
    animation.SetAttribute("DURATION", str.str());

    // get core track list
    CalCoreAnimation::TrackList& listCoreTrack = pCoreAnimation->tracks;

    animation.SetAttribute("NUMTRACKS", listCoreTrack.size());


    // write all core bones
    CalCoreAnimation::TrackList::iterator iteratorCoreTrack;
    for (iteratorCoreTrack = listCoreTrack.begin(); iteratorCoreTrack != listCoreTrack.end(); ++iteratorCoreTrack) {
        const CalCoreTrack* pCoreTrack = &*iteratorCoreTrack;

        TiXmlElement track("TRACK");
        track.SetAttribute("BONEID", pCoreTrack->coreBoneId);

        // Always save out the TRANSLATIONREQUIRED flag in XML, and save the translations iff the flag is true.
        bool translationIsDynamic = pCoreTrack->translationIsDynamic;

        track.SetAttribute("TRANSLATIONREQUIRED", (pCoreTrack->translationRequired ? 1 : 0));
        track.SetAttribute("TRANSLATIONISDYNAMIC", (translationIsDynamic ? 1 : 0));
        track.SetAttribute("HIGHRANGEREQUIRED", 1);
        track.SetAttribute("NUMKEYFRAMES", pCoreTrack->keyframes.size());

        // save all core keyframes
        for (size_t i = 0; i < pCoreTrack->keyframes.size(); ++i) {
            const CalCoreKeyframe& pCoreKeyframe = pCoreTrack->keyframes[i];

            TiXmlElement keyframe("KEYFRAME");

            str.str("");
            str << pCoreKeyframe.time;
            keyframe.SetAttribute("TIME", str.str());

            if (pCoreTrack->translationRequired) {

                // If translation required but not dynamic and i != 0, then I won't write the translation.
                if (translationIsDynamic || i == 0) {
                    TiXmlElement translation("TRANSLATION");
                    const CalVector& translationVector = pCoreKeyframe.transform.translation;

                    str.str("");
                    str << translationVector.x << " "
                        << translationVector.y << " "
                        << translationVector.z;

                    TiXmlText translationdata(str.str());

                    translation.InsertEndChild(translationdata);
                    keyframe.InsertEndChild(translation);
                }
            }

            TiXmlElement rotation("ROTATION");
            const CalQuaternion& rotationQuad = pCoreKeyframe.transform.rotation;

            float w;
            if (negateW) {
                w = -rotationQuad.w;
            } else {
                w = rotationQuad.w;
            }
            str.str("");
            str << rotationQuad.x << " "
                << rotationQuad.y << " "
                << rotationQuad.z << " "
                << w;

            TiXmlText rotationdata(str.str());
            rotation.InsertEndChild(rotationdata);
            keyframe.InsertEndChild(rotation);

            track.InsertEndChild(keyframe);
        }

        animation.InsertEndChild(track);
    }

    doc.InsertEndChild(animation);

    doc.Print(os);

    return true;
}


/*****************************************************************************/
/** Saves a core morph animation instance in a XML file.
  *
  * This function saves a core morph animation instance to a XML file.
  *
  * @param strFilename The name of the file to save the core animation instance
  *                    to.
  * @param pCoreAnimation A pointer to the core animation instance that should
  *                       be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/



bool CalSaver::saveXmlCoreMorphAnimation(const std::string& strFilename, CalCoreMorphAnimation* pCoreMorphAnimation) {
    std::ofstream of(strFilename.c_str());
    return saveXmlCoreMorphAnimation(of, pCoreMorphAnimation);
}

bool CalSaver::saveXmlCoreMorphAnimation(std::ostream& os, CalCoreMorphAnimation* pCoreMorphAnimation) {
    TiXmlDocument doc;
    std::stringstream str;

    TiXmlElement animation("ANIMATION");

    TiXmlElement header("HEADER");
    header.SetAttribute("MAGIC", cal3d::ANIMATEDMORPH_XMLFILE_EXTENSION);
    header.SetAttribute("VERSION", cal3d::LIBRARY_VERSION);

    doc.InsertEndChild(header);

    str.str("");
    str << pCoreMorphAnimation->duration;
    animation.SetAttribute("DURATION", str.str());

    // get core track list
    std::vector<CalCoreMorphTrack>& listCoreMorphTrack = pCoreMorphAnimation->tracks;

    animation.SetAttribute("NUMTRACKS", listCoreMorphTrack.size());

    std::vector<CalCoreMorphTrack>::iterator iteratorCoreMorphTrack;
    for (iteratorCoreMorphTrack = listCoreMorphTrack.begin(); iteratorCoreMorphTrack != listCoreMorphTrack.end(); ++iteratorCoreMorphTrack) {
        CalCoreMorphTrack* pCoreMorphTrack = &(*iteratorCoreMorphTrack);

        TiXmlElement track("TRACK");
        track.SetAttribute("MORPHNAME", pCoreMorphTrack->morphName);

        track.SetAttribute("NUMKEYFRAMES", pCoreMorphTrack->keyframes.size());

        // save all core keyframes
        for (size_t i = 0; i < pCoreMorphTrack->keyframes.size(); ++i) {
            CalCoreMorphKeyframe pCoreMorphKeyframe = pCoreMorphTrack->keyframes[i];

            TiXmlElement keyframe("KEYFRAME");

            str.str("");
            str << pCoreMorphKeyframe.time;
            keyframe.SetAttribute("TIME", str.str());

            TiXmlElement weight("WEIGHT");
            float w = pCoreMorphKeyframe.weight;

            str.str("");
            str << w;

            TiXmlText weightdata(str.str());

            weight.InsertEndChild(weightdata);
            keyframe.InsertEndChild(weight);

            track.InsertEndChild(keyframe);
        }

        animation.InsertEndChild(track);
    }

    doc.InsertEndChild(animation);

    doc.Print(os);

    return true;
}

/*****************************************************************************/
/** Saves a core mesh instance in a XML file.
  *
  * This function saves a core mesh instance to a XML file.
  *
  * @param strFilename The name of the file to save the core mesh instance to.
  * @param pCoreMesh A pointer to the core mesh instance that should be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/



bool CalSaver::saveXmlCoreMesh(const std::string& strFilename, CalCoreMesh* pCoreMesh) {
    std::ofstream of(strFilename.c_str());
    return CalSaver::saveXmlCoreMesh(of, pCoreMesh);
}

bool CalSaver::saveXmlCoreMesh(std::ostream& os, CalCoreMesh* pCoreMesh) {
    TiXmlDocument doc;

    std::stringstream str;

    TiXmlElement header("HEADER");
    header.SetAttribute("MAGIC", cal3d::MESH_XMLFILE_EXTENSION);
    header.SetAttribute("VERSION", cal3d::LIBRARY_VERSION);

    doc.InsertEndChild(header);

    TiXmlElement mesh("MESH");
    mesh.SetAttribute("NUMSUBMESH", pCoreMesh->submeshes.size());

    // get the submesh vector
    CalCoreMesh::CalCoreSubmeshVector& vectorCoreSubmesh = pCoreMesh->submeshes;

    // write all core submeshes
    int submeshId;
    for (submeshId = 0; submeshId < (int)vectorCoreSubmesh.size(); ++submeshId) {
        const CalCoreSubmeshPtr& pCoreSubmesh = vectorCoreSubmesh[submeshId];

        TiXmlElement submesh("SUBMESH");

        const CalCoreSubmesh::MorphTargetArray& vectorMorphs = pCoreSubmesh->getMorphTargets();

        // get the texture coordinate vector vector
        std::vector<std::vector<CalCoreSubmesh::TextureCoordinate>> vectorvectorTextureCoordinate;
        if (pCoreSubmesh->hasTextureCoordinates()) {
            vectorvectorTextureCoordinate.push_back(pCoreSubmesh->getTextureCoordinates());
        }

        submesh.SetAttribute("NUMVERTICES", pCoreSubmesh->getVertexCount());
        submesh.SetAttribute("NUMFACES", pCoreSubmesh->getFaces().size());
        submesh.SetAttribute("MATERIAL", pCoreSubmesh->coreMaterialThreadId);
        submesh.SetAttribute("NUMLODSTEPS", 0);
        submesh.SetAttribute("NUMSPRINGS", 0);
        submesh.SetAttribute("NUMMORPHS", vectorMorphs.size());
        submesh.SetAttribute("NUMTEXCOORDS", vectorvectorTextureCoordinate.size());

        const cal3d::SSEArray<CalCoreSubmesh::Vertex>& vectorVertex = pCoreSubmesh->getVectorVertex();
        const std::vector<CalColor32>& vertexColors = pCoreSubmesh->getVertexColors();

        const std::vector<CalCoreSubmesh::Face>& vectorFace = pCoreSubmesh->getFaces();

        const CalCoreSubmesh::Influence* currentInfluence = cal3d::pointerFromVector(pCoreSubmesh->getInfluences());
        for (int vertexId = 0; vertexId < (int)vectorVertex.size(); ++vertexId) {
            const CalCoreSubmesh::Vertex& Vertex = vectorVertex[vertexId];
            CalColor32 vertexColor = vertexColors[vertexId];

            const CalCoreSubmesh::Influence* nextVertex = currentInfluence;
            while (!nextVertex->lastInfluenceForThisVertex) {
                ++nextVertex;
            }
            ++nextVertex;

            TiXmlElement vertex("VERTEX");
            vertex.SetAttribute("ID", vertexId);
            vertex.SetAttribute("NUMINFLUENCES", size_t(nextVertex - currentInfluence));

            // write the vertex data

            TiXmlElement position("POS");

            str.str("");
            str << Vertex.position.x << " "
                << Vertex.position.y << " "
                << Vertex.position.z;

            TiXmlText positiondata(str.str());

            position.InsertEndChild(positiondata);
            vertex.InsertEndChild(position);

            TiXmlElement normal("NORM");

            str.str("");
            str << Vertex.normal.x << " "
                << Vertex.normal.y << " "
                << Vertex.normal.z;

            TiXmlText normaldata(str.str());

            normal.InsertEndChild(normaldata);
            vertex.InsertEndChild(normal);

            TiXmlElement vertColor("COLOR");

            str.str("");
            CalVector vc(CalVectorFromColor(vertexColor));
            str << vc.x << " " << vc.y << " " << vc.z;

            TiXmlText colordata(str.str());

            vertColor.InsertEndChild(colordata);
            vertex.InsertEndChild(vertColor);

            // write all texture coordinates of this vertex
            int textureCoordinateId;
            for (textureCoordinateId = 0; textureCoordinateId < (int)vectorvectorTextureCoordinate.size(); ++textureCoordinateId) {
                const CalCoreSubmesh::TextureCoordinate& textureCoordinate = vectorvectorTextureCoordinate[textureCoordinateId][vertexId];

                TiXmlElement tex("TEXCOORD");

                str.str("");
                str << textureCoordinate.u << " "
                    << textureCoordinate.v;

                TiXmlText texdata(str.str());

                tex.InsertEndChild(texdata);

                vertex.InsertEndChild(tex);
            }

            for (; currentInfluence != nextVertex; ++currentInfluence) {
                TiXmlElement influence("INFLUENCE");

                influence.SetAttribute("ID", currentInfluence->boneId);

                str.str("");
                str << currentInfluence->weight;

                TiXmlText influencedata(str.str());

                influence.InsertEndChild(influencedata);
                vertex.InsertEndChild(influence);
            }

            submesh.InsertEndChild(vertex);
        }

        // write all morphs
        for (size_t morphId = 0; morphId < vectorMorphs.size(); ++morphId) {
            CalCoreMorphTargetPtr morphTarget = vectorMorphs[morphId];

            TiXmlElement morph("MORPH");
            str.str("");
            str << morphId;

            morph.SetAttribute("MORPHID", str.str());
            morph.SetAttribute("NAME", morphTarget->name);

            int morphVertCount = 0;
            const CalCoreMorphTarget::VertexOffsetArray& vertices = morphTarget->vertexOffsets;
            for (size_t i = 0; i < vertices.size(); ++i) {
                VertexOffset const& bv = vertices[i];

                size_t blendId = bv.vertexId;
                const CalCoreSubmesh::Vertex& Vertex = vectorVertex[blendId];
                static float differenceTolerance = 1.0;
                CalVector positionDiff = bv.position.asCalVector();
                float positionDiffLength = positionDiff.length();

                bool skip = positionDiffLength < differenceTolerance;
                if (skip) {
                    continue;
                }
                morphVertCount++;
                TiXmlElement blendVert("BLENDVERTEX");
                str.str("");
                str << blendId;

                blendVert.SetAttribute("VERTEXID", str.str());
                str.str("");
                str << positionDiffLength;
                blendVert.SetAttribute("POSDIFF", str.str());

                const CalVector4 p = Vertex.position + bv.position;
                const CalVector4 n = Vertex.normal + bv.normal;

                TiXmlElement pos("POSITION");
                str.str("");
                str << p.x << " " << p.y << " " << p.z;
                TiXmlText posdata(str.str());
                pos.InsertEndChild(posdata);

                TiXmlElement norm("NORMAL");
                str.str("");
                str << n.x << " " << n.y << " " << n.z;
                TiXmlText normdata(str.str());
                norm.InsertEndChild(normdata);

                blendVert.InsertEndChild(pos);
                blendVert.InsertEndChild(norm);
                for (size_t tcI = 0; tcI < vectorvectorTextureCoordinate.size(); tcI++) {
                    TiXmlElement tcXml("TEXCOORD");
                    str.str("");
                    str << 0.0f << " " << 0.0f;
                    TiXmlText tcdata(str.str());
                    tcXml.InsertEndChild(tcdata);
                    blendVert.InsertEndChild(tcXml);
                }

                morph.InsertEndChild(blendVert);
            }
            str.str("");
            str << morphVertCount;
            morph.SetAttribute("NUMBLENDVERTS", str.str());
            submesh.InsertEndChild(morph);
        }

        // write all faces
        int faceId;
        for (faceId = 0; faceId < (int)vectorFace.size(); ++faceId) {
            const CalCoreSubmesh::Face& Face = vectorFace[faceId];

            TiXmlElement face("FACE");

            str.str("");
            str << Face.vertexId[0] << " "
                << Face.vertexId[1] << " "
                << Face.vertexId[2];
            face.SetAttribute("VERTEXID", str.str());
            submesh.InsertEndChild(face);
        }

        mesh.InsertEndChild(submesh);

    }
    doc.InsertEndChild(mesh);

    doc.Print(os);
    return true;
}



/*****************************************************************************/
/** Saves a core material instance to a XML file.
  *
  * This function saves a core material instance to a XML file.
  *
  * @param strFilename The name of the file to save the core material instance
  *                    to.
  * @param pCoreMaterial A pointer to the core material instance that should
  *                      be saved.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalSaver::saveXmlCoreMaterial(const std::string& strFilename, CalCoreMaterial* pCoreMaterial) {
    std::stringstream str;

    TiXmlDocument doc(strFilename);


    TiXmlElement header("HEADER");
    header.SetAttribute("MAGIC", cal3d::MATERIAL_XMLFILE_EXTENSION);
    header.SetAttribute("VERSION", cal3d::LIBRARY_VERSION);

    doc.InsertEndChild(header);

    TiXmlElement material("MATERIAL");
    material.SetAttribute("NUMMAPS", pCoreMaterial->maps.size());

    TiXmlElement ambient("AMBIENT");

    str.str("");
    str << 0 << " "
        << 0 << " "
        << 0 << " "
        << 0;

    TiXmlText ambientdata(str.str());


    ambient.InsertEndChild(ambientdata);
    material.InsertEndChild(ambient);

    TiXmlElement diffuse("DIFFUSE");

    str.str("");
    str << 0 << " "
        << 0 << " "
        << 0 << " "
        << 0;

    TiXmlText diffusedata(str.str());


    diffuse.InsertEndChild(diffusedata);
    material.InsertEndChild(diffuse);

    TiXmlElement specular("SPECULAR");

    str.str("");
    str << 0 << " "
        << 0 << " "
        << 0 << " "
        << 0;

    TiXmlText speculardata(str.str());

    specular.InsertEndChild(speculardata);
    material.InsertEndChild(specular);

    TiXmlElement shininess("SHININESS");

    str.str("");
    str << 0.0f;//pCoreMaterial->shininess;

    TiXmlText shininessdata(str.str());

    shininess.InsertEndChild(shininessdata);
    material.InsertEndChild(shininess);

    std::vector<CalCoreMaterial::Map>& vectorMap = pCoreMaterial->maps;

    for (int mapId = 0; mapId < (int)vectorMap.size(); ++mapId) {
        TiXmlElement map("MAP");
        map.SetAttribute("TYPE", vectorMap[mapId].type);
        TiXmlText mapdata(vectorMap[mapId].filename);
        map.InsertEndChild(mapdata);
        material.InsertEndChild(map);
    }

    doc.InsertEndChild(material);

    if (!doc.SaveFile()) {
        CalError::setLastError(CalError::FILE_WRITING_FAILED, __FILE__, __LINE__, strFilename);
        return false;
    }

    return true;

}
