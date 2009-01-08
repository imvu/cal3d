#include "TestPrologue.h"
#include <cal3d/coremodel.h>
#include <cal3d/renderer.h>
#include <cal3d/model.h>
#include <cal3d/skeleton.h>
#include <cal3d/submesh.h>
#include <cal3d/physique.h>
#include <cal3d/vector4.h>
#include <cal3d/streamops.h>

const CalColor32 black = 0;

TEST(not_static_without_vertices) {
    CalCoreSubmesh csm(0, 0, 0);
    CHECK(!csm.isStatic());
}

TEST(is_static_if_all_vertices_are_influenced_by_same_bone) {
    CalCoreSubmesh csm(1, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);

    BoneTransform bts[1];
    bts[0].rowx.x = 1.0f;
    bts[0].rowx.y = 0.0f;
    bts[0].rowx.z = 0.0f;
    bts[0].rowx.w = 0.0f;
    bts[0].rowy.x = 0.0f;
    bts[0].rowy.y = 1.0f;
    bts[0].rowy.z = 0.0f;
    bts[0].rowy.w = 0.0f;
    bts[0].rowz.x = 0.0f;
    bts[0].rowz.y = 0.0f;
    bts[0].rowz.z = 1.0f;
    bts[0].rowz.w = 0.0f;

    CHECK(csm.isStatic());
    CHECK_EQUAL(bts[0], csm.getStaticTransform(bts));
}

TEST(is_static_if_two_influences) {
    CalCoreSubmesh csm(1, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(2);
    inf[0].boneId = 0;
    inf[0].weight = 0.5f;
    inf[1].boneId = 1;
    inf[1].weight = 0.5f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);

    BoneTransform bts[2];
    bts[0].rowx.x = 1.0f;
    bts[0].rowx.y = 0.0f;
    bts[0].rowx.z = 0.0f;
    bts[0].rowx.w = 0.0f;
    bts[0].rowy.x = 0.0f;
    bts[0].rowy.y = 1.0f;
    bts[0].rowy.z = 0.0f;
    bts[0].rowy.w = 0.0f;
    bts[0].rowz.x = 0.0f;
    bts[0].rowz.y = 0.0f;
    bts[0].rowz.z = 1.0f;
    bts[0].rowz.w = 0.0f;

    bts[1] = bts[0];
    bts[1].rowx.w = 1.0f;
    bts[1].rowy.w = 1.0f;
    bts[1].rowz.w = 1.0f;

    CHECK(csm.isStatic());
    BoneTransform staticTransform = csm.getStaticTransform(bts);
    CHECK_EQUAL(0.5f, staticTransform.rowx.w);
    CHECK_EQUAL(0.5f, staticTransform.rowy.w);
    CHECK_EQUAL(0.5f, staticTransform.rowz.w);
}

TEST(not_static_if_two_vertices_are_influenced_by_different_bones) {
    CalCoreSubmesh csm(2, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);
    inf[0].boneId = 1;
    inf[0].weight = 1.0f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);

    CHECK(!csm.isStatic());
}

TEST(is_not_static_if_first_and_third_vertices_have_same_influence) {
    CalCoreSubmesh csm(3, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);
    inf[0].boneId = 1;
    inf[0].weight = 1.0f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);

    CHECK(!csm.isStatic());
}

TEST(is_static_if_two_vertices_have_influences_in_different_order) {
    CalCoreSubmesh csm(2, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(2);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    inf[1].boneId = 1;
    inf[1].weight = 0.0f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);
    inf[0].boneId = 1;
    inf[0].weight = 0.0f;
    inf[1].boneId = 0;
    inf[1].weight = 1.0f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);

    CHECK(csm.isStatic());
}

TEST(is_not_static_if_has_morph_targets) {
    CalCoreSubmesh csm(2, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(2);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    inf[1].boneId = 1;
    inf[1].weight = 0.0f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);
    inf[0].boneId = 1;
    inf[0].weight = 0.0f;
    inf[1].boneId = 0;
    inf[1].weight = 1.0f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);

    boost::shared_ptr<CalCoreSubMorphTarget> morphTarget(new CalCoreSubMorphTarget);
    csm.addCoreSubMorphTarget(morphTarget);

    CHECK(!csm.isStatic());
}

TEST(CalSubmesh_getFaces_succeeds_if_face_list_is_empty) {
    CalCoreSubmesh csm(0, 0, 0);

    CalSubmesh sm(&csm);

    sm.getVectorFace();
}

TEST(CalRenderer_getTextureCoordinates_when_there_are_no_texture_coordinates) {
    CalCoreSubmesh* coreSubmesh = new CalCoreSubmesh(0, 1, 0);

    CalCoreMesh* coreMesh = new CalCoreMesh;
    coreMesh->addCoreSubmesh(coreSubmesh);

    CalCoreModel coreModel;
    CHECK(coreModel.createInternal("model"));
    coreModel.addCoreMesh(coreMesh);

    CalCoreSkeleton* coreSkeleton = new CalCoreSkeleton;
    coreModel.setCoreSkeleton(coreSkeleton);

    CalModel model;
    model.create(&coreModel);
    CHECK(model.attachMesh(0));

    CalSubmesh* submesh = model.getMesh(0)->getSubmesh(0);
    const std::vector<CalCoreSubmesh::TextureCoordinate>& texCoords = submesh->getCoreSubmesh()->getVectorVectorTextureCoordinate()[0];
    CHECK_EQUAL(0, texCoords.size());

    model.destroy();
    coreModel.destroy();
}

TEST(CalRenderer_getNormals_when_there_are_no_normals) {
    CalCoreSubmesh* coreSubmesh = new CalCoreSubmesh(0, 0, 0);

    CalCoreMesh* coreMesh = new CalCoreMesh;
    coreMesh->addCoreSubmesh(coreSubmesh);

    CalCoreModel coreModel;
    CHECK(coreModel.createInternal("model"));
    coreModel.addCoreMesh(coreMesh);

    CalCoreSkeleton* coreSkeleton = new CalCoreSkeleton;
    coreModel.setCoreSkeleton(coreSkeleton);

    CalModel model;
    model.create(&coreModel);
    CHECK(model.attachMesh(0));

    CalSubmesh* submesh = model.getMesh(0)->getSubmesh(0);
    CalPhysique::calculateVerticesAndNormals(
      model.getSkeleton()->boneTransforms.data,
      submesh,
      0);

    model.destroy();
    coreModel.destroy();
}
