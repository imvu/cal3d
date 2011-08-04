#include "TestPrologue.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <cal3d/buffersource.h>
#include <cal3d/coreanimation.h>
#include <cal3d/corekeyframe.h>
#include <cal3d/coremesh.h>
#include <cal3d/coresubmesh.h>
#include <cal3d/coretrack.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/corebone.h>
#include <cal3d/loader.h>
#include <cal3d/saver.h>
#include <cal3d/streamops.h>

inline int getIntFromBuf(char* pbuf) {
    return *reinterpret_cast<int*>(pbuf);
}
inline float* getFloatVecFromBuf(char* pbuf) {
    return reinterpret_cast<float*>(pbuf);
}


const char goodMeshData[] =
    "<HEADER MAGIC=\"XMF\" VERSION=\"919\" />"
    "<MESH NUMSUBMESH=\"1\">"
    "    <SUBMESH NUMVERTICES=\"3\" NUMFACES=\"1\" NUMLODSTEPS=\"0\" NUMSPRINGS=\"0\" NUMMORPHS=\"0\" NUMTEXCOORDS=\"1\" MATERIAL=\"1\">"
    "        <VERTEX NUMINFLUENCES=\"3\" ID=\"0\">"
    "            <POS>5759.05 -1176.88 -0.00023478</POS>"
    "            <NORM>1.27676e-008 2.40249e-008 -1</NORM>"
    "            <COLOR>0 0 0</COLOR>"
    "            <TEXCOORD>0.99311 0.00973237</TEXCOORD>"
    "            <INFLUENCE ID=\"0\">1</INFLUENCE>"
    "            <INFLUENCE ID=\"1\">0.5</INFLUENCE>"
    "            <INFLUENCE ID=\"37\">0.25</INFLUENCE>"
    "        </VERTEX>"
    "        <VERTEX NUMINFLUENCES=\"0\" ID=\"1\">"
    "            <POS>-5759.05 -1176.88 -0.000413365</POS>"
    "            <NORM>1.55047e-008 -2.86491e-008 -1</NORM>"
    "            <COLOR>0 0 0</COLOR>"
    "            <TEXCOORD>0.99311 0.982444</TEXCOORD>"
    "        </VERTEX>"
    "        <VERTEX NUMINFLUENCES=\"0\" ID=\"2\">"
    "            <POS>-5759.05 -3274.86 -0.000507484</POS>"
    "            <NORM>1.11221e-008 6.89228e-008 -1</NORM>"
    "            <COLOR>0 0 0</COLOR>"
    "            <TEXCOORD>0.79062 0.982444</TEXCOORD>"
    "        </VERTEX>"
    "        <FACE VERTEXID=\"0 1 2\" />"
    "    </SUBMESH>"
    "</MESH>"
    ;

template<unsigned Length>
CalBufferSource fromString(const char(&p)[Length]) {
    return CalBufferSource(p, Length);
}

template<unsigned Length>
CalBufferSource fromString(const unsigned char(&p)[Length]) {
    return CalBufferSource(p, Length);
}

TEST(loads_mesh_which_causes_vector_Xlen_exception_and_not_crash) {
    CalBufferSource cbs(fromString(goodMeshData));
    CalCoreMeshPtr goodMesh(CalLoader::loadCoreMesh(cbs));
    CalCoreSubmeshPtr subMesh(goodMesh->getCoreSubmesh(0));
    std::ostringstream ssm;
    bool bRet = true;
    bRet = CalSaver::saveCoreMesh(ssm, goodMesh.get());
    CHECK_EQUAL(bRet, true);
    char* pBuf = new char[ssm.str().length()];
    memcpy(pBuf, ssm.str().c_str(), ssm.str().length());

    char* pCurPosInBuf = pBuf;
    float* tempFloatVec = NULL;
    int tempInt = 0;
    pCurPosInBuf = pBuf + sizeof(int); //magic num
    int version = getIntFromBuf(pCurPosInBuf);
    pCurPosInBuf += sizeof(int); //version
    tempInt = getIntFromBuf(pCurPosInBuf);
    pCurPosInBuf += sizeof(int); //submesh count
    bool hasVertexColors = (version >= Cal::FIRST_FILE_VERSION_WITH_VERTEX_COLORS);
    bool hasMorphTargetsInMorphFiles = (version >= Cal::FIRST_FILE_VERSION_WITH_MORPH_TARGETS_IN_MORPH_FILES);
    tempInt =  getIntFromBuf(pCurPosInBuf);
    pCurPosInBuf += sizeof(int); //coreMaterialThreadId
    int vertexCount = getIntFromBuf(pCurPosInBuf);
    pCurPosInBuf += sizeof(int);//vertexCount
    tempInt = getIntFromBuf(pCurPosInBuf);
    pCurPosInBuf += sizeof(int);//faceCount
    tempInt = getIntFromBuf(pCurPosInBuf);
    pCurPosInBuf += sizeof(int);//lodCount
    tempInt = getIntFromBuf(pCurPosInBuf);
    pCurPosInBuf += sizeof(int); //springCount
    int texCoordinateCount = getIntFromBuf(pCurPosInBuf);
    pCurPosInBuf += sizeof(int);//textureCoordinateCount
    if (hasMorphTargetsInMorphFiles) {
        tempInt = getIntFromBuf(pCurPosInBuf);
        pCurPosInBuf += sizeof(int);
    }
    if (vertexCount > 0) {
        tempFloatVec = getFloatVecFromBuf(pCurPosInBuf);
        pCurPosInBuf += sizeof(float) * 3;//pos
        tempFloatVec = getFloatVecFromBuf(pCurPosInBuf);
        pCurPosInBuf += sizeof(float) * 3;//normal
        if (hasVertexColors) {
            tempFloatVec = getFloatVecFromBuf(pCurPosInBuf);
            pCurPosInBuf += sizeof(float) * 3;
        }
        tempInt = getIntFromBuf(pCurPosInBuf);
        pCurPosInBuf += sizeof(int); //collapseId
        tempInt = getIntFromBuf(pCurPosInBuf);
        pCurPosInBuf += sizeof(int); //faceCollapseCount
        size_t texCoordOffset = texCoordinateCount * sizeof(float) * 2;
        tempFloatVec = getFloatVecFromBuf(pCurPosInBuf);
        pCurPosInBuf += texCoordOffset;
        tempInt = getIntFromBuf(pCurPosInBuf);
        //put in a value which is sure to cause an exception due to vector allocation
        int tempNewInfluenceCount =  std::numeric_limits<int>::max();
        *reinterpret_cast<int*>(pCurPosInBuf) = tempNewInfluenceCount;
    }
    //load the buffer back in
    //should not crash
    CalBufferSource cbs2(pBuf, ssm.str().length());
    shared_ptr<CalCoreMesh> badMesh(CalLoader::loadCoreMesh(cbs2));
    CalCoreMesh* pMesh = badMesh.get();

    CHECK(!pMesh);
    delete [] pBuf;
}

const char animationText[] =
    "<HEADER MAGIC=\"XAF\" VERSION=\"919\" />\n"
    "<ANIMATION NUMTRACKS=\"1\" DURATION=\"40\">\n"
    "    <TRACK BONEID=\"0\" TRANSLATIONREQUIRED=\"0\" TRANSLATIONISDYNAMIC=\"0\" HIGHRANGEREQUIRED=\"1\" NUMKEYFRAMES=\"2\">\n"
    "        <KEYFRAME TIME=\"0\">\n"
    "            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "        <KEYFRAME TIME=\"40\">\n"
    "            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "    </TRACK>\n"
    "</ANIMATION>\n"
    ;

TEST(LoadSimpleXmlAnimation) {
    CalBufferSource cbs(fromString(animationText));
    CalCoreAnimationPtr anim = CalLoader::loadCoreAnimation(cbs, 0);
    CHECK(anim);
    CHECK_EQUAL(anim->tracks.size(), 1u);
    CHECK_EQUAL(anim->duration, 40);

    const CalCoreTrack* track1 = &anim->tracks[0];
    const CalCoreTrack* track2 = anim->getCoreTrack(/*boneid*/ 0);
    CHECK(track1);
    CHECK_EQUAL(track1, track2);

    CHECK_EQUAL(track1->translationRequired, false);
    CHECK_EQUAL(track1->translationIsDynamic, false);
    CHECK_EQUAL(track1->keyframes.size(), 2u);

    std::ostringstream ss;
    CalSaver::saveXmlCoreAnimation(ss, anim.get());

    CHECK_EQUAL(ss.str(), animationText);
}

const char animation_with_out_of_order_keyframes[] =
    "<HEADER MAGIC=\"XAF\" VERSION=\"919\" />\n"
    "<ANIMATION NUMTRACKS=\"1\" DURATION=\"40\">\n"
    "    <TRACK BONEID=\"0\" TRANSLATIONREQUIRED=\"0\" TRANSLATIONISDYNAMIC=\"0\" HIGHRANGEREQUIRED=\"1\" NUMKEYFRAMES=\"2\">\n"
    "        <KEYFRAME TIME=\"40\">\n"
    "            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "        <KEYFRAME TIME=\"0\">\n"
    "            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "    </TRACK>\n"
    "</ANIMATION>\n"
    ;

TEST(sorts_keyframes_upon_load) {
    CalBufferSource cbs(fromString(animation_with_out_of_order_keyframes));
    CalCoreAnimationPtr anim = CalLoader::loadCoreAnimation(cbs, 0);
    CHECK(anim);

    CHECK_EQUAL(anim->tracks.size(), 1u);
    CHECK_EQUAL(anim->duration, 40);

    CalCoreTrack* track = &anim->tracks[0];
    CHECK_EQUAL(track->keyframes.size(), 2u);

    const CalCoreKeyframe& k1 = track->keyframes[0];
    CHECK_EQUAL(0.0f, k1.time);

    const CalCoreKeyframe& k2 = track->keyframes[1];
    CHECK_EQUAL(40.0f, k2.time);
}

const char animation_with_translations[] =
    "<HEADER MAGIC=\"XAF\" VERSION=\"919\" />\n"
    "<ANIMATION NUMTRACKS=\"1\" DURATION=\"40\">\n"
    "    <TRACK BONEID=\"0\" TRANSLATIONREQUIRED=\"1\" TRANSLATIONISDYNAMIC=\"0\" HIGHRANGEREQUIRED=\"1\" NUMKEYFRAMES=\"2\">\n"
    "        <KEYFRAME TIME=\"0\">\n"
    "            <TRANSLATION>1.0 2.0 3.0</TRANSLATION>\n"
    "            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "        <KEYFRAME TIME=\"40\">\n"
    "            <TRANSLATION>4.0 5.0 6.0</TRANSLATION>\n"
    "            <ROTATION>0.5 0.5 0.5 0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "    </TRACK>\n"
    "</ANIMATION>\n"
    ;

TEST(load_animation_with_translation) {
    CalBufferSource cbs(fromString(animation_with_translations));
    CalCoreAnimationPtr anim = CalLoader::loadCoreAnimation(cbs, 0);
    CHECK(anim);

    CHECK_EQUAL(anim->tracks.size(), 1u);
    CHECK_EQUAL(anim->duration, 40);

    CalCoreTrack* track = &anim->tracks[0];
    CHECK_EQUAL(track->keyframes.size(), 2u);

    const CalCoreKeyframe& k1 = track->keyframes[0];
    CHECK_EQUAL(0.0f, k1.time);
    CHECK_EQUAL(CalQuaternion(0.5, 0.5, 0.5, -0.5), k1.rotation);
    CHECK_EQUAL(CalVector(1, 2, 3), k1.translation);

    const CalCoreKeyframe& k2 = track->keyframes[1];
    CHECK_EQUAL(40.0f, k2.time);
    CHECK_EQUAL(CalQuaternion(0.5, 0.5, 0.5, 0.5), k2.rotation);
    CHECK_EQUAL(CalVector(4, 5, 6), k2.translation);
}

const char animation_with_static_translations[] =
    "<HEADER MAGIC=\"XAF\" VERSION=\"919\" />\n"
    "<ANIMATION NUMTRACKS=\"1\" DURATION=\"40\">\n"
    "    <TRACK BONEID=\"0\" TRANSLATIONREQUIRED=\"1\" TRANSLATIONISDYNAMIC=\"0\" HIGHRANGEREQUIRED=\"1\" NUMKEYFRAMES=\"2\">\n"
    "        <KEYFRAME TIME=\"0\">\n"
    "            <TRANSLATION>1.0 2.0 3.0</TRANSLATION>\n"
    "            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "        <KEYFRAME TIME=\"40\">\n"
    "            <ROTATION>0.5 0.5 0.5 0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "    </TRACK>\n"
    "</ANIMATION>\n"
    ;

TEST(load_animation_with_static_translations) {
    CalBufferSource cbs(fromString(animation_with_static_translations));
    CalCoreAnimationPtr anim = CalLoader::loadCoreAnimation(cbs, 0);
    CHECK(anim);

    CHECK_EQUAL(anim->tracks.size(), 1u);
    CHECK_EQUAL(anim->duration, 40);

    CalCoreTrack* track = &anim->tracks[0];
    CHECK_EQUAL(track->keyframes.size(), 2u);

    const CalCoreKeyframe& k1 = track->keyframes[0];
    CHECK_EQUAL(0.0f, k1.time);
    CHECK_EQUAL(CalQuaternion(0.5, 0.5, 0.5, -0.5), k1.rotation);
    CHECK_EQUAL(CalVector(1, 2, 3), k1.translation);

    const CalCoreKeyframe& k2 = track->keyframes[1];
    CHECK_EQUAL(40.0f, k2.time);
    CHECK_EQUAL(CalQuaternion(0.5, 0.5, 0.5, 0.5), k2.rotation);
    CHECK_EQUAL(CalVector(1, 2, 3), k2.translation);
}

const char animation_with_mismatched_track_and_keyframe_count[] =
    "<HEADER MAGIC=\"XAF\" VERSION=\"919\" />\n"
    "<ANIMATION NUMTRACKS=\"2\" DURATION=\"40\">\n"
    "    <TRACK BONEID=\"0\" TRANSLATIONREQUIRED=\"1\" TRANSLATIONISDYNAMIC=\"0\" HIGHRANGEREQUIRED=\"1\" NUMKEYFRAMES=\"3\">\n"
    "        <KEYFRAME TIME=\"0\">\n"
    "            <TRANSLATION>1.0 2.0 3.0</TRANSLATION>\n"
    "            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "        <KEYFRAME TIME=\"40\">\n"
    "            <ROTATION>0.5 0.5 0.5 0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "    </TRACK>\n"
    "</ANIMATION>\n"
    ;

TEST(load_animation_with_mismatched_counts) {
    CalBufferSource cbs(fromString(animation_with_mismatched_track_and_keyframe_count));
    CalCoreAnimationPtr anim = CalLoader::loadCoreAnimation(cbs, 0);
    CHECK(anim);

    CHECK_EQUAL(anim->tracks.size(), 1u);
    CHECK_EQUAL(anim->duration, 40);

    CalCoreTrack* track = &anim->tracks[0];
    CHECK_EQUAL(track->keyframes.size(), 2u);

    const CalCoreKeyframe& k1 = track->keyframes[0];
    CHECK_EQUAL(0.0f, k1.time);
    CHECK_EQUAL(CalQuaternion(0.5, 0.5, 0.5, -0.5), k1.rotation);
    CHECK_EQUAL(CalVector(1, 2, 3), k1.translation);

    const CalCoreKeyframe& k2 = track->keyframes[1];
    CHECK_EQUAL(40.0f, k2.time);
    CHECK_EQUAL(CalQuaternion(0.5, 0.5, 0.5, 0.5), k2.rotation);
    CHECK_EQUAL(CalVector(1, 2, 3), k2.translation);
}


const char simple_two_bone_skeleton[] =
    "<HEADER MAGIC=\"XSF\" VERSION=\"919\" />"
    "<SKELETON NUMBONES=\"2\" SCENEAMBIENTCOLOR=\"0.5 0.5 0.5\">"
    "    <BONE NAME=\"AttachmentRoot\" NUMCHILDS=\"1\" ID=\"0\">"
    "        <TRANSLATION>0.346893 -12.6875 772.958</TRANSLATION>"
    "        <ROTATION>0.706778 -0.0217732 0.706766 0.0217552</ROTATION>"
    "        <LOCALTRANSLATION>-772.275 34.8962 -0.333774</LOCALTRANSLATION>"
    "        <LOCALROTATION>0.706778 -0.0217732 0.706766 -0.0217552</LOCALROTATION>"
    "        <PARENTID>-1</PARENTID>"
    "        <CHILDID>1</CHILDID>"
    "    </BONE>"
    "    <BONE NAME=\"AttachmentNode\" NUMCHILDS=\"0\" ID=\"1\">"
    "        <TRANSLATION>0.0930318 1.64391e-006 -4.76993e-008</TRANSLATION>"
    "        <ROTATION>-4.40272e-008 -1.67311e-007 4.27578e-008 1</ROTATION>"
    "        <LOCALTRANSLATION>-772.368 34.8961 -0.334036</LOCALTRANSLATION>"
    "        <LOCALROTATION>0.706778 -0.0217731 0.706766 -0.0217552</LOCALROTATION>"
    "        <PARENTID>0</PARENTID>"
    "    </BONE>"
    "</SKELETON>"
    ;

TEST(simple_two_bone_skeleton) {
    float tol = 0.001f;
    CalBufferSource cbs(fromString(simple_two_bone_skeleton));
    boost::shared_ptr<CalCoreSkeleton> skel(CalLoader::loadCoreSkeleton(cbs));
    CHECK(skel);
    CHECK_EQUAL(skel->coreBones.size(), 2u);
    CalVector sceneAmbientClr = skel->sceneAmbientColor;
    CHECK_EQUAL(CalVector(0.5f, 0.5f, 0.5f), sceneAmbientClr);
    CalCoreBone* rootBone = skel->coreBones[0].get();
    CHECK(rootBone);
    const std::string rootBoneName = rootBone->name;
    CHECK_EQUAL(rootBoneName.c_str(), "AttachmentRoot");
    const std::vector<int>& childrenOfRootBone = rootBone->childIds;
    CHECK_EQUAL(childrenOfRootBone.size(), 1u);
    int parentIdOfRootBone = rootBone->parentId;
    CHECK_EQUAL(parentIdOfRootBone, -1);
    const CalQuaternion& rot = rootBone->relativeTransform.rotation;
    CHECK_CLOSE(rot.x, 0.706778, tol);
    CHECK_CLOSE(rot.y, -0.0217732, tol);
    CHECK_CLOSE(rot.z, 0.706766, tol);
    CHECK_CLOSE(rot.w, 0.0217552, tol);
    const CalQuaternion& boneSpaceRot = rootBone->boneSpaceTransform.rotation;
    CHECK_CLOSE(boneSpaceRot.x, 0.706778, tol);
    CHECK_CLOSE(boneSpaceRot.y, -0.0217732, tol);
    CHECK_CLOSE(boneSpaceRot.z, 0.706766, tol);
    CHECK_CLOSE(boneSpaceRot.w, -0.0217552, tol);
    const CalVector& trans = rootBone->relativeTransform.translation;
    CHECK_CLOSE(trans.x, 0.346893, tol);
    CHECK_CLOSE(trans.y, -12.6875, tol);
    CHECK_CLOSE(trans.z, 772.958, tol);
    const CalVector& boneSpaceTrans = rootBone->boneSpaceTransform.translation;
    CHECK_CLOSE(boneSpaceTrans.x, -772.275, tol);
    CHECK_CLOSE(boneSpaceTrans.y, 34.8962, tol);
    CHECK_CLOSE(boneSpaceTrans.z, -0.333774, tol);
    CalCoreBone* child = skel->coreBones[childrenOfRootBone[0]].get();
    int parentIdOfChildBone = child->parentId;
    CHECK_EQUAL(parentIdOfChildBone, 0);
}

const char one_bone_skeleton[] =
    "<HEADER MAGIC=\"XSF\" VERSION=\"919\" />"
    "<SKELETON NUMBONES=\"2\" SCENEAMBIENTCOLOR=\"1 1 1\">"
    "    <BONE NAME=\"AttachmentRoot\" NUMCHILDS=\"1\" ID=\"0\">"
    "        <TRANSLATION>0.346893 -12.6875 772.958</TRANSLATION>"
    "        <ROTATION>0.706778 -0.0217732 0.706766 0.0217552</ROTATION>"
    "        <LOCALTRANSLATION>-772.275 34.8962 -0.333774</LOCALTRANSLATION>"
    "        <LOCALROTATION>0.706778 -0.0217732 0.706766 -0.0217552</LOCALROTATION>"
    "        <PARENTID>-1</PARENTID>"
    "    </BONE>"
    "</SKELETON>"
    ;


const char animation_for_a_nonexistent_bone[] =
    "<HEADER MAGIC=\"XAF\" VERSION=\"919\" />\n"
    "<ANIMATION NUMTRACKS=\"1\" DURATION=\"40\">\n"
    "    <TRACK BONEID=\"2\" TRANSLATIONREQUIRED=\"0\" TRANSLATIONISDYNAMIC=\"0\" HIGHRANGEREQUIRED=\"1\" NUMKEYFRAMES=\"2\">\n"
    "        <KEYFRAME TIME=\"0\">\n"
    "            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "        <KEYFRAME TIME=\"40\">\n"
    "            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>\n"
    "        </KEYFRAME>\n"
    "    </TRACK>\n"
    "</ANIMATION>\n"
    ;

TEST(load_animation_for_non_existent_bone) {
    CalBufferSource cbs(fromString(one_bone_skeleton));
    boost::shared_ptr<CalCoreSkeleton> skel(CalLoader::loadCoreSkeleton(cbs));
    CHECK(skel);

    CalBufferSource cbs2(fromString(animation_for_a_nonexistent_bone));
    CalCoreAnimationPtr anim = CalLoader::loadCoreAnimation(cbs2, skel.get());
    CHECK(anim);
    CHECK_EQUAL(anim->tracks.size(), 0);
}


const size_t hmmmLength = 4730;
unsigned char hmmmAnimation[] = {
    67, 65, 70, 0, 150, 3, 0, 0, 185, 170, 106, 64, 70, 0, 0, 0, 17, 0, 9, 0, 3, 0, 0, 232, 8, 0, 19, 128, 0, 224, 72, 0, 139, 0, 2, 224, 200, 0, 235, 1, 8, 224, 200, 1, 83, 130, 9, 224, 72, 2, 99, 130, 9, 224, 200, 2, 83, 130, 9, 224, 200, 26, 59, 2, 9, 224, 72, 27, 11, 2, 8, 224, 136, 27, 18, 0, 5, 0, 3, 0, 0, 252, 5, 0, 15, 64, 1, 220, 5, 1, 23, 192, 3, 164, 5, 2, 23, 192, 6, 148,
    5, 4, 15, 64, 6, 156, 133, 27, 19, 0, 14, 0, 3, 0, 0, 132, 11, 0, 15, 192, 0, 116, 75, 0, 39, 64, 4, 20, 203, 0, 151, 192, 16, 212, 201, 1, 207, 192, 23, 108, 73, 2, 247, 64, 29, 60, 201, 2, 255, 192, 34, 60, 201, 3, 231, 192, 37, 92, 201, 4, 215, 192, 38, 140, 201, 6, 223, 64, 37, 164, 201, 10, 247, 192, 32, 164, 201, 14, 247, 64, 32, 156, 201, 26, 231, 192, 32, 132, 73, 27, 183, 64, 32, 116, 137, 27, 20, 0, 16, 0,
    3, 0, 0, 252, 2, 0, 15, 192, 0, 236, 66, 0, 31, 192, 1, 188, 130, 0, 103, 192, 4, 28, 2, 1, 191, 64, 9, 92, 129, 1, 39, 65, 14, 204, 0, 2, 119, 193, 18, 116, 128, 2, 175, 65, 22, 92, 0, 3, 199, 193, 26, 108, 0, 4, 183, 193, 29, 140, 0, 5, 183, 193, 30, 196, 0, 7, 183, 193, 28, 212, 0, 11, 175, 65, 24, 212, 0, 15, 175, 193, 23, 220, 0, 26, 175, 65, 24, 196, 0, 27, 119, 65, 25, 164, 128, 27, 21, 0, 14, 0,
    3, 0, 0, 100, 5, 0, 15, 0, 0, 92, 69, 0, 71, 64, 2, 228, 196, 0, 23, 65, 8, 124, 195, 1, 127, 193, 11, 4, 67, 2, 215, 193, 14, 204, 194, 2, 7, 66, 17, 204, 66, 3, 23, 66, 21, 228, 66, 4, 23, 194, 23, 36, 67, 6, 15, 194, 22, 68, 67, 10, 255, 193, 18, 68, 67, 14, 247, 65, 17, 76, 67, 26, 239, 193, 18, 28, 67, 27, 199, 193, 19, 20, 131, 27, 22, 0, 18, 0, 3, 0, 0, 152, 15, 0, 23, 0, 0, 168, 79, 0,
    63, 64, 1, 224, 143, 0, 119, 192, 1, 48, 208, 0, 31, 65, 4, 8, 81, 1, 223, 193, 6, 208, 209, 1, 151, 66, 9, 88, 82, 2, 39, 67, 11, 152, 210, 2, 111, 67, 13, 160, 82, 3, 135, 67, 15, 152, 210, 3, 119, 195, 18, 112, 210, 4, 111, 195, 20, 48, 210, 6, 103, 195, 18, 16, 210, 10, 87, 195, 14, 16, 210, 14, 87, 67, 14, 24, 210, 26, 87, 67, 14, 40, 18, 27, 55, 67, 15, 64, 82, 27, 231, 194, 16, 80, 146, 27, 23, 0, 2, 0,
    3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 128, 27, 24, 0, 6, 0, 89, 25, 197, 233, 32, 0, 65, 25, 198, 217, 160, 0, 137, 24, 209, 89, 32, 2, 97, 24, 211, 57, 160, 2, 97, 24, 211, 57, 32, 27, 121, 152, 209, 73, 160, 27, 25, 0, 39, 0, 163, 98, 0, 98, 22, 0, 179, 225, 14, 98, 85, 0, 227, 223, 38, 186, 147, 0, 3, 222, 55, 114, 210, 0, 123, 220, 59, 10, 18, 1, 139, 91, 47, 186, 82, 1, 139, 91, 14, 186, 148, 1, 155, 220,
    216, 33, 216, 1, 123, 222, 147, 113, 28, 2, 115, 96, 75, 185, 96, 2, 178, 99, 221, 144, 161, 2, 186, 102, 113, 224, 218, 2, 18, 104, 68, 24, 24, 3, 82, 232, 67, 248, 87, 3, 130, 232, 65, 192, 215, 3, 130, 104, 64, 152, 87, 4, 98, 232, 63, 152, 215, 4, 82, 232, 66, 192, 87, 5, 74, 104, 71, 24, 216, 5, 58, 104, 90, 104, 89, 7, 50, 104, 95, 192, 217, 7, 42, 104, 98, 240, 89, 8, 42, 104, 99, 248, 217, 8, 26, 232, 96, 208, 217, 9,
    234, 231, 86, 48, 217, 11, 202, 103, 83, 240, 216, 12, 194, 231, 82, 232, 88, 13, 178, 231, 82, 240, 216, 13, 170, 103, 84, 16, 89, 14, 154, 103, 90, 128, 89, 15, 146, 103, 95, 216, 89, 16, 138, 231, 97, 0, 90, 17, 146, 231, 97, 8, 90, 23, 138, 103, 99, 32, 90, 25, 146, 231, 97, 8, 154, 26, 162, 103, 96, 232, 217, 26, 186, 103, 93, 184, 25, 27, 162, 231, 89, 120, 89, 27, 194, 230, 104, 24, 154, 27, 26, 0, 7, 0, 127, 0, 23, 140, 0, 0,
    87, 128, 22, 132, 64, 0, 235, 0, 22, 124, 64, 1, 83, 129, 21, 116, 192, 1, 123, 129, 21, 116, 64, 2, 27, 1, 22, 124, 0, 3, 43, 1, 22, 124, 128, 27, 27, 64, 24, 0, 55, 2, 6, 96, 16, 0, 95, 2, 8, 216, 84, 0, 167, 2, 12, 32, 157, 0, 231, 130, 15, 80, 229, 0, 15, 3, 19, 88, 44, 1, 38, 131, 21, 208, 103, 1, 46, 131, 23, 24, 162, 1, 46, 131, 24, 128, 221, 1, 46, 3, 25, 168, 26, 2, 46, 131, 25, 168, 89, 2,
    46, 3, 25, 112, 155, 2, 46, 3, 24, 88, 224, 2, 46, 3, 23, 232, 34, 3, 46, 3, 23, 144, 162, 3, 46, 3, 23, 104, 34, 4, 46, 3, 23, 64, 34, 5, 46, 131, 23, 160, 33, 7, 46, 131, 23, 120, 33, 9, 46, 131, 23, 152, 33, 17, 46, 131, 23, 136, 33, 25, 46, 131, 23, 152, 161, 26, 46, 131, 23, 208, 33, 27, 46, 3, 23, 48, 98, 27, 46, 131, 23, 232, 161, 27, 28, 0, 2, 0, 79, 128, 0, 240, 0, 0, 79, 128, 0, 240, 128, 27,
    29, 0, 25, 0, 203, 67, 26, 196, 2, 0, 203, 67, 26, 196, 66, 2, 187, 200, 83, 108, 131, 2, 35, 85, 209, 236, 193, 2, 219, 218, 35, 113, 0, 3, 11, 219, 40, 125, 128, 3, 59, 91, 44, 77, 1, 4, 91, 219, 46, 245, 129, 4, 115, 91, 48, 85, 2, 5, 131, 219, 48, 117, 130, 5, 139, 219, 48, 93, 2, 6, 107, 91, 49, 245, 0, 8, 99, 219, 48, 213, 128, 8, 107, 219, 46, 5, 129, 9, 123, 91, 37, 205, 129, 11, 123, 219, 31, 13, 130, 12,
    107, 219, 27, 5, 130, 13, 99, 91, 26, 213, 1, 14, 43, 219, 24, 125, 0, 16, 27, 219, 24, 37, 0, 17, 19, 219, 24, 1, 0, 18, 27, 219, 26, 29, 128, 26, 19, 219, 30, 21, 0, 27, 3, 219, 35, 41, 64, 27, 171, 218, 36, 69, 128, 27, 30, 0, 2, 0, 11, 64, 18, 8, 2, 0, 11, 64, 18, 8, 130, 27, 31, 0, 35, 0, 175, 72, 3, 109, 6, 0, 175, 200, 2, 61, 70, 0, 167, 200, 1, 205, 133, 0, 159, 72, 0, 37, 197, 0, 127, 200,
    251, 196, 130, 1, 119, 200, 250, 28, 194, 1, 111, 72, 250, 164, 1, 2, 111, 72, 250, 124, 65, 2, 175, 200, 34, 53, 130, 2, 87, 201, 107, 133, 195, 2, 207, 73, 145, 53, 4, 3, 199, 73, 143, 61, 68, 3, 191, 201, 137, 77, 132, 3, 127, 201, 102, 197, 132, 4, 111, 73, 97, 213, 196, 4, 111, 201, 94, 221, 4, 5, 111, 201, 97, 213, 132, 5, 127, 201, 104, 189, 4, 6, 183, 73, 135, 85, 132, 7, 199, 201, 142, 61, 4, 8, 207, 73, 145, 53, 132, 8,
    199, 201, 143, 53, 4, 9, 199, 73, 140, 69, 132, 9, 175, 201, 127, 109, 132, 10, 143, 201, 112, 165, 132, 11, 119, 73, 100, 205, 132, 12, 111, 73, 96, 213, 4, 13, 111, 201, 94, 221, 132, 13, 111, 73, 96, 213, 4, 14, 119, 73, 100, 205, 132, 14, 143, 201, 112, 165, 132, 15, 175, 201, 127, 109, 132, 16, 199, 73, 140, 69, 132, 17, 199, 201, 143, 53, 4, 18, 207, 73, 145, 45, 132, 27, 32, 0, 25, 0, 119, 193, 37, 9, 3, 0, 119, 65, 40, 17, 67, 2,
    135, 65, 65, 65, 131, 2, 159, 65, 111, 153, 195, 2, 167, 193, 135, 201, 3, 3, 167, 65, 134, 201, 67, 3, 167, 193, 124, 185, 195, 3, 159, 65, 106, 145, 131, 4, 151, 65, 102, 137, 195, 4, 151, 193, 100, 137, 3, 5, 151, 193, 102, 137, 131, 5, 159, 193, 107, 153, 3, 6, 167, 193, 128, 193, 131, 7, 167, 193, 133, 201, 3, 8, 167, 193, 135, 201, 131, 8, 167, 193, 134, 201, 3, 9, 167, 65, 128, 193, 3, 10, 159, 65, 108, 153, 3, 12, 151, 193, 101, 137,
    3, 13, 151, 193, 100, 137, 131, 13, 151, 193, 101, 137, 3, 14, 159, 65, 108, 153, 3, 15, 167, 65, 128, 193, 3, 17, 167, 193, 134, 201, 3, 18, 167, 193, 135, 201, 131, 27, 33, 0, 26, 0, 40, 192, 209, 220, 4, 0, 40, 64, 212, 212, 68, 2, 16, 64, 241, 180, 132, 2, 28, 192, 42, 93, 196, 2, 52, 192, 75, 45, 4, 3, 44, 192, 73, 53, 68, 3, 44, 64, 68, 61, 132, 3, 20, 64, 36, 109, 132, 4, 20, 64, 31, 117, 196, 4, 20, 64, 29, 117,
    4, 5, 20, 192, 31, 109, 132, 5, 20, 64, 38, 101, 4, 6, 44, 64, 66, 61, 132, 7, 44, 192, 72, 53, 4, 8, 52, 192, 75, 45, 132, 8, 44, 64, 74, 53, 4, 9, 44, 64, 65, 61, 4, 10, 20, 192, 38, 101, 4, 12, 20, 64, 30, 117, 4, 13, 20, 64, 29, 117, 132, 13, 20, 64, 30, 117, 4, 14, 20, 192, 38, 101, 4, 15, 44, 64, 65, 61, 4, 17, 44, 192, 70, 53, 132, 17, 44, 64, 74, 53, 4, 18, 52, 192, 75, 45, 132, 27, 34, 0,
    2, 0, 200, 0, 153, 184, 0, 0, 200, 0, 153, 184, 128, 27, 35, 0, 14, 0, 7, 83, 29, 112, 41, 0, 23, 211, 27, 96, 169, 0, 63, 211, 22, 48, 41, 1, 135, 211, 13, 224, 232, 1, 47, 211, 3, 24, 41, 2, 119, 146, 12, 160, 105, 2, 23, 146, 21, 224, 169, 2, 103, 146, 19, 160, 41, 3, 87, 18, 20, 168, 41, 5, 111, 18, 14, 160, 41, 9, 23, 146, 12, 232, 41, 13, 15, 18, 5, 0, 42, 17, 191, 81, 5, 80, 42, 25, 175, 209, 6, 88,
    170, 27, 36, 0, 22, 0, 63, 198, 223, 157, 21, 0, 63, 198, 223, 157, 213, 1, 55, 70, 223, 189, 21, 2, 199, 198, 188, 229, 85, 2, 47, 71, 150, 141, 150, 2, 239, 69, 141, 117, 217, 2, 223, 68, 135, 173, 27, 3, 231, 68, 135, 157, 91, 3, 23, 69, 137, 53, 219, 3, 143, 197, 142, 29, 218, 4, 167, 197, 144, 213, 89, 5, 175, 197, 151, 117, 89, 6, 159, 197, 159, 61, 89, 7, 143, 197, 164, 45, 89, 8, 143, 197, 163, 53, 89, 9, 151, 197, 158, 101,
    89, 10, 159, 69, 147, 213, 89, 12, 159, 197, 143, 237, 89, 13, 159, 69, 145, 229, 89, 14, 151, 197, 155, 133, 89, 16, 143, 69, 158, 109, 89, 17, 143, 69, 159, 101, 153, 27, 37, 0, 22, 0, 159, 135, 1, 96, 1, 0, 159, 135, 1, 96, 193, 1, 175, 135, 155, 160, 0, 2, 159, 135, 37, 29, 64, 2, 127, 7, 110, 125, 128, 2, 143, 7, 67, 69, 0, 3, 143, 135, 68, 69, 128, 3, 143, 7, 74, 77, 128, 4, 143, 135, 75, 77, 0, 5, 143, 135, 73, 77,
    128, 5, 143, 7, 69, 69, 0, 6, 151, 135, 48, 45, 128, 7, 151, 135, 43, 37, 0, 8, 151, 135, 41, 37, 128, 8, 151, 7, 45, 37, 128, 9, 143, 7, 72, 77, 128, 12, 143, 135, 75, 77, 128, 13, 143, 135, 74, 77, 0, 14, 143, 7, 72, 77, 128, 14, 151, 7, 55, 53, 128, 16, 151, 135, 51, 45, 128, 17, 151, 135, 50, 45, 128, 27, 38, 0, 23, 0, 111, 65, 104, 200, 1, 0, 111, 65, 104, 200, 193, 1, 183, 65, 41, 184, 1, 2, 103, 130, 100, 144,
    65, 2, 191, 2, 176, 120, 129, 2, 135, 2, 131, 136, 1, 3, 135, 130, 132, 136, 129, 3, 143, 130, 138, 136, 129, 4, 143, 2, 140, 136, 1, 5, 143, 2, 138, 136, 129, 5, 135, 2, 133, 136, 1, 6, 111, 130, 111, 144, 129, 7, 111, 130, 106, 144, 1, 8, 103, 130, 104, 144, 129, 8, 111, 130, 108, 144, 129, 9, 143, 130, 136, 136, 129, 12, 143, 2, 140, 136, 129, 13, 143, 2, 139, 136, 1, 14, 143, 130, 136, 136, 129, 14, 127, 130, 126, 136, 129, 15, 119, 2,
    119, 136, 129, 16, 119, 2, 115, 144, 129, 17, 119, 2, 114, 144, 129, 27, 39, 0, 2, 0, 204, 138, 18, 44, 39, 0, 204, 138, 18, 44, 167, 27, 40, 0, 19, 0, 83, 78, 11, 164, 17, 0, 43, 78, 11, 156, 81, 0, 171, 205, 10, 140, 145, 0, 243, 76, 10, 116, 209, 0, 11, 76, 9, 92, 17, 1, 179, 199, 4, 228, 16, 2, 195, 198, 3, 212, 80, 2, 3, 70, 3, 196, 144, 2, 131, 197, 2, 188, 208, 2, 83, 69, 2, 180, 16, 3, 123, 197, 2, 188,
    16, 5, 219, 197, 2, 196, 16, 7, 123, 198, 3, 204, 16, 9, 67, 71, 4, 220, 16, 11, 27, 73, 6, 12, 17, 15, 27, 75, 8, 60, 17, 19, 219, 76, 10, 116, 17, 23, 139, 205, 10, 140, 17, 25, 35, 78, 11, 156, 145, 27, 41, 0, 20, 0, 191, 64, 66, 153, 5, 0, 191, 192, 69, 137, 69, 0, 191, 192, 78, 89, 133, 0, 191, 192, 92, 9, 197, 0, 199, 64, 110, 177, 4, 1, 7, 65, 190, 9, 3, 2, 31, 193, 206, 169, 66, 2, 47, 65, 220, 105,
    130, 2, 63, 193, 228, 57, 194, 2, 71, 193, 231, 41, 2, 3, 71, 193, 229, 41, 130, 3, 79, 65, 219, 33, 130, 4, 79, 65, 217, 33, 2, 5, 79, 65, 220, 33, 2, 6, 71, 65, 231, 41, 2, 8, 71, 193, 231, 41, 2, 9, 79, 65, 217, 33, 2, 13, 79, 65, 217, 33, 2, 14, 71, 193, 231, 41, 2, 18, 71, 193, 231, 41, 130, 27, 42, 0, 20, 0, 75, 194, 198, 105, 1, 0, 67, 194, 200, 105, 65, 0, 59, 194, 206, 97, 129, 0, 43, 194, 215, 89,
    193, 0, 251, 65, 240, 73, 65, 1, 203, 65, 11, 42, 193, 1, 163, 193, 34, 18, 65, 2, 147, 65, 43, 10, 129, 2, 131, 193, 48, 10, 193, 2, 131, 193, 50, 2, 1, 3, 131, 65, 49, 10, 129, 3, 147, 193, 42, 10, 129, 4, 147, 65, 41, 18, 1, 5, 147, 65, 43, 10, 1, 6, 131, 65, 50, 10, 1, 8, 131, 193, 50, 2, 1, 9, 147, 65, 41, 18, 1, 13, 147, 65, 41, 18, 1, 14, 131, 193, 50, 2, 1, 18, 131, 193, 50, 2, 129, 27, 43, 0,
    2, 0, 239, 64, 227, 36, 1, 0, 239, 64, 227, 36, 129, 27, 44, 0, 2, 0, 215, 197, 128, 76, 7, 0, 215, 197, 128, 76, 135, 27, 45, 0, 7, 0, 55, 194, 49, 72, 11, 0, 31, 194, 49, 56, 139, 0, 207, 64, 51, 160, 138, 2, 175, 64, 51, 144, 10, 3, 199, 64, 51, 152, 10, 7, 247, 65, 50, 40, 11, 23, 47, 194, 49, 64, 139, 27, 46, 0, 34, 0, 199, 203, 127, 20, 13, 0, 207, 75, 128, 196, 76, 0, 231, 75, 130, 236, 139, 0, 7, 204,
    132, 172, 202, 0, 87, 204, 140, 156, 71, 1, 135, 76, 146, 20, 134, 1, 167, 204, 152, 204, 196, 1, 191, 204, 159, 228, 3, 2, 191, 76, 167, 124, 67, 2, 231, 203, 187, 172, 134, 2, 47, 202, 215, 172, 204, 2, 15, 73, 229, 20, 16, 3, 255, 72, 227, 124, 80, 3, 15, 73, 221, 236, 144, 3, 95, 73, 202, 172, 17, 4, 199, 201, 182, 52, 146, 4, 231, 73, 176, 92, 210, 4, 247, 201, 173, 100, 18, 5, 231, 201, 176, 68, 146, 5, 199, 201, 184, 244, 17, 6,
    55, 73, 218, 140, 144, 7, 23, 201, 225, 52, 16, 8, 15, 73, 229, 20, 144, 8, 23, 73, 228, 28, 16, 9, 55, 73, 221, 52, 16, 10, 175, 201, 200, 124, 16, 12, 207, 201, 193, 148, 16, 13, 215, 201, 192, 148, 144, 13, 215, 201, 193, 148, 16, 14, 175, 73, 200, 124, 16, 15, 127, 73, 209, 92, 16, 16, 95, 201, 214, 76, 16, 17, 79, 73, 217, 60, 16, 18, 79, 201, 217, 60, 144, 27, 47, 0, 24, 0, 68, 194, 111, 89, 2, 0, 84, 194, 114, 89, 130, 0,
    116, 66, 122, 89, 2, 1, 244, 66, 148, 81, 66, 2, 28, 67, 156, 81, 130, 2, 68, 67, 166, 81, 194, 2, 92, 67, 171, 73, 2, 3, 84, 195, 169, 73, 66, 3, 68, 67, 165, 81, 130, 3, 204, 194, 139, 81, 130, 4, 180, 194, 135, 89, 194, 4, 172, 194, 133, 89, 2, 5, 188, 66, 136, 89, 130, 5, 212, 66, 141, 81, 2, 6, 60, 195, 163, 81, 130, 7, 84, 67, 169, 73, 2, 8, 92, 67, 171, 73, 130, 8, 84, 195, 168, 81, 130, 9, 252, 66, 150, 81,
    130, 12, 244, 194, 147, 81, 130, 13, 252, 194, 149, 81, 130, 14, 44, 195, 160, 81, 130, 16, 60, 67, 163, 81, 130, 17, 60, 195, 163, 81, 130, 27, 48, 0, 24, 0, 195, 193, 0, 28, 2, 0, 187, 129, 2, 44, 130, 0, 171, 129, 10, 76, 2, 1, 123, 1, 39, 204, 66, 2, 107, 129, 48, 244, 130, 2, 91, 129, 60, 44, 195, 2, 75, 129, 66, 68, 3, 3, 83, 129, 64, 60, 67, 3, 91, 129, 59, 36, 131, 3, 139, 129, 29, 164, 130, 4, 147, 1, 25, 140,
    194, 4, 155, 1, 23, 132, 2, 5, 147, 129, 25, 140, 130, 5, 139, 129, 31, 172, 2, 6, 91, 129, 57, 28, 131, 7, 83, 1, 64, 52, 3, 8, 75, 129, 66, 68, 131, 8, 83, 129, 63, 52, 131, 9, 123, 129, 41, 212, 130, 12, 123, 129, 38, 204, 130, 13, 123, 1, 41, 212, 130, 14, 99, 129, 53, 12, 131, 16, 91, 129, 56, 20, 131, 17, 91, 129, 57, 28, 131, 27, 49, 0, 2, 0, 208, 135, 152, 100, 6, 0, 208, 135, 152, 100, 134, 27, 50, 0, 7, 0,
    83, 193, 28, 188, 7, 0, 67, 193, 27, 180, 135, 0, 123, 64, 16, 156, 135, 2, 107, 64, 15, 148, 135, 4, 171, 192, 18, 156, 135, 12, 11, 193, 24, 172, 135, 20, 75, 65, 28, 188, 135, 27, 51, 0, 28, 0, 27, 67, 219, 208, 0, 0, 27, 195, 222, 208, 64, 0, 19, 195, 232, 208, 128, 0, 19, 67, 247, 208, 192, 0, 11, 67, 9, 209, 0, 1, 235, 194, 93, 209, 0, 2, 219, 66, 111, 209, 64, 2, 219, 66, 125, 209, 128, 2, 211, 194, 134, 209, 192, 2,
    211, 194, 137, 209, 0, 3, 211, 194, 136, 209, 64, 3, 211, 194, 128, 209, 192, 3, 219, 66, 118, 209, 64, 4, 227, 66, 110, 209, 192, 4, 227, 66, 109, 209, 64, 5, 219, 66, 112, 209, 192, 5, 211, 66, 134, 209, 192, 7, 211, 66, 137, 209, 64, 8, 211, 194, 137, 209, 192, 8, 211, 66, 133, 209, 192, 9, 219, 194, 116, 209, 192, 11, 227, 194, 110, 209, 192, 12, 227, 194, 108, 209, 64, 13, 227, 194, 110, 209, 64, 14, 219, 194, 116, 209, 64, 15, 211, 66, 133, 209,
    64, 17, 211, 194, 137, 209, 64, 18, 211, 194, 137, 209, 128, 27, 52, 128, 2, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 0, 128, 27, 53, 160, 35, 0, 160, 12, 128, 35, 192, 65, 36, 18, 2, 0, 160, 12, 128, 35, 192, 193, 40, 18, 66, 0, 159, 12, 160, 163, 208, 193, 52, 26, 130, 0, 157, 12, 176, 227, 232, 65, 70, 34, 194, 0, 156, 12, 192, 35, 8, 194, 92, 50, 2, 1, 154, 12, 224, 163, 40, 194, 117, 66, 66, 1, 148, 16,
    32, 164, 136, 194, 194, 114, 2, 2, 146, 16, 48, 228, 222, 236, 42, 132, 66, 2, 145, 16, 64, 36, 198, 107, 44, 140, 130, 2, 144, 16, 80, 100, 14, 107, 45, 148, 194, 2, 144, 16, 80, 100, 198, 106, 45, 156, 2, 3, 144, 16, 80, 100, 222, 106, 45, 148, 66, 3, 144, 16, 80, 100, 38, 235, 44, 148, 130, 3, 146, 16, 48, 228, 182, 236, 42, 132, 130, 4, 146, 16, 48, 228, 246, 236, 42, 132, 194, 4, 146, 16, 48, 228, 14, 237, 42, 132, 2, 5, 146, 16,
    48, 228, 238, 236, 42, 132, 130, 5, 146, 16, 48, 228, 158, 108, 43, 132, 2, 6, 144, 16, 80, 100, 62, 235, 44, 148, 130, 7, 144, 16, 80, 100, 230, 106, 45, 148, 2, 8, 144, 16, 80, 100, 198, 106, 45, 156, 130, 8, 144, 16, 80, 100, 214, 106, 45, 156, 2, 9, 144, 16, 80, 100, 6, 107, 45, 148, 130, 9, 144, 16, 80, 100, 150, 107, 44, 148, 130, 10, 145, 16, 64, 36, 70, 236, 43, 140, 130, 11, 145, 16, 64, 36, 214, 236, 42, 132, 130, 12, 146, 16,
    48, 228, 254, 236, 42, 132, 2, 13, 146, 16, 48, 228, 14, 237, 42, 132, 130, 13, 146, 16, 48, 228, 254, 236, 42, 132, 2, 14, 146, 16, 48, 228, 214, 236, 42, 132, 130, 14, 145, 16, 64, 36, 70, 236, 43, 140, 130, 15, 144, 16, 64, 36, 150, 107, 44, 148, 130, 16, 144, 16, 64, 36, 6, 107, 45, 148, 130, 17, 143, 16, 80, 100, 214, 106, 45, 156, 2, 18, 143, 16, 80, 100, 198, 106, 45, 156, 130, 27, 54, 0, 2, 0, 20, 4, 145, 248, 0, 0, 20, 4,
    145, 248, 128, 27, 55, 0, 7, 0, 238, 32, 197, 93, 25, 0, 14, 33, 197, 85, 153, 0, 102, 161, 197, 77, 25, 1, 38, 162, 198, 53, 25, 2, 94, 34, 199, 45, 153, 2, 94, 34, 199, 45, 25, 27, 62, 34, 199, 53, 153, 27, 56, 0, 20, 0, 199, 97, 255, 165, 22, 0, 199, 97, 255, 165, 86, 0, 215, 225, 253, 205, 150, 0, 7, 226, 251, 37, 215, 0, 55, 98, 249, 157, 23, 1, 87, 98, 248, 13, 88, 1, 79, 226, 249, 77, 152, 1, 31, 226, 253, 109,
    216, 1, 135, 225, 9, 102, 88, 2, 207, 96, 23, 38, 216, 2, 47, 224, 33, 222, 87, 3, 255, 95, 37, 198, 151, 3, 231, 95, 38, 190, 215, 3, 255, 95, 37, 214, 87, 4, 143, 96, 29, 134, 216, 5, 159, 224, 27, 158, 88, 6, 135, 224, 29, 118, 88, 7, 87, 224, 31, 70, 88, 8, 79, 96, 32, 62, 88, 27, 95, 224, 31, 54, 152, 27, 57, 0, 3, 0, 171, 128, 20, 152, 0, 0, 123, 0, 20, 152, 0, 2, 107, 0, 20, 152, 128, 27, 58, 64, 20, 0,
    75, 129, 48, 124, 5, 0, 75, 129, 48, 124, 133, 0, 75, 1, 49, 28, 198, 0, 75, 129, 49, 156, 7, 1, 67, 1, 50, 108, 73, 1, 67, 129, 50, 236, 138, 1, 67, 129, 50, 140, 203, 1, 67, 1, 51, 116, 11, 2, 43, 129, 52, 220, 138, 2, 251, 128, 57, 68, 137, 3, 243, 0, 58, 12, 201, 3, 243, 128, 58, 244, 8, 4, 243, 0, 58, 20, 137, 4, 19, 1, 55, 28, 10, 6, 27, 129, 54, 60, 138, 6, 19, 1, 55, 4, 138, 7, 3, 1, 56, 172,
    137, 8, 3, 129, 56, 164, 137, 9, 11, 129, 55, 236, 137, 11, 11, 129, 55, 236, 137, 27, 59, 0, 2, 0, 75, 64, 14, 112, 0, 0, 75, 64, 14, 112, 128, 27, 60, 0, 29, 0, 91, 192, 32, 92, 2, 0, 91, 192, 32, 92, 194, 0, 83, 64, 28, 172, 2, 1, 75, 64, 17, 100, 67, 1, 59, 64, 4, 68, 132, 1, 43, 0, 7, 252, 196, 1, 27, 128, 11, 68, 5, 2, 35, 128, 11, 12, 69, 2, 35, 128, 11, 124, 132, 2, 35, 128, 11, 164, 195, 2,
    43, 128, 11, 156, 128, 3, 51, 128, 11, 56, 192, 3, 51, 128, 11, 208, 0, 4, 51, 128, 11, 8, 65, 4, 51, 128, 11, 240, 128, 4, 51, 128, 11, 176, 192, 4, 51, 128, 11, 20, 64, 5, 43, 128, 11, 4, 193, 5, 43, 128, 11, 204, 65, 6, 43, 128, 11, 12, 130, 6, 43, 128, 11, 36, 194, 6, 43, 128, 11, 252, 65, 7, 43, 128, 11, 188, 192, 8, 43, 128, 11, 140, 64, 9, 43, 128, 11, 172, 192, 9, 43, 128, 11, 124, 65, 11, 43, 128, 11, 156,
    193, 11, 43, 128, 11, 36, 193, 13, 43, 128, 11, 20, 129, 27, 61, 0, 2, 0, 55, 192, 4, 252, 0, 0, 55, 192, 4, 252, 128, 27, 62, 0, 2, 0, 83, 196, 179, 120, 3, 0, 83, 196, 179, 120, 131, 27, 63, 0, 2, 0, 243, 192, 47, 25, 0, 0, 243, 192, 47, 25, 128, 27, 64, 0, 2, 0, 132, 65, 211, 32, 4, 0, 132, 65, 211, 32, 132, 27, 65, 0, 2, 0, 56, 0, 9, 228, 3, 0, 56, 0, 9, 228, 131, 27, 66, 0, 2, 0, 147, 84,
    28, 212, 37, 0, 147, 84, 28, 212, 165, 27, 67, 0, 2, 0, 3, 194, 12, 105, 22, 0, 3, 194, 12, 105, 150, 27, 68, 0, 2, 0, 131, 73, 118, 164, 5, 0, 131, 73, 118, 164, 133, 27, 69, 0, 2, 0, 211, 193, 185, 48, 0, 0, 211, 193, 185, 48, 128, 27, 70, 0, 2, 0, 79, 86, 140, 92, 1, 0, 79, 86, 140, 92, 129, 27, 71, 0, 2, 0, 151, 74, 9, 152, 17, 0, 151, 74, 9, 152, 145, 27, 72, 0, 2, 0, 219, 64, 217, 124, 12, 0,
    219, 64, 217, 124, 140, 27, 73, 0, 2, 0, 103, 66, 90, 97, 3, 0, 103, 66, 90, 97, 131, 27, 74, 0, 2, 0, 3, 64, 225, 136, 0, 0, 3, 64, 225, 136, 128, 27, 75, 0, 2, 0, 147, 5, 1, 16, 0, 0, 147, 5, 1, 16, 128, 27, 76, 0, 2, 0, 151, 192, 15, 148, 9, 0, 151, 192, 15, 148, 137, 27, 77, 0, 2, 0, 147, 198, 99, 24, 10, 0, 147, 198, 99, 24, 138, 27, 78, 0, 2, 0, 12, 192, 76, 205, 2, 0, 12, 192, 76, 205,
    130, 27, 79, 0, 2, 0, 223, 65, 65, 48, 1, 0, 223, 65, 65, 48, 129, 27, 80, 0, 2, 0, 80, 192, 24, 40, 0, 0, 80, 192, 24, 40, 128, 27, 81, 0, 2, 0, 143, 133, 26, 152, 8, 0, 143, 133, 26, 152, 136, 27, 82, 0, 2, 0, 111, 68, 54, 77, 2, 0, 111, 68, 54, 77, 130, 27, 83, 0, 2, 0, 115, 64, 122, 185, 1, 0, 115, 64, 122, 185, 129, 27, 84, 0, 2, 0, 180, 65, 208, 136, 1, 0, 180, 65, 208, 136, 129, 27, 85, 0,
    2, 0, 0, 64, 3, 128, 0, 0, 0, 64, 3, 128, 128, 27, 86, 64, 2, 0, 58, 237, 1, 24, 0, 0, 58, 237, 1, 24, 128, 27
};

BOOST_STATIC_ASSERT(sizeof(hmmmAnimation) == hmmmLength);

TEST(load_hmmm) {
    CalBufferSource cbs(fromString(hmmmAnimation));
    CalCoreAnimationPtr anim = CalLoader::loadCoreAnimation(cbs);
    CHECK_EQUAL(anim->tracks.size(), 70);

    CalCoreKeyframe expected[14] = {
        CalCoreKeyframe(0, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(0, 0, -0.179775f, 0.983708f)),
        CalCoreKeyframe(0.0333333f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.00048852f, -0.00048852f, -0.178798f, 0.983886f)),
        CalCoreKeyframe(0.1f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.00195408f, -0.00390816f, -0.172936f, 0.984923f)),
        CalCoreKeyframe(0.233333f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.00879336f, -0.0161212f, -0.153395f, 0.987994f)),
        CalCoreKeyframe(0.3f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.012213f, -0.0229604f, -0.147044f, 0.988788f)),
        CalCoreKeyframe(0.366667f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.0146556f, -0.0283341f, -0.144113f, 0.989047f)),
        CalCoreKeyframe(0.5f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.0151441f, -0.0337079f, -0.144113f, 0.988871f)),
        CalCoreKeyframe(0.633333f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.0136786f, -0.036639f, -0.146067f, 0.988501f)),
        CalCoreKeyframe(0.9f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.0127015f, -0.037616f, -0.148999f, 0.98804f)),
        CalCoreKeyframe(1.43333f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.01319f, -0.0361505f, -0.150464f, 0.987866f)),
        CalCoreKeyframe(1.96667f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.0146556f, -0.0317538f, -0.150464f, 0.987997f)),
        CalCoreKeyframe(3.56667f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.0146556f, -0.0312653f, -0.149976f, 0.988087f)),
        CalCoreKeyframe(3.63333f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.0136786f, -0.0317538f, -0.14851f, 0.988306f)),
        CalCoreKeyframe(3.66667f, CalVector(1e+010, 1e+010, 1e+010), CalQuaternion(-0.0107474f, -0.0312653f, -0.147533f, 0.988504f)),
    };

    CalCoreTrack* track2 = &anim->tracks[2];
    CHECK_EQUAL(14u, track2->keyframes.size());
    for (size_t i = 0; i < 14; ++i) {
        CHECK_EQUAL(expected[i], track2->keyframes[i]);
    }
}

// non-track siblings

TEST(CalVectorFromDataSrc) {
    char buf[7];
    CalBufferSource bs(buf, sizeof(buf) / sizeof(char));

    CalVector vec;
    CHECK_EQUAL(false, CalVectorFromDataSrc(bs, &vec));
}


TEST(loading_mesh_without_vertex_colors_defaults_to_white) {
    boost::shared_ptr<CalCoreSubmesh> sm(new CalCoreSubmesh(1, 0, 0));
    CalCoreSubmesh::Vertex v;
    sm->addVertex(v, CalMakeColor(CalVector(0, 0, 0)), std::vector<CalCoreSubmesh::Influence>());
    sm->setHasNonWhiteVertexColors(false);

    CalCoreMesh cm;
    cm.addCoreSubmesh(sm);

    std::ostringstream os;
    CalSaver::saveCoreMesh(os, &cm);

    std::string str = os.str();
    CalBufferSource cbs(str.data(), str.size());
    CalCoreMesh* loaded = CalLoader::loadCoreMesh(cbs);
    CHECK(loaded);
    CHECK_EQUAL(1u, cm.getCoreSubmeshCount());
    CHECK_EQUAL(1u, loaded->getCoreSubmeshCount());

    CHECK_EQUAL(cm.getCoreSubmesh(0)->hasNonWhiteVertexColors(),
                loaded->getCoreSubmesh(0)->hasNonWhiteVertexColors());

    delete loaded;
}

TEST(converting_xml_to_binary_then_back_to_xml_does_not_modify_animation) {
    CalBufferSource cbs(fromString(animationText));
    CalCoreAnimationPtr anim1 = CalLoader::loadCoreAnimation(cbs);
    CHECK(anim1);

    std::stringstream buf1;
    CalSaver::saveXmlCoreAnimation(buf1, anim1.get());

    std::stringstream output;
    CalSaver::saveCoreAnimation(output, anim1.get());

    std::string str = output.str();
    CalBufferSource cbs2(str.data(), str.size());
    CalCoreAnimationPtr anim2 = CalLoader::loadCoreAnimation(cbs2);
    CHECK(anim2);

    CHECK_EQUAL(*anim1, *anim2);
}

const char invalid_morph[] =
    "<HEADER MAGIC=\"XAF\" VERSION=\"919\" />"
    "<ANIMATION NUMTRACKS=\"85\" DURATION=\"4.8571429\">"
    "    <TRACK BONEID=\"1\" NUMKEYFRAMES=\"2\">"
    "        <KEYFRAME TIME=\"0\">"
    "            <TRANSLATION>0 0 663.894</TRANSLATION>"
    "            <ROTATION>0 0 0 1</ROTATION>"
    "        </KEYFRAME>"
    "        <KEYFRAME TIME=\"4.8571429\">"
    "            <TRANSLATION>0 0 663.894</TRANSLATION>"
    "            <ROTATION>0 0 0 1</ROTATION>"
    "        </KEYFRAME>"
    "    </TRACK>"
    "</ANIMATION>"
    ;

const char header_only[] =
    "<HEADER MAGIC=\"XPF\" VERSION=\"919\" />"
    ;

const char header_only_without_magic[] =
    "<HEADER MAGIC=\"XPF\" VERSION=\"919\" />"
    "<ANIMATION>"
    "</ANIMATION>"
    ;

TEST(morph_loader_doesnt_crash_on_invalid_data) {
    CalBufferSource cbs1(fromString(invalid_morph));
    CHECK(!CalLoader::loadCoreAnimatedMorph(cbs1));

    CalBufferSource cbs2(fromString(header_only));
    CHECK(!CalLoader::loadCoreAnimatedMorph(cbs2));

    // passes on mac, fails on windows...  weird.
    //CHECK(CalLoader::loadCoreAnimatedMorph(header_only_without_magic));
}
