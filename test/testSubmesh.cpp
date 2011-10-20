#include "TestPrologue.h"
#include <cal3d/buffersource.h>
#include <cal3d/coremesh.h>
#include <cal3d/mesh.h>
#include <cal3d/loader.h>
#include <cal3d/saver.h>
#include <cal3d/skeleton.h>
#include <cal3d/submesh.h>
#include <cal3d/physique.h>
#include <cal3d/vector4.h>
#include <cal3d/streamops.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/coremorphtarget.h>

TEST(saving_and_loading_submesh_with_morph_stores_differences_in_memory_but_absolute_in_file) {
    CalCoreSubmeshPtr csm(new CalCoreSubmesh(1, 0, 0));
    CalCoreSubmesh::Vertex vertex;
    vertex.position = CalVector4(1, 1, 1, 1);
    vertex.normal = CalVector4(-1, -1 -1, 0);
    csm->addVertex(vertex, CalColor32(), CalCoreSubmesh::InfluenceVector());

    CalCoreMorphTarget::MorphVertexArray morphVertices;
    CalCoreMorphTarget::MorphVertex mv;
    mv.vertexId = 0;
    mv.position = CalVector4(2, 2, 2, 1);
    mv.normal = CalVector4(-2, -2, -2, 0);
    morphVertices.push_back(mv);
    CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget("m", 1, morphVertices));
    csm->addMorphTarget(morphTarget);

    CalCoreMesh cm;
    cm.submeshes.push_back(csm);

    { // test binary format

        std::ostringstream os;
        CHECK(CalSaver::saveCoreMesh(os, &cm));

        std::string buffer = os.str();
        CalBufferSource source(buffer.c_str(), buffer.size());
        CalCoreMeshPtr loaded = CalLoader::loadCoreMesh(source);
        CHECK(loaded);

        CHECK_EQUAL(1u, loaded->submeshes.size());
        CHECK_EQUAL(1u, loaded->submeshes[0]->getMorphTargets().size());
        CHECK_EQUAL(1u, loaded->submeshes[0]->getMorphTargets()[0]->morphVertices.size());
        CHECK_EQUAL(CalVector4(2, 2, 2, 1),    loaded->submeshes[0]->getMorphTargets()[0]->morphVertices[0].position);
        CHECK_EQUAL(CalVector4(-2, -2, -2, 0), loaded->submeshes[0]->getMorphTargets()[0]->morphVertices[0].normal);
    }

#if 0
    { // test XML format

        std::ostringstream os;
        CHECK(CalSaver::saveCoreMesh(os, &cm));

        CalBufferSource source(os.str().c_str(), os.str().size());
        CalCoreMeshPtr loaded = CalLoader::loadCoreMesh(source);
        CHECK(loaded);

        CHECK_EQUAL(CalVector4(1, 1, 1, 1),    loaded->submeshes[0]->getMorphTargets()[0]->morphVertices[0].position);
        CHECK_EQUAL(CalVector4(-1, -1, -1, 0), loaded->submeshes[0]->getMorphTargets()[0]->morphVertices[0].normal);
    }
#endif
}

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
    csm.addVertex(v, black, inf);

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
    csm.addVertex(v, black, inf);

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
    csm.addVertex(v, black, inf);
    inf[0].boneId = 1;
    inf[0].weight = 1.0f;
    csm.addVertex(v, black, inf);

    CHECK(!csm.isStatic());
}

TEST(is_not_static_if_first_and_third_vertices_have_same_influence) {
    CalCoreSubmesh csm(3, 0, 0);

    CalCoreSubmesh::Vertex v;
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    csm.addVertex(v, black, inf);
    inf[0].boneId = 1;
    inf[0].weight = 1.0f;
    csm.addVertex(v, black, inf);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    csm.addVertex(v, black, inf);

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
    csm.addVertex(v, black, inf);
    inf[0].boneId = 1;
    inf[0].weight = 0.0f;
    inf[1].boneId = 0;
    inf[1].weight = 1.0f;
    csm.addVertex(v, black, inf);

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
    csm.addVertex(v, black, inf);
    inf[0].boneId = 1;
    inf[0].weight = 0.0f;
    inf[1].boneId = 0;
    inf[1].weight = 1.0f;
    csm.addVertex(v, black, inf);

    CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget("", 0, CalCoreMorphTarget::MorphVertexArray()));
    csm.addMorphTarget(morphTarget);

    CHECK(!csm.isStatic());
}

TEST(CalRenderer_getTextureCoordinates_when_there_are_no_texture_coordinates) {
    boost::shared_ptr<CalCoreSubmesh> coreSubmesh(new CalCoreSubmesh(0, 1, 0));

    boost::shared_ptr<CalCoreMesh> coreMesh(new CalCoreMesh);
    coreMesh->submeshes.push_back(coreSubmesh);

    CalMesh mesh(coreMesh);

    CalSubmesh* submesh = mesh.submeshes[0].get();
    const std::vector<CalCoreSubmesh::TextureCoordinate>& texCoords = submesh->coreSubmesh->getVectorVectorTextureCoordinate()[0];
    CHECK_EQUAL(0, texCoords.size());
}

TEST(CalRenderer_getNormals_when_there_are_no_normals) {
    boost::shared_ptr<CalCoreSubmesh> coreSubmesh(new CalCoreSubmesh(0, 0, 0));

    boost::shared_ptr<CalCoreMesh> coreMesh(new CalCoreMesh);
    coreMesh->submeshes.push_back(coreSubmesh);

    boost::shared_ptr<CalCoreSkeleton> coreSkeleton(new CalCoreSkeleton);

    CalSkeleton skeleton(coreSkeleton);
    CalMesh mesh(coreMesh);

    CalSubmesh* submesh = mesh.submeshes[0].get();
    CHECK(submesh);

    CalPhysique::calculateVerticesAndNormals(
        skeleton.boneTransforms.data(),
        submesh,
        0);
}

TEST(aabox_empty_without_vertices) {
    CalCoreSubmesh csm(0, 0, 0);
    CHECK_EQUAL(CalAABox(), csm.getBoundingVolume());
}

TEST(aabox_is_single_point_with_one_vertex) {
    CalCoreSubmesh csm(1, 0, 0);
    CalVector pos(1.0f, 2.0f, 3.0f);

    CalCoreSubmesh::Vertex v;
    v.position.setAsPoint(pos);
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    CHECK_EQUAL(CalAABox(pos, pos), csm.getBoundingVolume());
}

TEST(aabox_is_min_max_of_all_vertices) {
    CalCoreSubmesh csm(3, 0, 0);

    CalCoreSubmesh::Vertex v;
    v.position.setAsPoint(CalVector(1.0f, 2.0f, 3.0f));
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    v.position.setAsPoint(CalVector(2.0f, 3.0f, 1.0f));
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    v.position.setAsPoint(CalVector(3.0f, 1.0f, 2.0f));
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    CHECK_EQUAL(
        CalAABox(CalVector(1.0f, 1.0f, 1.0f), CalVector(3.0f, 3.0f, 3.0f)),
        csm.getBoundingVolume());
}
