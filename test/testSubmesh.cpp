#include "TestPrologue.h"
#include <cal3d/buffersource.h>
#include <cal3d/coremesh.h>
#include <cal3d/loader.h>
#include <cal3d/saver.h>
#include <cal3d/skeleton.h>
#include <cal3d/submesh.h>
#include <cal3d/physique.h>
#include <cal3d/vector4.h>
#include <cal3d/streamops.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/coremorphtarget.h>


inline std::ostream& operator<<(std::ostream& os, const CalCoreSubmesh::Face& f) {
    return os << '(' << f.vertexId[0] << ", " << f.vertexId[1] << ", " << f.vertexId[2] << ')';
}

TEST(saving_and_loading_submesh_with_morph_stores_differences_in_memory_but_absolute_in_file) {
    CalCoreSubmeshPtr csm(new CalCoreSubmesh(1, 0, 0));
    CalCoreSubmesh::Vertex vertex;
    vertex.position = CalVector4(1, 1, 1, 0);
    vertex.normal = CalVector4(-1, -1 -1, 0);
    csm->addVertex(vertex, CalColor32(), CalCoreSubmesh::InfluenceVector());

    CalCoreMorphTarget::VertexOffsetArray vertexOffsets;
    VertexOffset mv;
    mv.vertexId = 0;
    mv.position = CalVector4(2, 2, 2, 0);
    mv.normal = CalVector4(-2, -2, -2, 0);
    vertexOffsets.push_back(mv);
    CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget("m", 1, vertexOffsets));
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
        CHECK_EQUAL(1u, loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets.size());
        CHECK_EQUAL(CalVector4(2, 2, 2, 0),    loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets[0].position);
        CHECK_EQUAL(CalVector4(-2, -2, -2, 0), loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets[0].normal);
    }

    { // test XML format

        std::ostringstream os;
        CHECK(CalSaver::saveXmlCoreMesh(os, &cm));

        std::string buffer = os.str();
        CalBufferSource source(buffer.c_str(), buffer.size());
        CalCoreMeshPtr loaded = CalLoader::loadCoreMesh(source);
        CHECK(loaded);

        CHECK_EQUAL(1u, loaded->submeshes.size());
        CHECK_EQUAL(1u, loaded->submeshes[0]->getMorphTargets().size());
        CHECK_EQUAL(1u, loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets.size());
        CHECK_EQUAL(CalVector4(2, 2, 2, 0),    loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets[0].position);
        CHECK_EQUAL(CalVector4(-2, -2, -2, 0), loaded->submeshes[0]->getMorphTargets()[0]->vertexOffsets[0].normal);
    }
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

    CalCoreMorphTarget::VertexOffsetArray vertexOffsets;
    VertexOffset mv;
    mv.vertexId = 0;
    mv.position = CalVector4(0, 1, 2, 0);
    mv.normal = CalVector4(0, -1, -2, 0);
    vertexOffsets.push_back(mv);

    CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget("", 1, vertexOffsets));
    csm.addMorphTarget(morphTarget);

    CHECK(!csm.isStatic());
}

TEST(CalRenderer_getTextureCoordinates_when_there_are_no_texture_coordinates) {
    CalCoreSubmeshPtr coreSubmesh(new CalCoreSubmesh(0, 1, 0));

    const std::vector<CalCoreSubmesh::TextureCoordinate>& texCoords = coreSubmesh->getTextureCoordinates();
    CHECK_EQUAL(0u, texCoords.size());
}

TEST(CalRenderer_getNormals_when_there_are_no_normals) {
    CalCoreSubmeshPtr coreSubmesh(new CalCoreSubmesh(0, 0, 0));

    CalCoreMeshPtr coreMesh(new CalCoreMesh);
    coreMesh->submeshes.push_back(coreSubmesh);

    CalCoreSkeletonPtr coreSkeleton(new CalCoreSkeleton);

    CalSkeleton skeleton(coreSkeleton);
    CalSubmesh submesh(coreSubmesh);

    CalPhysique::calculateVerticesAndNormals(
        skeleton.boneTransforms.data(),
        &submesh,
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
    v.position = CalPoint4(pos);
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    CHECK_EQUAL(CalAABox(pos, pos), csm.getBoundingVolume());
}

TEST(aabox_is_min_max_of_all_vertices) {
    CalCoreSubmesh csm(3, 0, 0);

    CalCoreSubmesh::Vertex v;
    v.position = CalPoint4(CalVector(1.0f, 2.0f, 3.0f));
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    v.position = CalPoint4(CalVector(2.0f, 3.0f, 1.0f));
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    v.position = CalPoint4(CalVector(3.0f, 1.0f, 2.0f));
    csm.addVertex(v, CalColor32(), std::vector<CalCoreSubmesh::Influence>());

    CHECK_EQUAL(
        CalAABox(CalVector(1.0f, 1.0f, 1.0f), CalVector(3.0f, 3.0f, 3.0f)),
        csm.getBoundingVolume());
}

TEST(face_constructors) {
    CalCoreSubmesh::Face f2(20, 21, 22);    
    CHECK_EQUAL(f2.vertexId[0],20);
    CHECK_EQUAL(f2.vertexId[1],21);
    CHECK_EQUAL(f2.vertexId[2],22);  
}

TEST(make_cube) {
    CalCoreSubmeshPtr submeshPtr = MakeCube();
    const CalCoreSubmesh::VectorVertex &vertices =  submeshPtr->getVectorVertex();
    CHECK_EQUAL(submeshPtr->getVertexCount(), static_cast<size_t>(24));
    CHECK_EQUAL(CalVector(0,0,1), (vertices[0].position.asCalVector()));   
    CHECK_EQUAL(CalVector(1,0,1), (vertices[23].position.asCalVector()));
    CHECK_EQUAL(CalVector(0,0,1), (vertices[0].normal.asCalVector()));
    CHECK_EQUAL(CalVector(0,-1,0), (vertices[23].normal.asCalVector()));
    CHECK_EQUAL(submeshPtr->getFaces()[0], CalCoreSubmesh::Face(0,1,2));
    CHECK_EQUAL(submeshPtr->getFaces()[11], CalCoreSubmesh::Face(20, 23, 21));
}


TEST(applyZUpToYUp_mesh) {
    //make a cube submesh
    CalCoreSubmeshPtr submeshPtr = MakeCube();    
    //put some morph target data into it
    CalCoreMorphTarget::VertexOffsetArray vertexOffsets;
    VertexOffset mv;
    mv.vertexId = 0;
    mv.position = CalVector4(0, 1, 2, 0);
    mv.normal = CalVector4(0, -1, -2, 0);
    vertexOffsets.push_back(mv);
    CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget("m", 1, vertexOffsets));
    submeshPtr->addMorphTarget(morphTarget);

    //make a mesh
    CalCoreMesh cm;
    cm.submeshes.push_back(submeshPtr);




    //get some data for checking    
    const CalCoreSubmesh::VectorVertex &vertices =  submeshPtr->getVectorVertex();
    CalVector firstVertexPos = vertices[0].position.asCalVector();        
    CalVector lastVertexPos = vertices[vertices.size()-1].position.asCalVector();    
    CalVector firstVertexNormal = vertices[0].normal.asCalVector();        
    CalVector lastVertexNormal = vertices[vertices.size()-1].normal.asCalVector();
    CalVector morphTargetVertexOffset_Pos = mv.position.asCalVector();
    CalVector morphTargetVertexOffset_Normal = mv.normal.asCalVector();
    CalAABox bbox = submeshPtr->getBoundingVolume();

    cm.applyZUpToYUp();

    cal3d::applyZUpToYUp(firstVertexPos);
    cal3d::applyZUpToYUp(lastVertexPos);
    cal3d::applyZUpToYUp(firstVertexNormal);
    cal3d::applyZUpToYUp(lastVertexNormal);
    cal3d::applyZUpToYUp(morphTargetVertexOffset_Pos);
    cal3d::applyZUpToYUp(morphTargetVertexOffset_Normal);
    cal3d::applyZUpToYUp(bbox.max);
    cal3d::applyZUpToYUp(bbox.min);

    {
        const CalCoreSubmeshPtr testSubmeshPtr = cm.submeshes[0];
        const CalCoreSubmesh::VectorVertex &testVertices =  testSubmeshPtr->getVectorVertex();
        CHECK_EQUAL(firstVertexPos, (testVertices[0].position.asCalVector()));   
        CHECK_EQUAL(lastVertexPos, (testVertices[testVertices.size()-1].position.asCalVector()));
        CHECK_EQUAL(firstVertexNormal, (testVertices[0].normal.asCalVector()));
        CHECK_EQUAL(lastVertexNormal, (testVertices[testVertices.size()-1].normal.asCalVector()));


        CalAABox testAabox = testSubmeshPtr->getBoundingVolume();
        CHECK_EQUAL(bbox.max, testAabox.max);

        const CalCoreSubmesh::MorphTargetArray& testMta = testSubmeshPtr->getMorphTargets();
        const CalCoreMorphTargetPtr& testMtp = testMta[0];
        const CalCoreMorphTarget::VertexOffsetArray& testVoses = testMtp->vertexOffsets;
        const VertexOffset& testVos = testVoses[0];


        CHECK_EQUAL(testVos.vertexId, static_cast<size_t>(0));
        CHECK_EQUAL(morphTargetVertexOffset_Pos, (testVos.position.asCalVector()));
        CHECK_EQUAL(morphTargetVertexOffset_Normal, (testVos.normal.asCalVector()));
    }
}

TEST(applyCoordinateTransform_mesh) {
    // Note that cal3d quaternions are "left-handed", so that the following positive rotation is, in fact, the clockwise
    // rotation needed to convert z-up coordinates to y-up.
    CalQuaternion zUpToYUp(0.70710678f, 0.0f, 0.0f, 0.70710678f);
    //make a cube submesh
    CalCoreSubmeshPtr submeshPtr = MakeCube();
    //put some morph target data into it
    CalCoreMorphTarget::VertexOffsetArray vertexOffsets;
    VertexOffset mv;
    mv.vertexId = 0;
    mv.position = CalVector4(0, 1, 2, 0);
    mv.normal = CalVector4(0, -1, -2, 0);
    vertexOffsets.push_back(mv);
    CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget("m", 1, vertexOffsets));
    submeshPtr->addMorphTarget(morphTarget);

    //make a mesh
    CalCoreMesh cm;
    cm.submeshes.push_back(submeshPtr);




    //get some data for checking
    const CalCoreSubmesh::VectorVertex &vertices =  submeshPtr->getVectorVertex();
    CalVector firstVertexPos = vertices[0].position.asCalVector();
    CalVector lastVertexPos = vertices[vertices.size()-1].position.asCalVector();
    CalVector firstVertexNormal = vertices[0].normal.asCalVector();
    CalVector lastVertexNormal = vertices[vertices.size()-1].normal.asCalVector();
    CalVector morphTargetVertexOffset_Pos = mv.position.asCalVector();
    CalVector morphTargetVertexOffset_Normal = mv.normal.asCalVector();
    CalAABox bbox = submeshPtr->getBoundingVolume();

    cm.applyZUpToYUp();

    //
    cal3d::applyZUpToYUp(firstVertexPos);
    cal3d::applyZUpToYUp(lastVertexPos);
    cal3d::applyZUpToYUp(firstVertexNormal);
    cal3d::applyZUpToYUp(lastVertexNormal);
    cal3d::applyZUpToYUp(morphTargetVertexOffset_Pos);
    cal3d::applyZUpToYUp(morphTargetVertexOffset_Normal);
    cal3d::applyZUpToYUp(bbox.max);
    cal3d::applyZUpToYUp(bbox.min);

    {
        const CalCoreSubmeshPtr testSubmeshPtr = cm.submeshes[0];
        const CalCoreSubmesh::VectorVertex &testVertices =  testSubmeshPtr->getVectorVertex();
        CHECK_EQUAL(firstVertexPos, (testVertices[0].position.asCalVector()));
        CHECK_EQUAL(lastVertexPos, (testVertices[testVertices.size()-1].position.asCalVector()));
        CHECK_EQUAL(firstVertexNormal, (testVertices[0].normal.asCalVector()));
        CHECK_EQUAL(lastVertexNormal, (testVertices[testVertices.size()-1].normal.asCalVector()));


        CalAABox testAabox = testSubmeshPtr->getBoundingVolume();
        CHECK_EQUAL(bbox.max, testAabox.max);

        const CalCoreSubmesh::MorphTargetArray& testMta = testSubmeshPtr->getMorphTargets();
        const CalCoreMorphTargetPtr& testMtp = testMta[0];
        const CalCoreMorphTarget::VertexOffsetArray& testVoses = testMtp->vertexOffsets;
        const VertexOffset& testVos = testVoses[0];


        CHECK_EQUAL(testVos.vertexId, static_cast<size_t>(0));
        CHECK_EQUAL(morphTargetVertexOffset_Pos, (testVos.position.asCalVector()));
        CHECK_EQUAL(morphTargetVertexOffset_Normal, (testVos.normal.asCalVector()));
    }
}

TEST(minimumVertexBufferSize_starts_at_0) {
    CalCoreSubmesh csm(1, false, 1);
    CHECK_EQUAL(0u, csm.getMinimumVertexBufferSize());
}

TEST(minimumVertexBufferSize_emcompasses_face_range) {
    CalCoreSubmesh csm(1, false, 1);
    csm.addFace(CalCoreSubmesh::Face(1, 3, 2));
    CHECK_EQUAL(4u, csm.getMinimumVertexBufferSize());
}
