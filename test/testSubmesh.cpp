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

TEST(duplicate_triangles) {
    CalCoreSubmesh csm(3, 0, 1);
    CalCoreSubmesh::Vertex vertex;
    vertex.position = CalVector4(1, 1, 1, 0);
    vertex.normal = CalVector4(-1, -1 -1, 0);
    CalCoreSubmesh::InfluenceVector iv(1);
    csm.addVertex(vertex, 0, iv);
    csm.addVertex(vertex, 0, iv);
    csm.addVertex(vertex, 0, iv);

    csm.addFace(CalCoreSubmesh::Face(0, 1, 2));

    csm.duplicateTriangles();
    CHECK_EQUAL(2u, csm.getFaceCount());
    CHECK_EQUAL(CalCoreSubmesh::Face(0, 2, 1), csm.getFaces()[1]);
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

TEST(make_cube_validate_submesh) {
    CalCoreSubmeshPtr submeshPtr = MakeCube();
    CHECK_EQUAL(submeshPtr->validateSubmesh(), true);
}

TEST(validate_submesh_is_failed) {
    CalCoreSubmeshPtr cube(new CalCoreSubmesh(4, true, 2));
    
    const CalColor32 black = 0;

        
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;

    int curVertexId=0;
    CalCoreSubmesh::TextureCoordinate texCoord;
    CalCoreSubmesh::Vertex vertex;
    //triangle face f0, f1 vertices
    //v0
    curVertexId = 0;
    vertex.position = CalPoint4(0, 0, 1); 
    vertex.normal = CalVector4(0, 0, 1); 
    cube->addVertex(vertex, black, inf);    
    texCoord.u = 0.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId, texCoord);
    //v1
    ++curVertexId;
    vertex.position = CalPoint4(1,1,1);
    cube->addVertex(vertex, black, inf);    
    texCoord.u = 1.0f;
    texCoord.v = 1.0f;    
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v2
    ++curVertexId;
    vertex.position = CalPoint4(0,1,1);
    cube->addVertex(vertex, black, inf);   
    texCoord.u = 0.0f;
    texCoord.v = 1.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);
    //v3
    ++curVertexId;
    vertex.position = CalPoint4(1,0,1);
    cube->addVertex(vertex, black, inf);
    texCoord.u = 1.0f;
    texCoord.v = 0.0f;
    cube->setTextureCoordinate(curVertexId,  texCoord);

    cube->addFace(CalCoreSubmesh::Face(0, 1, 5));
    cube->addFace(CalCoreSubmesh::Face(0, 3, 1));
    CHECK_EQUAL(cube->validateSubmesh(), false);
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

CalCoreSubmesh::Vertex makeVertex(int id) {
    CalCoreSubmesh::Vertex v;
    v.position = CalPoint4(id, id, id);
    v.normal = CalVector4(id, id, id);
    return v;
}

TEST(renumber_renumberIndices_without_texcoords) {
    CalCoreSubmesh csm(4, false, 2);
    csm.addFace(CalCoreSubmesh::Face(3, 2, 1));
    csm.addFace(CalCoreSubmesh::Face(2, 0, 1));

    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].weight = 1.0f;

    inf[0].boneId = 0;
    csm.addVertex(makeVertex(0), 0, inf);
    inf[0].boneId = 1;
    csm.addVertex(makeVertex(1), 1, inf);
    inf[0].boneId = 2;
    csm.addVertex(makeVertex(2), 2, inf);
    inf[0].boneId = 3;
    csm.addVertex(makeVertex(3), 3, inf);

    csm.renumberIndices();

    CHECK_EQUAL(CalCoreSubmesh::Face(0, 1, 2), csm.getFaces()[0]);
    CHECK_EQUAL(CalCoreSubmesh::Face(1, 3, 2), csm.getFaces()[1]);

    CHECK_EQUAL(makeVertex(3), csm.getVectorVertex()[0]);
    CHECK_EQUAL(makeVertex(2), csm.getVectorVertex()[1]);
    CHECK_EQUAL(makeVertex(1), csm.getVectorVertex()[2]);
    CHECK_EQUAL(makeVertex(0), csm.getVectorVertex()[3]);

    CHECK_EQUAL(CalColor32(3), csm.getVertexColors()[0]);
    CHECK_EQUAL(CalColor32(2), csm.getVertexColors()[1]);
    CHECK_EQUAL(CalColor32(1), csm.getVertexColors()[2]);
    CHECK_EQUAL(CalColor32(0), csm.getVertexColors()[3]);

    CHECK_EQUAL(3u, csm.getInfluences()[0].boneId);
    CHECK_EQUAL(2u, csm.getInfluences()[1].boneId);
    CHECK_EQUAL(1u, csm.getInfluences()[2].boneId);
    CHECK_EQUAL(0u, csm.getInfluences()[3].boneId);
}

CalCoreSubmesh::TextureCoordinate makeTextureCoordinate(int i) {
    return CalCoreSubmesh::TextureCoordinate(i, i);
}

TEST(renumber_vertices_with_texcoords) {
    CalCoreSubmesh csm(4, true, 2);
    csm.addFace(CalCoreSubmesh::Face(3, 2, 1));
    csm.addFace(CalCoreSubmesh::Face(2, 0, 1));

    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].weight = 1.0f;
    inf[0].boneId = 0;

    csm.addVertex(makeVertex(0), 0, inf);
    csm.setTextureCoordinate(0, makeTextureCoordinate(0));
    csm.addVertex(makeVertex(1), 0, inf);
    csm.setTextureCoordinate(1, makeTextureCoordinate(1));
    csm.addVertex(makeVertex(2), 0, inf);
    csm.setTextureCoordinate(2, makeTextureCoordinate(2));
    csm.addVertex(makeVertex(3), 0, inf);
    csm.setTextureCoordinate(3, makeTextureCoordinate(3));

    csm.renumberIndices();

    CHECK_EQUAL(makeTextureCoordinate(3), csm.getTextureCoordinates()[0]);
    CHECK_EQUAL(makeTextureCoordinate(2), csm.getTextureCoordinates()[1]);
    CHECK_EQUAL(makeTextureCoordinate(1), csm.getTextureCoordinates()[2]);
    CHECK_EQUAL(makeTextureCoordinate(0), csm.getTextureCoordinates()[3]);
}

TEST(renumber_morphs) {
    CalCoreSubmesh csm(3, false, 1);
    csm.addFace(CalCoreSubmesh::Face(2, 0, 1));

    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].weight = 1.0f;

    inf[0].boneId = 0;
    csm.addVertex(makeVertex(0), 0, inf);
    inf[0].boneId = 1;
    csm.addVertex(makeVertex(1), 1, inf);
    inf[0].boneId = 2;
    csm.addVertex(makeVertex(2), 2, inf);

    CalCoreMorphTarget::VertexOffsetArray offsets;
    offsets.push_back(VertexOffset(2, CalPoint4(), CalVector4()));
    csm.addMorphTarget(CalCoreMorphTargetPtr(new CalCoreMorphTarget("morph", 3, offsets)));

    csm.renumberIndices();

    CHECK_EQUAL(CalCoreSubmesh::Face(0, 1, 2), csm.getFaces()[0]);
    CHECK_EQUAL(0u, csm.getMorphTargets()[0]->vertexOffsets[0].vertexId);
}

TEST(drop_morph_offsets_for_unused_vertices) {
    CalCoreSubmesh csm(4, false, 1);
    csm.addFace(CalCoreSubmesh::Face(2, 0, 1));

    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;

    csm.addVertex(makeVertex(0), 0, inf);
    csm.addVertex(makeVertex(1), 1, inf);
    csm.addVertex(makeVertex(2), 2, inf);
    csm.addVertex(makeVertex(3), 3, inf);

    CalCoreMorphTarget::VertexOffsetArray offsets;
    offsets.push_back(VertexOffset(3, CalPoint4(), CalVector4()));
    csm.addMorphTarget(CalCoreMorphTargetPtr(new CalCoreMorphTarget("morph", csm.getVertexCount(), offsets)));

    csm.renumberIndices();

    CHECK_EQUAL(CalCoreSubmesh::Face(0, 1, 2), csm.getFaces()[0]);
    CHECK_EQUAL(3u, csm.getVectorVertex().size());
    CHECK_EQUAL(0u, csm.getMorphTargets()[0]->vertexOffsets.size());
}

TEST(minimumVertexBufferSize_is_changed_if_vertex_list_is_shrunk) {
    CalCoreSubmesh csm(4, false, 1);
    csm.addFace(CalCoreSubmesh::Face(1, 2, 3));
    CHECK_EQUAL(4u, csm.getMinimumVertexBufferSize());

    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;

    csm.addVertex(makeVertex(0), 0, inf);
    csm.addVertex(makeVertex(1), 1, inf);
    csm.addVertex(makeVertex(2), 2, inf);
    csm.addVertex(makeVertex(3), 3, inf);

    csm.renumberIndices();

    CHECK_EQUAL(CalCoreSubmesh::Face(0, 1, 2), csm.getFaces()[0]);
    CHECK_EQUAL(3u, csm.getVectorVertex().size());
    CHECK_EQUAL(3u, csm.getMinimumVertexBufferSize());
}

static void s_checkNormalizedNormals(CalVector4 exp, CalVector4 in) {
    CalCoreSubmesh csm(3, 0, 1);
    CalCoreSubmesh::Vertex vertex;
    vertex.position = CalPoint4(1, 1, 1, 0);
    vertex.normal = in;
    CalCoreSubmesh::InfluenceVector iv(1);
    csm.addVertex(vertex, 0, iv);
    csm.addVertex(vertex, 0, iv);
    csm.addVertex(vertex, 0, iv);

    csm.normalizeNormals();
    const CalCoreSubmesh::VectorVertex& vertices =  csm.getVectorVertex();
    CHECK_EQUAL(3u, vertices.size());
    const CalVector4& normal = vertices[0].normal; 
    CHECK_CLOSE(exp.x, normal.x, 0.001);
    CHECK_CLOSE(exp.y, normal.y, 0.001);
    CHECK_CLOSE(exp.z, normal.z, 0.001);
}

TEST(can_normalize_bad_normals) {
    s_checkNormalizedNormals(
        CalVector4(0.707106781187f, -0.707106781187f, 0.0f, 0.0f),
        CalVector4(1000.0f, -1000.0f, 0.0f, 0.0f)
    );
}

TEST(returns_a_default_when_asked_to_normalize_zero_normals) {
    s_checkNormalizedNormals(
        CalVector4(0.0f, 1.0f, 0.0f, 0.0f),
        CalVector4(0.0f, 0.0f, 0.0f, 0.0f)
    );
}

TEST(returns_a_default_when_asked_to_normalize_inf_normals) {
    float inf = 1.0f / 0.0f;
    s_checkNormalizedNormals(
        CalVector4(0.0f, 1.0f, 0.0f, 0.0f),
        CalVector4(inf, 1.0f, 0.0f, 0.0f)
    );
}

TEST(returns_a_default_when_asked_to_normalize_negative_inf_normals) {
    float inf = 1.0f / 0.0f;
    s_checkNormalizedNormals(
        CalVector4(0.0f, 1.0f, 0.0f, 0.0f),
        CalVector4(-inf, 1.0f, 0.0f, 0.0f)
    );
}

TEST(returns_a_default_when_asked_to_normalize_nan_normals) {
    float nan = 0.0f / 0.0f;
    s_checkNormalizedNormals(
        CalVector4(0.0f, 1.0f, 0.0f, 0.0f),
        CalVector4(nan, 1.0f, 0.0f, 0.0f)
    );
}
