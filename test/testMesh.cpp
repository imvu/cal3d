#include "TestPrologue.h"
#include <cal3d/coremesh.h>
#include <cal3d/coresubmesh.h>
#include <cal3d/coremorphtarget.h>
#include <cal3d/streamops.h>

TEST(can_replace_base_mesh_with_morph_target) {
    CalCoreSubmesh::Face face(0, 0, 0);

    CalCoreSubmesh::Vertex vertex;
    vertex.position = CalPoint4(1, 1, 1);
    vertex.normal = CalVector4(1, 1, 1);

    CalCoreSubmesh::Influence influence;
    influence.boneId = 0;
    influence.weight = 1.0f;
    influence.lastInfluenceForThisVertex = 1;

    std::vector<CalCoreSubmesh::Influence> influences;
    influences.push_back(influence);

    CalCoreMesh mesh;
    CalCoreSubmeshPtr submesh(new CalCoreSubmesh(1, false, 1));
    submesh->addFace(face);
    submesh->addVertex(vertex, CalColor32(), influences);

    VertexOffset aVO;
    aVO.vertexId = 0;
    aVO.position = CalVector4(2, 2, 2);
    aVO.normal = CalVector4(2, 2, 2);

    CalCoreMorphTarget::VertexOffsetArray aVertexOffsets;
    aVertexOffsets.push_back(aVO);
    CalCoreMorphTargetPtr aTarget(new CalCoreMorphTarget("A", 1, aVertexOffsets));

    VertexOffset bVO;
    bVO.vertexId = 0;
    bVO.position = CalVector4(3, 3, 3);
    bVO.normal = CalVector4(3, 3, 3);

    CalCoreMorphTarget::VertexOffsetArray bVertexOffsets;
    bVertexOffsets.push_back(bVO);
    CalCoreMorphTargetPtr bTarget(new CalCoreMorphTarget("B", 1, bVertexOffsets));

    submesh->addMorphTarget(aTarget);
    submesh->addMorphTarget(bTarget);

    mesh.submeshes.push_back(submesh);
    
    CHECK_EQUAL(CalPoint4(1, 1, 1), submesh->getVectorVertex()[0].position);
    CHECK_EQUAL(CalVector4(1, 1, 1), submesh->getVectorVertex()[0].normal);

    mesh.replaceMeshWithMorphTarget("b"); // testing case insensitivity?
    CHECK_EQUAL(CalPoint4(1, 1, 1), submesh->getVectorVertex()[0].position);
    CHECK_EQUAL(CalVector4(1, 1, 1), submesh->getVectorVertex()[0].normal);

    mesh.replaceMeshWithMorphTarget("B"); // testing case insensitivity?
    CHECK_EQUAL(CalPoint4(4, 4, 4), submesh->getVectorVertex()[0].position);
    CHECK_EQUAL(CalVector4(4, 4, 4), submesh->getVectorVertex()[0].normal);
}
