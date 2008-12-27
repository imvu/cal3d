#include "TestPrologue.h"
#include <cal3d/renderer.h>
#include <cal3d/model.h>
#include <cal3d/submesh.h>
#include <cal3d/physique.h>

CalColor32 black = 0;

TEST(core_submesh_without_vertices_is_not_static) {
    CalCoreSubmesh csm(0, 0, 0);
    CHECK(!csm.isStatic());
}

TEST(core_submesh_is_marked_as_static_if_all_vertices_are_influenced_by_same_bone) {
    CalCoreSubmesh csm(1, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    csm.addVertex(v, black, CalCoreSubmesh::LodData(), inf);

    CHECK(csm.isStatic());
}

TEST(core_submesh_is_not_static_if_two_vertices_are_influenced_by_different_bones) {
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
