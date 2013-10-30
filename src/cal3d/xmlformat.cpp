#include <boost/optional.hpp>
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

struct InvalidFileFormat {
    InvalidFileFormat() {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
    }

    operator CalCoreSkeletonPtr() const { return CalCoreSkeletonPtr(); }
    operator CalCoreMaterialPtr() const { return CalCoreMaterialPtr(); }
    operator CalCoreMeshPtr() const { return CalCoreMeshPtr(); }
    operator CalCoreAnimationPtr() const { return CalCoreAnimationPtr(); }
    operator CalCoreMorphAnimationPtr() const { return CalCoreMorphAnimationPtr(); }
};

static inline void ReadPair(char const* buffer, float* f1, float* f2) {
    float a = 0.0f;
    float b = 0.0f;
    sscanf(buffer, " %f %f", &a, &b);
    *f1 = a;
    *f2 = b;
}

static inline void ReadPair(char const* buffer, int* f1, int* f2) {
    int a = 0;
    int b = 0;
    sscanf(buffer, " %d %d", &a, &b);
    *f1 = a;
    *f2 = b;
}

// atof does wacky things in VC++ 2010 CRT in Serbia...
static inline float imvu_atof(const char* p) {
    float f = 0.0f;
    sscanf(p, " %f", &f);
    return f;
}


#define CAL3D_VALIDATE_XML_TAGS ( 1 )


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

static bool has_attribute(rapidxml::xml_node<>* node, const char* name) {
    auto attribute = node->first_attribute(name, 0, false);
    return !!attribute;
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

static int get_int_value(rapidxml::xml_node<>* node) {
    const char* v = node->value();
    return v ? atoi(v) : 0;
}

static const char* get_string_value(rapidxml::xml_node<>* node) {
    const char* v = node->value();
    return v ? v : "";
}

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
    float a = 0.0f;
    float b = 0.0f;
    float c = 0.0f;
    sscanf(buffer, " %f %f %f", &a, &b, &c);
    *f1 = a;
    *f2 = b;
    *f3 = c;
}

static inline void ReadTripleInt(char const* buffer, int* f1, int* f2, int* f3) {
    int a = 0;
    int b = 0;
    int c = 0;
    sscanf(buffer, " %d %d %d", &a, &b, &c);
    *f1 = a;
    *f2 = b;
    *f3 = c;
}

static inline void ReadQuadInt(char const* buffer, int* f1, int* f2, int* f3, int* f4) {
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    sscanf(buffer, " %d %d %d %d", &a, &b, &c, &d);
    *f1 = a;
    *f2 = b;
    *f3 = c;
    *f4 = d;
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
    float a = 0.0f;
    float b = 0.0f;
    float c = 0.0f;
    float d = 0.0f;
    sscanf(buffer, " %f %f %f %f", &a, &b, &c, &d);
    *f1 = a;
    *f2 = b;
    *f3 = c;
    *f4 = d;
}

CalCoreSkeletonPtr CalLoader::loadXmlCoreSkeleton(char* dataSrc) {
    rapidxml::xml_document<> document;
    try {
        document.parse<rapidxml::parse_no_data_nodes | rapidxml::parse_no_entity_translation>(dataSrc);
    } catch (const rapidxml::parse_error&) {
        return InvalidFileFormat();
    }

    return loadXmlCoreSkeletonDoc(document);
}

CalCoreMeshPtr CalLoader::loadXmlCoreMesh(char* dataSrc) {
    rapidxml::xml_document<> document;
    try {
        document.parse<rapidxml::parse_no_data_nodes | rapidxml::parse_no_entity_translation>(dataSrc);
    } catch (const rapidxml::parse_error&) {
        return InvalidFileFormat();
    }

    return loadXmlCoreMeshDoc(document);
}

CalCoreMaterialPtr CalLoader::loadXmlCoreMaterial(char* dataSrc) {
    rapidxml::xml_document<> doc;

    try {
        doc.parse<rapidxml::parse_no_data_nodes | rapidxml::parse_no_entity_translation>(dataSrc);
    } catch (const rapidxml::parse_error&) {
        return InvalidFileFormat();
    }

    return loadXmlCoreMaterialDoc(doc);
}

CalCoreAnimationPtr CalLoader::loadXmlCoreAnimation(char* dataSrc) {
    TiXmlDocument doc;

    doc.Parse(dataSrc);
    if (doc.Error()) {
        CalError::setLastError(CalError::INVALID_FILE_FORMAT, __FILE__, __LINE__);
        return CalCoreAnimationPtr();
    }

    return loadXmlCoreAnimationDoc(doc);
}

CalCoreMorphAnimationPtr CalLoader::loadXmlCoreMorphAnimation(char* dataSrc) {
    TiXmlDocument doc;

    doc.Parse(dataSrc);
    if (doc.Error()) {
        return InvalidFileFormat();
    }

    return loadXmlCoreMorphAnimationDoc(doc);
}

CalCoreSkeletonPtr CalLoader::loadXmlCoreSkeletonDoc(const rapidxml::xml_document<>& doc) {
    typedef rapidxml::xml_node<> TiXmlNode;
    typedef rapidxml::xml_node<> TiXmlElement;

    TiXmlElement* header = doc.first_node();
    if (!header || !has_name(header, "header")) {
        return InvalidFileFormat();
    }

    if (!isHeaderWellFormed(header)) {
        return InvalidFileFormat();
    }

    if (!has_attribute_value(header, "magic", cal3d::SKELETON_XMLFILE_EXTENSION)) {
        return InvalidFileFormat();
    }

    if (get_int_attribute(header, "version") < cal3d::EARLIEST_COMPATIBLE_FILE_VERSION) {
        return InvalidFileFormat();
    }

    TiXmlElement* skeleton = header->next_sibling();
    if (!skeleton || !has_name(skeleton, "skeleton")) {
        return InvalidFileFormat();
    }

    boost::optional<CalVector> sceneAmbientColor;

    char const* attrStr = get_string_attribute(skeleton, "SCENEAMBIENTCOLOR");
    if (attrStr) {
        CalVector sceneColor;
        ReadTripleFloat(attrStr, &sceneColor.x, &sceneColor.y, &sceneColor.z);
        sceneAmbientColor = sceneColor;
    }

    std::vector<CalCoreBonePtr> bones;

    for (TiXmlElement* bone = skeleton->first_node(); bone; bone = bone->next_sibling()) {
        if (!has_name(bone, "BONE")) {
            return InvalidFileFormat();
        }

        // safe because rapidxml keeps string pointers alive as long as the source text is alive
        const char* strName = get_string_attribute(bone, "NAME");

        CalLightType lightType = LIGHT_TYPE_NONE;
        CalVector lightColor;

        if (has_attribute(bone, "LIGHTTYPE")) {
            lightType = (CalLightType)get_int_attribute(bone, "LIGHTTYPE");
        }

        const char* attrStr = get_string_attribute(bone, "LIGHTCOLOR");
        if (attrStr) {
            ReadTripleFloat(attrStr, &lightColor.x, &lightColor.y, &lightColor.z);
        }
        
        // get the translation of the bone

        TiXmlElement* translation = bone->first_node();
        if (!translation || !has_name(translation, "TRANSLATION")) {
            return InvalidFileFormat();
        }

        float tx, ty, tz;
        ReadTripleFloat(get_string_value(translation), &tx, &ty, &tz);

        // get the rotation of the bone

        TiXmlElement* rotation = translation->next_sibling();
        if (!rotation || !has_name(rotation, "ROTATION")) {
            return InvalidFileFormat();
        }

        float rx, ry, rz, rw;
        ReadQuadFloat(get_string_value(rotation), &rx, &ry, &rz, &rw);

        // get the bone space translation of the bone

        TiXmlElement* translationBoneSpace = rotation->next_sibling();
        if (!translationBoneSpace || !has_name(translationBoneSpace, "LOCALTRANSLATION")) {
            return InvalidFileFormat();
        }

        float txBoneSpace, tyBoneSpace, tzBoneSpace;
        ReadTripleFloat(get_string_value(translationBoneSpace), &txBoneSpace, &tyBoneSpace, &tzBoneSpace);

        // get the bone space rotation of the bone

        TiXmlElement* rotationBoneSpace = translationBoneSpace->next_sibling();
        if (!rotationBoneSpace || !has_name(rotationBoneSpace, "LOCALROTATION")) {
            return InvalidFileFormat();
        }

        float rxBoneSpace, ryBoneSpace, rzBoneSpace, rwBoneSpace;
        ReadQuadFloat(get_string_value(rotationBoneSpace), &rxBoneSpace, &ryBoneSpace, &rzBoneSpace, &rwBoneSpace);

        // get the parent bone id

        TiXmlElement* parent = rotationBoneSpace->next_sibling();
        if (!parent || !has_name(parent, "PARENTID")) {
            return InvalidFileFormat();
        }

        int parentId = get_int_value(parent);

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
        for (child = parent->next_sibling(); child; child = child->next_sibling()) {
            if (!has_name(child, "CHILDID")) {
                return InvalidFileFormat();
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
    TiXmlNode* node;

    TiXmlElement* header = doc.FirstChildElement();
    if (!header || cal3d_stricmp(header->Value(), "HEADER") != 0) {
        return InvalidFileFormat();
    }

    if (!isHeaderWellFormed(header)) {
        return InvalidFileFormat();
    }

    if (cal3d_stricmp(header->Attribute("MAGIC"), cal3d::ANIMATION_XMLFILE_EXTENSION) != 0) {
        return InvalidFileFormat();
    }

    if (atoi(header->Attribute("VERSION")) < cal3d::EARLIEST_COMPATIBLE_FILE_VERSION) {
        return InvalidFileFormat();
    }

    TiXmlElement* animation = header->NextSiblingElement();
    if (!animation || cal3d_stricmp(animation->Value(), "ANIMATION") != 0) {
        return InvalidFileFormat();
    }

    float duration = imvu_atof(animation->Attribute("DURATION"));

    // allocate a new core animation instance
    CalCoreAnimationPtr pCoreAnimation(new CalCoreAnimation);

    // check for a valid duration
    if (duration <= 0.0f) {
        return InvalidFileFormat();
    }

    pCoreAnimation->duration = duration;
    for (
        TiXmlElement* track = animation->FirstChildElement();
        track;
        track = track->NextSiblingElement()
    ) {
        if (!track || cal3d_stricmp(track->Value(), "TRACK") != 0) {
            return InvalidFileFormat();
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
            return InvalidFileFormat();
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
                return InvalidFileFormat();
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
                return InvalidFileFormat();
            }
            if (cal3d_stricmp(translation->Value(), "TRANSLATION") == 0) {
                node = translation->FirstChild();
                if (!node) {
                    return InvalidFileFormat();
                }

                TiXmlText* translationdata = node->ToText();
                if (!translationdata) {
                    return InvalidFileFormat();
                }
                ReadTripleFloat(translationdata->Value(), &t.x, &t.y, &t.z);
                rotation = rotation->NextSiblingElement();
            }

            if (!rotation || cal3d_stricmp(rotation->Value(), "ROTATION") != 0) {
                return InvalidFileFormat();
            }

            float rx, ry, rz, rw;

            node = rotation->FirstChild();
            if (!node) {
                return InvalidFileFormat();
            }
            TiXmlText* rotationdata = node->ToText();
            if (!rotationdata) {
                return InvalidFileFormat();
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

    return pCoreAnimation;
}

CalCoreMorphAnimationPtr CalLoader::loadXmlCoreMorphAnimationDoc(TiXmlDocument& doc) {
    TiXmlElement* header = doc.FirstChildElement();
    if (!header || cal3d_stricmp(header->Value(), "HEADER") != 0) {
        return InvalidFileFormat();
    }

    if (!isHeaderWellFormed(header)) {
        return InvalidFileFormat();
    }

    if (cal3d_stricmp(header->Attribute("MAGIC"), cal3d::ANIMATEDMORPH_XMLFILE_EXTENSION) != 0) {
        return InvalidFileFormat();
    }

    if (atoi(header->Attribute("VERSION")) < cal3d::EARLIEST_COMPATIBLE_FILE_VERSION) {
        return InvalidFileFormat();
    }

    // allocate a new core animatedMorph instance
    CalCoreMorphAnimationPtr pCoreMorphAnimation(new CalCoreMorphAnimation);

    TiXmlElement* animatedMorph = header->NextSiblingElement();
    if (!animatedMorph || cal3d_stricmp(animatedMorph->Value(), "ANIMATION") != 0) {
        return InvalidFileFormat();
    }

    if (!BindFromXml(*animatedMorph, pCoreMorphAnimation.get())) {
        return InvalidFileFormat();
    }

    // check for a valid duration
    if (pCoreMorphAnimation->duration <= 0.0f) {
        return InvalidFileFormat();
    }

    return pCoreMorphAnimation;
}

CalCoreMeshPtr CalLoader::loadXmlCoreMeshDoc(const rapidxml::xml_document<>& doc) {
    typedef rapidxml::xml_node<> xml_node;

    xml_node* header = doc.first_node();
    if (!header || !has_name(header, "header")) {
        return InvalidFileFormat();
    }

    if (!isHeaderWellFormed(header)) {
        return InvalidFileFormat();
    }

    if (!has_attribute_value(header, "magic", cal3d::MESH_XMLFILE_EXTENSION)) {
        return InvalidFileFormat();
    }

    int version = get_int_attribute(header, "version");
    if (version < cal3d::EARLIEST_COMPATIBLE_FILE_VERSION) {
        return InvalidFileFormat();
    }

    bool hasVertexColors = (version >= cal3d::FIRST_FILE_VERSION_WITH_VERTEX_COLORS);

    xml_node* mesh = header->next_sibling();
    if (!mesh || !has_name(mesh, "mesh")) {
        return InvalidFileFormat();
    }

    //  get the number of submeshes
    int submeshCount = get_int_attribute(mesh, "numsubmesh");

    CalCoreMeshPtr pCoreMesh(new CalCoreMesh);

    xml_node* submesh = mesh->first_node();

    // load all core submeshes
    for (int submeshId = 0; submeshId < submeshCount; ++submeshId) {
        if (!submesh || !has_name(submesh, "submesh")) {
            return InvalidFileFormat();
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
                return InvalidFileFormat();
            }
            CalCoreSubmesh::Vertex Vertex;
            CalColor32 vertexColor = CalMakeColor(CalVector(1.0f, 1.0f, 1.0f));

            xml_node* pos = vertex->first_node();
            if (!ValidateTag(pos, "POS", pCoreMesh, pCoreSubmesh)) {
                return InvalidFileFormat();
            }
            ReadTripleFloat(pos->value(), &Vertex.position.x, &Vertex.position.y, &Vertex.position.z);

            xml_node* norm = pos->next_sibling();
            if (!ValidateTag(norm, "NORM", pCoreMesh, pCoreSubmesh)) {
                return InvalidFileFormat();
            }

            ReadTripleFloat(norm->value(),  &Vertex.normal.x, &Vertex.normal.y, &Vertex.normal.z);

            xml_node* vertColor = norm->next_sibling();
            xml_node* collapse = 0;
            if (!vertColor || !has_name(vertColor, "color")) {
                if (hasVertexColors) {
                    return InvalidFileFormat();
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
                return InvalidFileFormat();
            }
            if (has_name(collapse, "COLLAPSEID")) {
                const char* collapseid = collapse->value();
                if (!collapseid) {
                    return InvalidFileFormat();
                }
                xml_node* collapseCount = collapse->next_sibling();
                if (!collapseCount || !has_name(collapseCount, "COLLAPSECOUNT")) {
                    return InvalidFileFormat();
                }

                const char* collapseCountData = collapseCount->value();
                if (!collapseCountData) {
                    return InvalidFileFormat();
                }
                collapse = collapseCount->next_sibling();
            }

            xml_node* texcoord = collapse;

            // load all texture coordinates of the vertex
            for (int textureCoordinateId = 0; textureCoordinateId < textureCoordinateCount; ++textureCoordinateId) {
                CalCoreSubmesh::TextureCoordinate textureCoordinate;
                // load data of the influence
#if CAL3D_VALIDATE_XML_TAGS
                if (!texcoord || !has_name(texcoord, "TEXCOORD")) {
                    return InvalidFileFormat();
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
                return InvalidFileFormat();
            }

            std::vector<CalCoreSubmesh::Influence> influences(influenceCount);

            xml_node* influence = texcoord;

            // load all influences of the vertex
            int influenceId;
            for (influenceId = 0; influenceId < influenceCount; ++influenceId) {
#if CAL3D_VALIDATE_XML_TAGS
                if (!influence || !has_name(influence, "INFLUENCE")) {
                    return InvalidFileFormat();
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
                return InvalidFileFormat();
            }
            spring = spring->next_sibling();
        }

        xml_node* face = spring;

        xml_node* morph = face;
        for (int morphId = 0; morphId < morphCount; morphId++) {
            if (!has_name(morph, "MORPH")) {
                return InvalidFileFormat();
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
                        return InvalidFileFormat();
                    }

                    xml_node* pos = blendVert->first_node();
                    if (!CalVectorFromXml(pos, "POSITION", &Vertex.position, pCoreMesh, pCoreSubmesh)) {
                        return InvalidFileFormat();
                    }

                    xml_node* norm = pos->next_sibling();
                    if (!CalVectorFromXml(norm, "NORMAL", &Vertex.normal, pCoreMesh, pCoreSubmesh)) {
                        return InvalidFileFormat();
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
                            return InvalidFileFormat();
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
                return InvalidFileFormat();
            }
#endif
            int tmp[3];
            ReadTripleInt(get_string_attribute(face, "VERTEXID"), tmp, tmp + 1, tmp + 2);

            if (sizeof(CalIndex) == 2) {
                if (tmp[0] > 65535 || tmp[1] > 65535 || tmp[2] > 65535) {
                    return InvalidFileFormat();
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


CalCoreMaterialPtr CalLoader::loadXmlCoreMaterialDoc(const rapidxml::xml_document<>& doc) {
    typedef rapidxml::xml_node<> TiXmlNode;
    typedef rapidxml::xml_node<> TiXmlElement;

    TiXmlElement* header = doc.first_node();
    if (!header || !has_name(header, "HEADER")) {
        return InvalidFileFormat();
    }

    if (!isHeaderWellFormed(header)) {
        return InvalidFileFormat();
    }

    if (!has_attribute_value(header, "MAGIC", cal3d::MATERIAL_XMLFILE_EXTENSION)) {
        return InvalidFileFormat();
    }

    if (get_int_attribute(header, "VERSION") < cal3d::EARLIEST_COMPATIBLE_FILE_VERSION) {
        return InvalidFileFormat();
    }

    TiXmlElement* material = header->next_sibling();
    if (!material || !has_name(material, "MATERIAL")) {
        return InvalidFileFormat();
    }

    CalCoreMaterialPtr pCoreMaterial(new CalCoreMaterial);

    TiXmlElement* ambient = material->first_node();
    if (!ambient || !has_name(ambient, "AMBIENT")) {
        return InvalidFileFormat();
    }

    int r, g, b, a; // never used
    ReadQuadInt(get_string_value(ambient), &r, &g, &b, &a);

    TiXmlElement* diffuse = ambient->next_sibling();
    if (!diffuse || !has_name(diffuse, "DIFFUSE")) {
        return InvalidFileFormat();
    }

    ReadQuadInt(get_string_value(diffuse), &r, &g, &b, &a);

    TiXmlElement* specular = diffuse->next_sibling();
    if (!specular || !has_name(specular, "SPECULAR")) {
        return InvalidFileFormat();
    }

    ReadQuadInt(get_string_value(specular), &r, &g, &b, &a);

    TiXmlElement* shininess = specular->next_sibling();
    if (!shininess || !has_name(shininess, "SHININESS")) {
        return InvalidFileFormat();
    }

    std::vector<std::string> MatFileName;
    std::vector<std::string> MatTypes;

    for (TiXmlElement* map = shininess->next_sibling(); map; map = map->next_sibling()) {
        if (!map || !has_name(map, "MAP")) {
            return InvalidFileFormat();
        }
        
        MatFileName.push_back(get_string_value(map));

        std::string mapType = "Diffuse Color";
        if (auto attr = map->first_attribute("TYPE", 0, false)) {
            mapType = get_string_attribute(map, "TYPE");
        }
        MatTypes.push_back(mapType);
    }

    for (unsigned int mapId = 0; mapId < MatFileName.size(); ++mapId) {
        CalCoreMaterial::Map Map;

        Map.filename = MatFileName[mapId];
        Map.type = MatTypes[mapId];

        pCoreMaterial->maps.push_back(Map);
    }

    return pCoreMaterial;
}
