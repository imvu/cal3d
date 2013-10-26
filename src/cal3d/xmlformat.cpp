#include <boost/optional.hpp>
#include <sstream>
#include <stdexcept>
#include <rapidxml.hpp>
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
    const CalCoreMeshPtr& /*pCoreMesh*/,
    const CalCoreSubmeshPtr& /*pCoreSubmesh*/,
    char const* file,
    int line
) {
    bool ok = true;
#if CAL3D_VALIDATE_XML_TAGS
    if (!element) {
        ok = false;
    }

    if (ok && requiredTag && !has_name(element, requiredTag)) {
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
    const CalCoreSubmeshPtr& pCoreSubmesh
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

static inline bool TexCoordFromXml(
    rapidxml::xml_node<>* texcoord,
    char const* tag,
    CalCoreSubmesh::TextureCoordinate* texCoord,
    const CalCoreMeshPtr& pCoreMesh,
    const CalCoreSubmeshPtr& pCoreSubmesh
) {
    if (!ValidateTag(texcoord, tag, pCoreMesh, pCoreSubmesh)) {
        return false;
    }
    if (!texcoord->value()) {
        return false;
    } else {
        ReadPair(texcoord->value(), &texCoord->u, &texCoord->v);
        return true;
    }
}


static inline void ReadTripleFloat(char const* buffer, float* f1, float* f2, float* f3) {
    std::stringstream str;
    str << buffer;
    str >> *f1 >> *f2 >> *f3;
}

static inline bool CalVectorFromXml(
    rapidxml::xml_node<>* pos,
    char const* tag,
    CalVector* calVec,
    const CalCoreMeshPtr& pCoreMesh,
    const CalCoreSubmeshPtr& pCoreSubmesh
) {
    if (!ValidateTag(pos, tag, pCoreMesh, pCoreSubmesh)) {
        return false;
    }
    if (pos->value()) {
        ReadTripleFloat(pos->value(), &calVec->x, &calVec->y, &calVec->z);
        return true;
    } else {
        return false;
    }
}

static inline bool CalVectorFromXml(
    TiXmlElement* pos,
    char const* tag,
    CalVector* calVec,
    const CalCoreMeshPtr& pCoreMesh,
    const CalCoreSubmeshPtr& pCoreSubmesh
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
    rapidxml::xml_node<>* pos,
    char const* tag,
    CalVector4* calVec,
    const CalCoreMeshPtr& pCoreMesh,
    const CalCoreSubmeshPtr& pCoreSubmesh
) {
    CalVector v;
    if (CalVectorFromXml(pos, tag, &v, pCoreMesh, pCoreSubmesh)) {
        *calVec = CalVector4(v);
        return true;
    } else {
        return false;
    }
}

static inline bool CalVectorFromXml(
    TiXmlElement* pos,
    char const* tag,
    CalVector4* calVec,
    const CalCoreMeshPtr& pCoreMesh,
    const CalCoreSubmeshPtr& pCoreSubmesh
) {
    CalVector v;
    if (CalVectorFromXml(pos, tag, &v, pCoreMesh, pCoreSubmesh)) {
        *calVec = CalVector4(v);
        return true;
    } else {
        return false;
    }
}

static inline bool CalVectorFromXml(
    rapidxml::xml_node<>* pos,
    char const* tag,
    CalPoint4* calVec,
    const CalCoreMeshPtr& pCoreMesh,
    const CalCoreSubmeshPtr& pCoreSubmesh
) {
    CalVector v;
    if (CalVectorFromXml(pos, tag, &v, pCoreMesh, pCoreSubmesh)) {
        *calVec = CalPoint4(v);
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
    const CalCoreSubmeshPtr& pCoreSubmesh
) {
    CalVector v;
    if (CalVectorFromXml(pos, tag, &v, pCoreMesh, pCoreSubmesh)) {
        *calVec = CalPoint4(v);
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

CalCoreSkeletonPtr CalLoader::loadXmlCoreSkeleton(std::vector<char>& dataSrc) {
    const CalCoreSkeletonPtr null;

    TiXmlDocument doc;

    doc.Parse(cal3d::pointerFromVector(dataSrc));
    if (doc.Error()) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        return null;
    }

    return loadXmlCoreSkeletonDoc(doc);
}

CalCoreMeshPtr CalLoader::loadXmlCoreMesh(std::vector<char>& dataSrc) {
    rapidxml::xml_document<> document;
    try {
        document.parse<rapidxml::parse_no_data_nodes | rapidxml::parse_no_entity_translation>(cal3d::pointerFromVector(dataSrc));
    } catch (const rapidxml::parse_error&) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        return CalCoreMeshPtr();
    }

    return loadXmlCoreMeshDoc(document);
}

CalCoreMaterialPtr CalLoader::loadXmlCoreMaterial(std::vector<char>& dataSrc) {
    const CalCoreMaterialPtr null;

    TiXmlDocument doc;
    
    doc.Parse(cal3d::pointerFromVector(dataSrc));
    if (doc.Error()) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        return null;
    }

    return loadXmlCoreMaterialDoc(doc);
}

CalCoreAnimationPtr CalLoader::loadXmlCoreAnimation(std::vector<char>& dataSrc) {
    TiXmlDocument doc;
    doc.Clear();

    doc.Parse(cal3d::pointerFromVector(dataSrc));
    if (doc.Error()) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        return CalCoreAnimationPtr();
    }

    return loadXmlCoreAnimationDoc(doc);
}


CalCoreMorphAnimationPtr CalLoader::loadXmlCoreMorphAnimation(std::vector<char>& dataSrc) {
    TiXmlDocument doc;
    doc.Clear();

    doc.Parse(cal3d::pointerFromVector(dataSrc));
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
        if (!translationBoneSpace || cal3d_stricmp(translationBoneSpace->Value(), "LOCALTRANSLATION") != 0) {
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

        CalCoreBonePtr pCoreBone(new CalCoreBone(strName, parentId));

        CalVector trans = CalVector(tx, ty, tz);
        CalQuaternion rot = CalQuaternion(rx, ry, rz, rw);

        pCoreBone->relativeTransform.translation = trans;
        pCoreBone->relativeTransform.rotation = rot;
        pCoreBone->inverseBindPoseTransform.translation = CalVector(txBoneSpace, tyBoneSpace, tzBoneSpace);
        pCoreBone->inverseBindPoseTransform.rotation = CalQuaternion(rxBoneSpace, ryBoneSpace, rzBoneSpace, rwBoneSpace);

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

static bool has_name(TiXmlNode* node, const char* name) {
    return 0 == cal3d_stricmp(node->Value(), name);
}

static bool has_name(rapidxml::xml_node<>* node, const char* name) {
    return 0 == cal3d_stricmp(node->name(), name);
}

static bool has_attribute_value(rapidxml::xml_node<>* node, const char* name, const char* value) {
    auto attribute = node->first_attribute(name, 0, false);
    if (!attribute) {
        return false;
    }
    return 0 == cal3d_stricmp(attribute->value(), value);
}

static int get_int_attribute(rapidxml::xml_node<>* node, const char* name) {
    auto attribute = node->first_attribute(name, 0, false);
    if (!attribute) {
        return 0;
    }
    return atoi(attribute->value());
}

static const char* get_string_attribute(rapidxml::xml_node<>* node, const char* name) {
    auto attribute = node->first_attribute(name, 0, false);
    if (!attribute) {
        return "";
    }
    return attribute->value() ? attribute->value() : "";
}

CalCoreMeshPtr CalLoader::loadXmlCoreMeshDoc(const rapidxml::xml_document<>& doc) {
    typedef rapidxml::xml_node<> xml_node;

    const CalCoreMeshPtr null;

    std::string strFilename = "";

    xml_node* header = doc.first_node();
    if (!header || !has_name(header, "header")) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (!isHeaderWellFormed(header)) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    if (!has_attribute_value(header, "magic", cal3d::MESH_XMLFILE_EXTENSION)) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    int version = get_int_attribute(header, "version");
    if (version < cal3d::EARLIEST_COMPATIBLE_FILE_VERSION) {
        CalError::setLastError(CalError::INCOMPATIBLE_FILE_VERSION, __FILE__, __LINE__, strFilename);
        return null;
    }

    bool hasVertexColors = (version >= cal3d::FIRST_FILE_VERSION_WITH_VERTEX_COLORS);

    xml_node* mesh = header->next_sibling();
    if (!mesh || !has_name(mesh, "mesh")) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
        return null;
    }

    //  get the number of submeshes
    int submeshCount = get_int_attribute(mesh, "numsubmesh");

    CalCoreMeshPtr pCoreMesh(new CalCoreMesh);

    xml_node* submesh = mesh->first_node();

    // load all core submeshes
    for (int submeshId = 0; submeshId < submeshCount; ++submeshId) {
        if (!submesh || !has_name(submesh, "submesh")) {
            CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
            return null;
        }

        int coreMaterialThreadId = get_int_attribute(submesh, "MATERIAL");
        int vertexCount = get_int_attribute(submesh, "numvertices");
        int faceCount = get_int_attribute(submesh, "NUMFACES");
        int springCount = get_int_attribute(submesh, "NUMSPRINGS");
        int textureCoordinateCount = get_int_attribute(submesh, "NUMTEXCOORDS");
        int morphCount = get_int_attribute(submesh, "nummorphs");

        CalCoreSubmeshPtr pCoreSubmesh(new CalCoreSubmesh(vertexCount, textureCoordinateCount ? true : false, faceCount));
        pCoreSubmesh->coreMaterialThreadId = coreMaterialThreadId;

        xml_node* vertex = submesh->first_node();

        for (int vertexId = 0; vertexId < vertexCount; ++vertexId) {
            if (!ValidateTag(vertex, "VERTEX", pCoreMesh, pCoreSubmesh)) {
                return null;
            }
            CalCoreSubmesh::Vertex Vertex;
            CalColor32 vertexColor = CalMakeColor(CalVector(1.0f, 1.0f, 1.0f));

            xml_node* pos = vertex->first_node();
            if (!ValidateTag(pos, "POS", pCoreMesh, pCoreSubmesh)) {
                return null;
            }
            ReadTripleFloat(pos->value(), &Vertex.position.x, &Vertex.position.y, &Vertex.position.z);

            xml_node* norm = pos->next_sibling();
            if (!ValidateTag(norm, "NORM", pCoreMesh, pCoreSubmesh)) {
                return null;
            }

            ReadTripleFloat(norm->value(),  &Vertex.normal.x, &Vertex.normal.y, &Vertex.normal.z);

            xml_node* vertColor = norm->next_sibling();
            xml_node* collapse = 0;
            if (!vertColor || !has_name(vertColor, "color")) {
                if (hasVertexColors) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                } else {
                    collapse = vertColor;
                }
            } else {
                CalVector vc(1.0f, 1.0f, 1.0f);
                ReadTripleFloat(vertColor->value(), &vc.x, &vc.y, &vc.z);
                vertexColor = CalMakeColor(vc);

                collapse = vertColor->next_sibling();
            }
            if (!collapse) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
            if (has_name(collapse, "COLLAPSEID")) {
                const char* collapseid = collapse->value();
                if (!collapseid) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
                xml_node* collapseCount = collapse->next_sibling();
                if (!collapseCount || !has_name(collapseCount, "COLLAPSECOUNT")) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }

                const char* collapseCountData = collapseCount->value();
                if (!collapseCountData) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
                collapse = collapseCount->next_sibling();
            }

            xml_node* texcoord = collapse;

            // load all texture coordinates of the vertex
            int textureCoordinateId;
            for (textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; ++textureCoordinateId) {
                CalCoreSubmesh::TextureCoordinate textureCoordinate;
                // load data of the influence
#if CAL3D_VALIDATE_XML_TAGS
                if (!texcoord || !has_name(texcoord, "TEXCOORD")) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
#endif

                ReadPair(texcoord->value(), &textureCoordinate.u, &textureCoordinate.v);

                // set texture coordinate in the core submesh instance
                if (textureCoordinateId == 0) {
                    pCoreSubmesh->setTextureCoordinate(vertexId, textureCoordinate);
                }
                texcoord = texcoord->next_sibling();
            }

            // get the number of influences
            int influenceCount = get_int_attribute(vertex, "NUMINFLUENCES");
            if (influenceCount < 0) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }

            std::vector<CalCoreSubmesh::Influence> influences(influenceCount);

            xml_node* influence = texcoord;

            // load all influences of the vertex
            int influenceId;
            for (influenceId = 0; influenceId < influenceCount; ++influenceId) {
#if CAL3D_VALIDATE_XML_TAGS
                if (!influence || !has_name(influence, "INFLUENCE")) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
#endif

                influences[influenceId].boneId = get_int_attribute(influence, "ID");
                influences[influenceId].weight = imvu_atof(influence->value());

                influence = influence->next_sibling();
            }

            pCoreSubmesh->addVertex(Vertex, vertexColor, influences);
            vertex = vertex->next_sibling();
        }

        xml_node* spring = vertex;

        for (int springId = 0; springId < springCount; ++springId) {
            if (!spring || !has_name(spring, "SPRING")) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
            spring = spring->next_sibling();
        }

        xml_node* face = spring;

        xml_node* morph = face;
        for (int morphId = 0; morphId < morphCount; morphId++) {
            if (!has_name(morph, "MORPH")) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }

            CalCoreMorphTarget::VertexOffsetArray vertexOffsets;

            xml_node* blendVert = morph->first_node();
            for (int blendVertI = 0; blendVertI < vertexCount; blendVertI++) {
                VertexOffset Vertex;

                bool copyOrig = true;
                if (blendVert && has_name(blendVert, "BLENDVERTEX")) {
                    int vertId = get_int_attribute(blendVert, "VERTEXID");

                    if (vertId == blendVertI) {
                        copyOrig = false;
                    }
                }

                if (!copyOrig) {
                    if (!ValidateTag(blendVert, "BLENDVERTEX", pCoreMesh, pCoreSubmesh)) {
                        return null;
                    }

                    xml_node* pos = blendVert->first_node();
                    if (!CalVectorFromXml(pos, "POSITION", &Vertex.position, pCoreMesh, pCoreSubmesh)) {
                        return null;
                    }

                    xml_node* norm = pos->next_sibling();
                    if (!CalVectorFromXml(norm, "NORMAL", &Vertex.normal, pCoreMesh, pCoreSubmesh)) {
                        return null;
                    }

                    xml_node* texcoord = norm->next_sibling();
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
                        texcoord = texcoord->next_sibling();
                    }
                    blendVert = blendVert->next_sibling();
                    Vertex.vertexId = blendVertI;
                    Vertex.position -= pCoreSubmesh->getVectorVertex()[blendVertI].position;
                    Vertex.normal -= pCoreSubmesh->getVectorVertex()[blendVertI].normal;
                    vertexOffsets.push_back(Vertex);
                }
            }

            auto nameAttribute = morph->first_attribute("name", 0, false);
            const char* name;
            if (!nameAttribute) {
                name = "";
            } else {
                name = nameAttribute->value();
            }

            CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget(name, vertexCount, vertexOffsets));
            pCoreSubmesh->addMorphTarget(morphTarget);

            morph = morph->next_sibling();
        }

        face = morph;
        // load all faces
        for (int faceId = 0; faceId < faceCount; ++faceId) {
#if CAL3D_VALIDATE_XML_TAGS
            if (!has_name(face, "FACE")) {
                CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                return null;
            }
#endif
            int tmp[3];

            // load data of the face
            std::stringstream str;
            str << get_string_attribute(face, "VERTEXID");
            str >> tmp[0] >> tmp [1] >> tmp[2];

            if (sizeof(CalIndex) == 2) {
                if (tmp[0] > 65535 || tmp[1] > 65535 || tmp[2] > 65535) {
                    CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__, strFilename);
                    return null;
                }
            }
            pCoreSubmesh->addFace(CalCoreSubmesh::Face(tmp[0], tmp[1], tmp[2]));

            face = face->next_sibling();
        }
        submesh = submesh->next_sibling();

        // add the core submesh to the core mesh instance
        pCoreMesh->submeshes.push_back(pCoreSubmesh);
    }

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
