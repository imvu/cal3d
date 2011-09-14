#include <boost/optional.hpp>
#include <sstream>
#include <stdexcept>
#include "cal3d/loader.h"
#include "cal3d/error.h"
#include "cal3d/vector.h"
#include "cal3d/quaternion.h"
#include "cal3d/coreskeleton.h"
#include "cal3d/corebone.h"
#include "cal3d/coreanimation.h"
#include "cal3d/coremorphanimation.h"
#include "cal3d/coretrack.h"
#include "cal3d/corekeyframe.h"
#include "cal3d/coremesh.h"
#include "cal3d/coremorphtrack.h"
#include "cal3d/coremorphkeyframe.h"
#include "cal3d/coresubmesh.h"
#include "cal3d/coremorphtarget.h"
#include "cal3d/corematerial.h"
#include "cal3d/tinyxml.h"
#include "cal3d/buffersource.h"
#include "cal3d/xmlformat.h"
#include "cal3d/calxmlbindings.h"


static inline void ReadPair(char const* buffer, float* f1, float* f2) {
    std::stringstream str(buffer);
    str >> *f1 >> *f2;
}

static inline void ReadPair(char const* buffer, int* f1, int* f2) {
    std::stringstream str(buffer);
    str >> *f1 >> *f2;
}

// atof does wacky things in VC++ 2010 CRT in Serbia...
static inline float imvu_atof(const char* p) {
    std::stringstream str(p);
    float f;
    str >> f;
    return f;
}


#define CAL3D_VALIDATE_XML_TAGS ( 1 )


template<class T>
static inline bool _ValidateTag(
    T* element,
    char const* requiredTag,
    const CalCoreMeshPtr& pCoreMesh,
    const boost::shared_ptr<CalCoreSubmesh>& /*pCoreSubmesh*/,
    char const* file,
    int line
) {
    bool ok = true;
#if CAL3D_VALIDATE_XML_TAGS
    if (!element) {
        ok = false;
    }

    if (ok && requiredTag && cal3d_stricmp(element->Value(), requiredTag) != 0) {
        ok = false;
    }

    if (! ok) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, file, line);
    }
#endif

    return ok;
}

#define ValidateTag(a,b,c,d) _ValidateTag(a,b,c,d,__FILE__,__LINE__)


static inline bool TexCoordFromXml(
    TiXmlElement* texcoord,
    char const* tag,
    CalCoreSubmesh::TextureCoordinate* texCoord,
    const CalCoreMeshPtr& pCoreMesh,
    const boost::shared_ptr<CalCoreSubmesh>& pCoreSubmesh
) {
    if (!ValidateTag(texcoord, tag, pCoreMesh, pCoreSubmesh)) {
        return false;
    }
    TiXmlNode* node = texcoord->FirstChild();
    if (!ValidateTag(node, NULL, pCoreMesh, pCoreSubmesh)) {
        return false;
    }
    TiXmlText* texcoorddata = node->ToText();
    if (!ValidateTag(texcoorddata , NULL, pCoreMesh, pCoreSubmesh)) {
        return false;
    }

    ReadPair(texcoorddata->Value(), &texCoord->u, &texCoord->v);

    return true;
}



static inline void ReadTripleFloat(char const* buffer, float* f1, float* f2, float* f3) {
    std::stringstream str;
    str << buffer;
    str >> *f1 >> *f2 >> *f3;
}



static inline bool CalVectorFromXml(
    TiXmlElement* pos,
    char const* tag,
    CalVector* calVec,
    const CalCoreMeshPtr& pCoreMesh,
    const boost::shared_ptr<CalCoreSubmesh>& pCoreSubmesh
) {
    if (!ValidateTag(pos, tag, pCoreMesh, pCoreSubmesh)) {
        return false;
    }
    TiXmlNode* node = pos->FirstChild();
    if (!ValidateTag(node, NULL, pCoreMesh, pCoreSubmesh)) {
        return false;
    }
    TiXmlText* posdata = node->ToText();
    if (!ValidateTag(posdata, NULL, pCoreMesh, pCoreSubmesh)) {
        return false;
    }
    ReadTripleFloat(posdata->Value(), &calVec->x, &calVec->y, &calVec->z);
    return true;
}


static inline bool CalVectorFromXml(
    TiXmlElement* pos,
    char const* tag,
    CalVector4* calVec,
    const CalCoreMeshPtr& pCoreMesh,
    const boost::shared_ptr<CalCoreSubmesh>& pCoreSubmesh
) {
    CalVector v;
    if (CalVectorFromXml(pos, tag, &v, pCoreMesh, pCoreSubmesh)) {
        calVec->setAsVector(v);
        return true;
    } else {
        return false;
    }
}


static inline bool CalVectorFromXml(
    TiXmlElement* pos,
    char const* tag,
    CalPoint4* calVec,
    const CalCoreMeshPtr& pCoreMesh,
    const boost::shared_ptr<CalCoreSubmesh>& pCoreSubmesh
) {
    CalVector v;
    if (CalVectorFromXml(pos, tag, &v, pCoreMesh, pCoreSubmesh)) {
        calVec->setAsPoint(v);
        return true;
    } else {
        return false;
    }
}


static inline void ReadQuadFloat(char const* buffer, float* f1, float* f2, float* f3, float* f4) {
    std::stringstream str;
    str << buffer;
    str >> *f1 >> *f2 >> *f3 >> *f4;
}

CalCoreSkeletonPtr CalLoader::loadXmlCoreSkeleton(const char* dataSrc) {
    const CalCoreSkeletonPtr null;

    TiXmlDocument doc;

    doc.Parse(dataSrc);
    if (doc.Error()) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        return null;
    }

    return loadXmlCoreSkeletonDoc(doc);
}

CalCoreMeshPtr CalLoader::loadXmlCoreMesh(const char* dataSrc) {
    const CalCoreMeshPtr null;

    TiXmlDocument doc;
    doc.Clear();
    
    doc.Parse(dataSrc);
    if (doc.Error()) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        return null;
    }

    return loadXmlCoreMeshDoc(doc);
}

CalCoreMaterialPtr CalLoader::loadXmlCoreMaterial(const char* dataSrc) {
    const CalCoreMaterialPtr null;

    TiXmlDocument doc;
    
    doc.Parse(dataSrc);
    if (doc.Error()) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        return null;
    }

    return loadXmlCoreMaterialDoc(doc);
}

CalCoreAnimationPtr CalLoader::loadXmlCoreAnimation(const char* dataSrc) {
    TiXmlDocument doc;
    doc.Clear();

    doc.Parse(dataSrc);
    if (doc.Error()) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        return CalCoreAnimationPtr();
    }

    return loadXmlCoreAnimationDoc(doc);
}


CalCoreMorphAnimationPtr CalLoader::loadXmlCoreMorphAnimation(const char* dataSrc) {
    TiXmlDocument doc;
    doc.Clear();

    doc.Parse(dataSrc);
    if (doc.Error()) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        return CalCoreMorphAnimationPtr();
    }

    return loadXmlCoreMorphAnimationDoc(doc);
}

CalCoreSkeletonPtr CalLoader::loadXmlCoreSkeletonDoc(TiXmlDocument& doc) {
    const CalCoreSkeletonPtr null;
    std::stringstream str;

    std::string strFilename = "";

    TiXmlNode* node;
    TiXmlElement* header = doc.FirstChildElement();
    if (!header || cal3d_stricmp(header->Value(), "HEADER") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (!isHeaderWellFormed(header)) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (cal3d_stricmp(header->Attribute("MAGIC"), cal3d::SKELETON_XMLFILE_EXTENSION) != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (atoi(header->Attribute("VERSION")) < cal3d::EARLIEST_COMPATIBLE_FILE_VERSION) {
        CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__, strFilename);
        return null;
    }

    TiXmlElement* skeleton = header->NextSiblingElement();
    if (!skeleton || cal3d_stricmp(skeleton->Value(), "SKELETON") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    boost::optional<CalVector> sceneAmbientColor;

    char const* attrStr = skeleton->Attribute("SCENEAMBIENTCOLOR");
    if (attrStr) {
        CalVector sceneColor;
        ReadTripleFloat(attrStr, &sceneColor.x, &sceneColor.y, &sceneColor.z);
        sceneAmbientColor = sceneColor;
    }

    std::vector<CalCoreBonePtr> bones;

    for (TiXmlElement* bone = skeleton->FirstChildElement(); bone; bone = bone->NextSiblingElement()) {
        if (cal3d_stricmp(bone->Value(), "BONE") != 0) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }

        std::string strName = bone->Attribute("NAME");

        CalLightType lightType = LIGHT_TYPE_NONE;
        CalVector lightColor;

        char const* attrStr = bone->Attribute("LIGHTTYPE");
        if (attrStr) {
            lightType = (CalLightType)atoi(attrStr);
        }

        attrStr = bone->Attribute("LIGHTCOLOR");
        if (attrStr) {
            ReadTripleFloat(attrStr, &lightColor.x, &lightColor.y, &lightColor.z);
        }


        // get the translation of the bone

        TiXmlElement* translation = bone->FirstChildElement();
        if (!translation || cal3d_stricmp(translation->Value(), "TRANSLATION") != 0) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }

        float tx, ty, tz;

        node = translation->FirstChild();
        if (!node) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }
        TiXmlText* translationdata = node->ToText();
        if (!translationdata) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }
        str.clear();
        str << translationdata->Value();
        str >> tx >> ty >> tz;

        // get the rotation of the bone

        TiXmlElement* rotation = translation->NextSiblingElement();
        if (!rotation || cal3d_stricmp(rotation->Value(), "ROTATION") != 0) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }

        float rx, ry, rz, rw;

        node = rotation->FirstChild();
        if (!node) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }
        TiXmlText* rotationdata = node->ToText();
        if (!rotationdata) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }
        str.clear();
        str << rotationdata->Value();
        str >> rx >> ry >> rz >> rw;

        // get the bone space translation of the bone


        TiXmlElement* translationBoneSpace = rotation->NextSiblingElement();
        if (!rotation || cal3d_stricmp(translationBoneSpace->Value(), "LOCALTRANSLATION") != 0) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }

        float txBoneSpace, tyBoneSpace, tzBoneSpace;

        node = translationBoneSpace->FirstChild();
        if (!node) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }
        TiXmlText* translationBoneSpacedata = node->ToText();
        if (!translationBoneSpacedata) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }
        str.clear();
        str << translationBoneSpacedata->Value();
        str >> txBoneSpace >> tyBoneSpace >> tzBoneSpace;

        // get the bone space rotation of the bone

        TiXmlElement* rotationBoneSpace = translationBoneSpace->NextSiblingElement();
        if (!rotationBoneSpace || cal3d_stricmp(rotationBoneSpace->Value(), "LOCALROTATION") != 0) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }

        float rxBoneSpace, ryBoneSpace, rzBoneSpace, rwBoneSpace;

        node = rotationBoneSpace->FirstChild();
        if (!node) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }
        TiXmlText* rotationBoneSpacedata = node->ToText();
        if (!rotationBoneSpacedata) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }
        str.clear();
        str << rotationBoneSpacedata->Value();
        str >> rxBoneSpace >> ryBoneSpace >> rzBoneSpace >> rwBoneSpace;

        // get the parent bone id

        TiXmlElement* parent = rotationBoneSpace->NextSiblingElement();
        if (!parent || cal3d_stricmp(parent->Value(), "PARENTID") != 0) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }


        node = parent->FirstChild();
        if (!node) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }
        TiXmlText* parentid = node->ToText();
        if (!parentid) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }
        int parentId = atoi(parentid->Value());

        boost::shared_ptr<CalCoreBone> pCoreBone(new CalCoreBone(strName, parentId));

        CalVector trans = CalVector(tx, ty, tz);
        CalQuaternion rot = CalQuaternion(rx, ry, rz, rw);

        pCoreBone->relativeTransform.translation = trans;
        pCoreBone->relativeTransform.rotation = rot;
        pCoreBone->boneSpaceTransform.translation = CalVector(txBoneSpace, tyBoneSpace, tzBoneSpace);
        pCoreBone->boneSpaceTransform.rotation = CalQuaternion(rxBoneSpace, ryBoneSpace, rzBoneSpace, rwBoneSpace);

        pCoreBone->lightType = lightType;
        pCoreBone->lightColor = lightColor;

        TiXmlElement* child;
        for (child = parent->NextSiblingElement(); child; child = child->NextSiblingElement()) {
            if (cal3d_stricmp(child->Value(), "CHILDID") != 0) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }

            TiXmlNode* node = child->FirstChild();
            if (!node) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
            TiXmlText* childid = node->ToText();
            if (!childid) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
        }

        bones.push_back(pCoreBone);
    }

    CalCoreSkeletonPtr pCoreSkeleton(new CalCoreSkeleton(bones));
    if (sceneAmbientColor) {
        pCoreSkeleton->sceneAmbientColor = *sceneAmbientColor;
    }
    return pCoreSkeleton;
}

CalCoreAnimationPtr CalLoader::loadXmlCoreAnimationDoc(TiXmlDocument& doc) {
    const CalCoreAnimationPtr null;
    std::stringstream str;

    const std::string strFilename = "";
    TiXmlNode* node;

    TiXmlElement* header = doc.FirstChildElement();
    if (!header || cal3d_stricmp(header->Value(), "HEADER") != 0) {
        str.clear();
        str << "Header element is " << (header ? header->Value() : "<unknown>");
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, str.str());
        return null;
    }

    if (!isHeaderWellFormed(header)) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (cal3d_stricmp(header->Attribute("MAGIC"), cal3d::ANIMATION_XMLFILE_EXTENSION) != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (atoi(header->Attribute("VERSION")) < cal3d::EARLIEST_COMPATIBLE_FILE_VERSION) {
        CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__, strFilename);
        return null;
    }

    TiXmlElement* animation = header->NextSiblingElement();
    if (!animation || cal3d_stricmp(animation->Value(), "ANIMATION") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    float duration = imvu_atof(animation->Attribute("DURATION"));

    // allocate a new core animation instance
    CalCoreAnimationPtr pCoreAnimation(new CalCoreAnimation);

    // check for a valid duration
    if (duration <= 0.0f) {
        CalError::setLastError(CalError::INVALID_ANIMATION_DURATION, __FILE__, __LINE__, strFilename);
        return null;
    }

    pCoreAnimation->duration = duration;
    for (
        TiXmlElement* track = animation->FirstChildElement();
        track;
        track = track->NextSiblingElement()
    ) {
        if (!track || cal3d_stricmp(track->Value(), "TRACK") != 0) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }

        int coreBoneId = atoi(track->Attribute("BONEID"));

        CalCoreTrack::KeyframeList keyframes;

        const char* trstr = track->Attribute("TRANSLATIONREQUIRED");
        bool translationRequired = true; // Default value if flag is not supplied (for backwards compatibility).
        if (trstr) {
            translationRequired = atoi(trstr) ? true : false;
        }

        const char* trstr2 = track->Attribute("HIGHRANGEREQUIRED");
        bool highRangeRequired = true; // Default value if flag is not supplied (for backwards compatibility).
        if (trstr2) {
            highRangeRequired = atoi(trstr2) ? true : false;
        }

        const char* trstr3 = track->Attribute("TRANSLATIONISDYNAMIC");
        bool translationIsDynamic = true; // Default value if flag is not supplied (for backwards compatibility).
        if (trstr3) {
            translationIsDynamic = atoi(trstr3) ? true : false;
        }

        // XML files may or may not have a translationRequired flag.  The default value is true if it is not supplied.
        // XML files may or may not have the translation in keyframes, but if they don't, then either (a) they do
        // have a translationRequired flag and it is false, or (b) they have a false translationIsDynamic flag AND
        // the keyframe is not the first.
        // If XML files do not have the flag, then we assume translation is required until we decide otherwise.
        // If the caller has passed in a skeleton (not NULL for the skeleton), then we will re-derive
        // whether translation is required, and we will update the translationRequired flag.

        // read the number of keyframes
        int keyframeCount = atoi(track->Attribute("NUMKEYFRAMES"));

        if (keyframeCount <= 0) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }

        // load all core keyframes
        bool hasLastKeyframe = false;
        CalCoreKeyframe prevCoreKeyframe;
        for (
            TiXmlElement* keyframe = track->FirstChildElement();
            keyframe;
            keyframe = keyframe->NextSiblingElement()
        ) {
            // load the core keyframe
            if (!keyframe || cal3d_stricmp(keyframe->Value(), "KEYFRAME") != 0) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }

            float time = imvu_atof(keyframe->Attribute("TIME"));

            // Translation component in the XML is now optional.
            // I first fill the translation with zero.
            // Then if I have a skeleton, I fill it with the values from the skeleton.
            // Then if I have an XML translation entry, I fill it with the value from that entry.
            TiXmlElement* translation = keyframe->FirstChildElement();
            TiXmlElement* rotation = translation;
            CalVector t = InvalidTranslation;

            // If translation is required but not dynamic, then I may elide the translation
            // values for all but the first frame, and for each frame's translation I will
            // copy the translation from the previous frame.
            if (hasLastKeyframe && !translationIsDynamic && translationRequired) {
                t = prevCoreKeyframe.transform.translation;
            }

            if (!translation) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
            if (cal3d_stricmp(translation->Value(), "TRANSLATION") == 0) {
                node = translation->FirstChild();
                if (!node) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }

                TiXmlText* translationdata = node->ToText();
                if (!translationdata) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
                ReadTripleFloat(translationdata->Value(), &t.x, &t.y, &t.z);
                rotation = rotation->NextSiblingElement();
            }

            if (!rotation || cal3d_stricmp(rotation->Value(), "ROTATION") != 0) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }

            float rx, ry, rz, rw;

            node = rotation->FirstChild();
            if (!node) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
            TiXmlText* rotationdata = node->ToText();
            if (!rotationdata) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
            ReadQuadFloat(rotationdata->Value(), &rx, &ry, &rz, &rw);

            CalCoreKeyframe pCoreKeyframe(time, t, CalQuaternion(rx, ry, rz, rw));
            hasLastKeyframe = true;
            prevCoreKeyframe = pCoreKeyframe;

            keyframes.push_back(pCoreKeyframe);
        }
        CalCoreTrackPtr pCoreTrack(new CalCoreTrack(coreBoneId, keyframes));
        pCoreTrack->translationRequired = translationRequired;
        pCoreTrack->translationIsDynamic = translationIsDynamic;
        pCoreAnimation->tracks.push_back(*pCoreTrack);
    }

    // explicitly close the file
    doc.Clear();

    return pCoreAnimation;
}

CalCoreMorphAnimationPtr CalLoader::loadXmlCoreMorphAnimationDoc(TiXmlDocument& doc) {
    const CalCoreMorphAnimationPtr null;

    std::string strFilename = "";

    TiXmlElement* header = doc.FirstChildElement();
    if (!header || cal3d_stricmp(header->Value(), "HEADER") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (!isHeaderWellFormed(header)) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (cal3d_stricmp(header->Attribute("MAGIC"), cal3d::ANIMATEDMORPH_XMLFILE_EXTENSION) != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (atoi(header->Attribute("VERSION")) < cal3d::EARLIEST_COMPATIBLE_FILE_VERSION) {
        CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__, strFilename);
        return null;
    }

    // allocate a new core animatedMorph instance
    CalCoreMorphAnimationPtr pCoreMorphAnimation(new CalCoreMorphAnimation);

    TiXmlElement* animatedMorph = header->NextSiblingElement();
    if (!animatedMorph || cal3d_stricmp(animatedMorph->Value(), "ANIMATION") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (!BindFromXml(*animatedMorph, pCoreMorphAnimation.get())) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    // check for a valid duration
    if (pCoreMorphAnimation->duration <= 0.0f) {
        CalError::setLastError(CalError::INVALID_ANIMATION_DURATION, __FILE__, __LINE__, strFilename);
        return null;
    }

    // explicitly close the file
    doc.Clear();

    return pCoreMorphAnimation;
}

/*****************************************************************************/
/** Loads a core mesh instance from a Xml file.
*
* This function loads a core mesh instance from a Xml file.
*
* @param strFilename The name of the file to load the core mesh instance from.
*
* @return One of the following values:
*         \li a pointer to the core mesh
*         \li \b 0 if an error happened
*****************************************************************************/

CalCoreMeshPtr CalLoader::loadXmlCoreMeshDoc(TiXmlDocument& doc) {
    const CalCoreMeshPtr null;

    std::string strFilename = "";

    TiXmlNode* node;

    TiXmlElement* header = doc.FirstChildElement();
    if (!header || cal3d_stricmp(header->Value(), "HEADER") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (!isHeaderWellFormed(header)) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (cal3d_stricmp(header->Attribute("MAGIC"), cal3d::MESH_XMLFILE_EXTENSION) != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    int version = atoi(header->Attribute("VERSION"));
    if (version < cal3d::EARLIEST_COMPATIBLE_FILE_VERSION) {
        CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__, strFilename);
        return null;
    }

    bool hasVertexColors = (version >= cal3d::FIRST_FILE_VERSION_WITH_VERTEX_COLORS);

    TiXmlElement* mesh = header->NextSiblingElement();
    if (!mesh || cal3d_stricmp(mesh->Value(), "MESH") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    // get the number of submeshes
    int submeshCount = atoi(mesh->Attribute("NUMSUBMESH"));

    // allocate a new core mesh instance
    CalCoreMeshPtr pCoreMesh(new CalCoreMesh);

    TiXmlElement* submesh = mesh->FirstChildElement();

    // load all core submeshes
    int submeshId;
    for (submeshId = 0; submeshId < submeshCount; ++submeshId) {
        if (!submesh || cal3d_stricmp(submesh->Value(), "SUBMESH") != 0) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }

        // get the material thread id of the submesh
        int coreMaterialThreadId = atoi(submesh->Attribute("MATERIAL"));

        // get the number of vertices, faces, level-of-details and springs
        int vertexCount = atoi(submesh->Attribute("NUMVERTICES"));

        int faceCount = atoi(submesh->Attribute("NUMFACES"));

        int springCount = atoi(submesh->Attribute("NUMSPRINGS"));

        int textureCoordinateCount = atoi(submesh->Attribute("NUMTEXCOORDS"));

        char const* numMorphStr = submesh->Attribute("NUMMORPHS");
        int morphCount = 0;
        if (numMorphStr) {
            morphCount = atoi(numMorphStr);
        }

        boost::shared_ptr<CalCoreSubmesh> pCoreSubmesh(new CalCoreSubmesh(vertexCount, textureCoordinateCount, faceCount));

        pCoreSubmesh->coreMaterialThreadId = coreMaterialThreadId;

        TiXmlElement* vertex = submesh->FirstChildElement();

        for (int vertexId = 0; vertexId < vertexCount; ++vertexId) {
            if (!ValidateTag(vertex, "VERTEX", pCoreMesh, pCoreSubmesh)) {
                return null;
            }
            CalCoreSubmesh::Vertex Vertex;
            CalColor32 vertexColor = CalMakeColor(CalVector(1.0f, 1.0f, 1.0f));

            TiXmlElement* pos = vertex->FirstChildElement();
            if (!ValidateTag(pos, "POS", pCoreMesh, pCoreSubmesh)) {
                return null;
            }
            node = pos->FirstChild();
            if (!ValidateTag(node, NULL, pCoreMesh, pCoreSubmesh)) {
                return null;
            }
            TiXmlText* posdata = node->ToText();
            if (!ValidateTag(posdata, NULL, pCoreMesh, pCoreSubmesh)) {
                return null;
            }
            ReadTripleFloat(posdata->Value(), &Vertex.position.x, &Vertex.position.y, &Vertex.position.z);

            TiXmlElement* norm = pos->NextSiblingElement();
            if (!ValidateTag(norm, "NORM", pCoreMesh, pCoreSubmesh)) {
                return null;
            }
            node = norm->FirstChild();
            if (!node) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
            TiXmlText* normdata = node->ToText();
            if (!normdata) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }

            ReadTripleFloat(normdata->Value(),  &Vertex.normal.x, &Vertex.normal.y, &Vertex.normal.z);

            TiXmlElement* vertColor = norm->NextSiblingElement();
            TiXmlElement* collapse = 0;
            if (!vertColor || cal3d_stricmp(vertColor->Value(), "COLOR") != 0) {
                if (hasVertexColors) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                } else {
                    collapse = vertColor;
                }
            } else {
                node = vertColor->FirstChild();
                if (!node) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
                TiXmlText* vcdata = node->ToText();
                if (!vcdata) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
                CalVector vc(1.0f, 1.0f, 1.0f);
                ReadTripleFloat(vcdata->Value(), &vc.x, &vc.y, &vc.z);
                vertexColor = CalMakeColor(vc);

                collapse = vertColor->NextSiblingElement();
            }
            if (!collapse) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
            if (cal3d_stricmp(collapse->Value(), "COLLAPSEID") == 0) {
                node = collapse->FirstChild();
                if (!node) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
                TiXmlText* collapseid = node->ToText();
                if (!collapseid) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
                TiXmlElement* collapseCount = collapse->NextSiblingElement();
                if (!collapseCount || cal3d_stricmp(collapseCount->Value(), "COLLAPSECOUNT") != 0) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }

                node = collapseCount->FirstChild();
                if (!node) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
                TiXmlText* collapseCountdata = node->ToText();
                if (!collapseCountdata) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
                collapse = collapseCount->NextSiblingElement();
            }


            TiXmlElement* texcoord = collapse;

            // load all texture coordinates of the vertex
            int textureCoordinateId;
            for (textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; ++textureCoordinateId) {
                CalCoreSubmesh::TextureCoordinate textureCoordinate;
                // load data of the influence
#if CAL3D_VALIDATE_XML_TAGS
                if (!texcoord || cal3d_stricmp(texcoord->Value(), "TEXCOORD") != 0) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
#endif
                node = texcoord->FirstChild();
                if (!node) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
                TiXmlText* texcoorddata = node->ToText();
                if (!texcoorddata) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }

                ReadPair(texcoorddata->Value(), &textureCoordinate.u, &textureCoordinate.v);

                // set texture coordinate in the core submesh instance
                pCoreSubmesh->setTextureCoordinate(vertexId, textureCoordinateId, textureCoordinate);
                texcoord = texcoord->NextSiblingElement();
            }

            // get the number of influences
            int influenceCount = atoi(vertex->Attribute("NUMINFLUENCES"));

            if (influenceCount < 0) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }

            std::vector<CalCoreSubmesh::Influence> influences(influenceCount);

            TiXmlElement* influence = texcoord;

            // load all influences of the vertex
            int influenceId;
            for (influenceId = 0; influenceId < influenceCount; ++influenceId) {
#if CAL3D_VALIDATE_XML_TAGS
                if (!influence || cal3d_stricmp(influence->Value(), "INFLUENCE") != 0) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
#endif
                node = influence->FirstChild();
                if (!node) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
                TiXmlText* influencedata = node->ToText();
                if (!influencedata) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }

                influences[influenceId].boneId = atoi(influence->Attribute("ID"));
                influences[influenceId].weight = imvu_atof(influencedata->Value());

                influence = influence->NextSiblingElement();
            }

            pCoreSubmesh->addVertex(Vertex, vertexColor, influences);
            vertex = vertex->NextSiblingElement();
        }

        TiXmlElement* spring = vertex;

        // load all springs
        for (int springId = 0; springId < springCount; ++springId) {
            if (!spring || cal3d_stricmp(spring->Value(), "SPRING") != 0) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
            spring = spring->NextSiblingElement();
        }

        TiXmlElement* face = spring;

        TiXmlElement* morph = face;
        for (int morphId = 0; morphId < morphCount; morphId++) {
            if (cal3d_stricmp(morph->Value(), "MORPH") != 0) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
            CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget(morph->Attribute("NAME")));
            morphTarget->reserve(vertexCount);

            TiXmlElement* blendVert = morph->FirstChildElement();
            for (int blendVertI = 0; blendVertI < vertexCount; blendVertI++) {
                CalCoreMorphTarget::BlendVertex Vertex;
                Vertex.textureCoords.clear();
                Vertex.textureCoords.reserve(textureCoordinateCount);

                bool copyOrig = true;
                if (blendVert && !cal3d_stricmp(blendVert->Value(), "BLENDVERTEX")) {
                    int vertId = atoi(blendVert->Attribute("VERTEXID"));

                    if (vertId == blendVertI) {
                        copyOrig = false;
                    }
                }

                if (!copyOrig) {
                    if (!ValidateTag(blendVert, "BLENDVERTEX", pCoreMesh, pCoreSubmesh)) {
                        return null;
                    }

                    TiXmlElement* pos = blendVert->FirstChildElement();
                    if (!CalVectorFromXml(pos, "POSITION", &Vertex.position, pCoreMesh, pCoreSubmesh)) {
                        return null;
                    }

                    TiXmlElement* norm = pos->NextSiblingElement();
                    if (!CalVectorFromXml(norm, "NORMAL", &Vertex.normal, pCoreMesh, pCoreSubmesh)) {
                        return null;
                    }

                    TiXmlElement* texcoord = norm->NextSiblingElement();
                    int textureCoordinateId;
                    for (textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; ++textureCoordinateId) {
                        CalCoreSubmesh::TextureCoordinate textureCoordinate;
                        if (
                            !TexCoordFromXml(
                                texcoord,
                                "TEXCOORD",
                                &textureCoordinate,
                                pCoreMesh,
                                pCoreSubmesh)
                        ) {
                            return null;
                        }
                        Vertex.textureCoords.push_back(textureCoordinate);
                        texcoord = texcoord->NextSiblingElement();
                    }
                    blendVert = blendVert->NextSiblingElement();
                    morphTarget->setBlendVertex(blendVertI, Vertex);
                }
            }
            pCoreSubmesh->addCoreSubMorphTarget(morphTarget);

            morph = morph->NextSiblingElement();
        }

        face = morph;
        // load all faces
        int faceId;
        for (faceId = 0; faceId < faceCount; ++faceId) {
            CalCoreSubmesh::Face Face;

#if CAL3D_VALIDATE_XML_TAGS
            if (!face || cal3d_stricmp(face->Value(), "FACE") != 0) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
#endif
            int tmp[3];

            // load data of the face
            std::stringstream str;
            str << face->Attribute("VERTEXID");
            str >> tmp[0] >> tmp [1] >> tmp[2];

            if (sizeof(CalIndex) == 2) {
                if (tmp[0] > 65535 || tmp[1] > 65535 || tmp[2] > 65535) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
            }
            Face.vertexId[0] = tmp[0];
            Face.vertexId[1] = tmp[1];
            Face.vertexId[2] = tmp[2];

            pCoreSubmesh->faces[faceId] = Face;

            face = face->NextSiblingElement();
        }
        submesh = submesh->NextSiblingElement();

        // add the core submesh to the core mesh instance
        pCoreMesh->submeshes.push_back(pCoreSubmesh);

    }


    // explicitly close the file
    doc.Clear();
    return pCoreMesh;
}


/*****************************************************************************/
/** Loads a core material instance from a XML file.
*
* This function loads a core material instance from a XML file.
*
* @param strFilename The name of the file to load the core material instance
*                    from.
*
* @return One of the following values:
*         \li a pointer to the core material
*         \li \b 0 if an error happened
*****************************************************************************/


CalCoreMaterialPtr CalLoader::loadXmlCoreMaterialDoc(TiXmlDocument& doc) {
    const CalCoreMaterialPtr null;

    std::stringstream str;

    const std::string strFilename = "";
    TiXmlNode* node;

    TiXmlElement* header = doc.FirstChildElement();
    if (!header || cal3d_stricmp(header->Value(), "HEADER") != 0) {
        str.clear();
        str << "Header element is " << (header ? header->Value() : "<unknown>");
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, str.str());
        return null;
    }

    if (!isHeaderWellFormed(header)) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (cal3d_stricmp(header->Attribute("MAGIC"), cal3d::MATERIAL_XMLFILE_EXTENSION) != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (atoi(header->Attribute("VERSION")) < cal3d::EARLIEST_COMPATIBLE_FILE_VERSION) {
        CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__, strFilename);
        return null;
    }

    TiXmlElement* material = header->NextSiblingElement();
    if (!material || cal3d_stricmp(material->Value(), "MATERIAL") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }


    CalCoreMaterialPtr pCoreMaterial(new CalCoreMaterial);

    TiXmlElement* ambient = material->FirstChildElement();
    if (!ambient || cal3d_stricmp(ambient->Value(), "AMBIENT") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    node = ambient->FirstChild();
    if (!node) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }
    TiXmlText* ambientdata = node->ToText();
    if (!ambientdata) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    int r, g, b, a;

    str << ambientdata->Value();
    str >> r >> g >> b >> a;

    TiXmlElement* diffuse = ambient->NextSiblingElement();
    if (!diffuse || cal3d_stricmp(diffuse->Value(), "DIFFUSE") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    node = diffuse->FirstChild();
    if (!node) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }
    TiXmlText* diffusedata = node->ToText();
    if (!diffusedata) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }
    str.clear();
    str << diffusedata->Value();
    str >> r >> g >> b >> a;


    TiXmlElement* specular = diffuse->NextSiblingElement();
    if (!specular || cal3d_stricmp(specular->Value(), "SPECULAR") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    node = specular->FirstChild();
    if (!node) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }
    TiXmlText* speculardata = node->ToText();
    if (!speculardata) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }
    str.clear();
    str << speculardata->Value();
    str >> r >> g >> b >> a;


    TiXmlElement* shininess = specular->NextSiblingElement();
    if (!shininess || cal3d_stricmp(shininess->Value(), "SHININESS") != 0) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    float fshininess;
    node = shininess->FirstChild();
    if (!node) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }
    TiXmlText* shininessdata = node->ToText();
    if (!shininessdata) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }
    fshininess = imvu_atof(shininessdata->Value());

    std::vector<std::string> MatFileName;
    std::vector<std::string> MatTypes;

    TiXmlElement* map;

    for (map = shininess->NextSiblingElement(); map; map = map->NextSiblingElement()) {
        if (!map || cal3d_stricmp(map->Value(), "MAP") != 0) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }


        node = map->FirstChild();
        if (!node) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }

        TiXmlText* mapfile = node->ToText();
        if (!mapfile) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }

        MatFileName.push_back(mapfile->Value());
        std::string mapType = "Diffuse Color";

        if (map->Attribute("TYPE")) {
            mapType = map->Attribute("TYPE");
        }
        MatTypes.push_back(mapType);
    }

    for (unsigned int mapId = 0; mapId < MatFileName.size(); ++mapId) {
        CalCoreMaterial::Map Map;

        Map.filename = MatFileName[mapId];
        Map.type = MatTypes[mapId];

        // set map in the core material instance
        pCoreMaterial->maps.push_back(Map);
    }

    doc.Clear();

    return pCoreMaterial;
}
