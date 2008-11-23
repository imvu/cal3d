#include "TestPrologue.h"
#include <cal3d/renderer.h>
#include <cal3d/model.h>
#include <cal3d/submesh.h>
#include <cal3d/physique.h>

template <class T> void caldestroy(T* ptr) {
    ptr->destroy();
    delete ptr;
}

CalCoreSubmesh::Vertex createVertex(float x, float y, float z, float nx, float ny, float nz, int numInfluences) {    
    CalCoreSubmesh::Vertex myVertex;
    myVertex.position = CalVector(x, y, z);
    myVertex.normal = CalVector(nx, ny, nz);

    CalCoreSubmesh::Influence myInfluence;
    for (int i = 0; i < numInfluences; i++) {
        myInfluence.boneId = i;
        myInfluence.weight = 1.0f;
        myVertex.vectorInfluence.push_back(myInfluence);
    }

    return myVertex;
}

CalCoreSkeleton * createTestCoreSkeleton(int numBones) {
    CalCoreSkeleton * coreSkeleton = new CalCoreSkeleton();

    for (int i = 0; i < numBones; i++) {
        CalCoreBone * coreBone = new CalCoreBone();
        char buf[32];
        _snprintf_s(buf, 32, _TRUNCATE, "bone%d", i);
        coreBone->createWithName(buf);
        coreSkeleton->addCoreBone(coreBone);
    }

    return coreSkeleton;
}

CalCoreMesh * createTestCoreMesh() {
    CalCoreSubmesh * coreSubMesh = new CalCoreSubmesh();
    
    CalCoreMesh * coreMesh = new CalCoreMesh();
    coreMesh->addCoreSubmesh(coreSubMesh);
    return coreMesh;
}

shared_ptr<CalModel> createTestModel(shared_ptr<CalCoreModel> coreModel) {
    shared_ptr<CalModel> model(new CalModel(), &caldestroy<CalModel>);
    model->create(coreModel.get());
    CHECK(model->attachMesh(0));

    // This sets up the transforms for the skeleton's bones so that we get vertex data out of the pipeline
    model->getMixer()->updateAnimation(0.0f);
    model->getMixer()->updateSkeleton();
    // std::vector<CalMesh *>::iterator iteratorVectorMesh = m_vectorMesh.begin();
    // m_pMorpher->update(...);
    //m_pMorphTargetMixer->update(deltaTime);
    //m_pPhysique->update();
    //m_pSpringSystem->update(deltaTime);
    return model;
}

TEST(getVerticesAndNormals_on_mesh_with_one_bone_generates_vertices) {
    CalCoreMesh * coreMesh = createTestCoreMesh();
    CalCoreSubmesh * coreSubMesh = coreMesh->getCoreSubmesh(0);
    coreSubMesh->reserve(1, 0, 1);
    coreSubMesh->setVertex(0, createVertex(1, 2, 3, 0, 1, 0, 1), CalMakeColor(CalVector(1, 1, 1)));

    shared_ptr<CalCoreModel> coreModel(new CalCoreModel(), &caldestroy<CalCoreModel>);
    coreModel->createWithName("test");
    coreModel->addCoreMesh(coreMesh);
    coreModel->setCoreSkeleton(createTestCoreSkeleton(1)); // No coreSkeleton == model->create() returns false

    shared_ptr<CalModel> model(createTestModel(coreModel));

    CalSubmesh* sm = model->getMesh(0)->getSubmesh(0);
    int numVertices = sm->getVertexCount() * 6;
    CHECK_EQUAL(numVertices, 6);
    scoped_array<float> vertexBuffer(new float[numVertices]);

    int numResultVertices = CalPhysique::calculateVerticesAndNormals(model.get(), sm, vertexBuffer.get());
    CHECK_EQUAL(numResultVertices, 1);
    CHECK_EQUAL(vertexBuffer[0], 1);
    CHECK_EQUAL(vertexBuffer[1], 2);
    CHECK_EQUAL(vertexBuffer[2], 3);
    CHECK_EQUAL(vertexBuffer[3], 0);
    CHECK_EQUAL(vertexBuffer[4], 1);
    CHECK_EQUAL(vertexBuffer[5], 0);
}

TEST(getVerticesAndNormals_on_mesh_with_two_bones_generates_normals_that_are_not_unit_vectors) {
    CalCoreMesh * coreMesh = createTestCoreMesh();
    CalCoreSubmesh * coreSubMesh = coreMesh->getCoreSubmesh(0);
    coreSubMesh->reserve(1, 0, 1);
    coreSubMesh->setVertex(0, createVertex(1, 2, 3, 1, 1, 0, 2), CalMakeColor(CalVector(1, 1, 1)));

    shared_ptr<CalCoreModel> coreModel(new CalCoreModel(), &caldestroy<CalCoreModel>);
    coreModel->createWithName("test");
    coreModel->addCoreMesh(coreMesh);
    coreModel->setCoreSkeleton(createTestCoreSkeleton(2)); // No coreSkeleton == model->create() returns false

    shared_ptr<CalModel> model(createTestModel(coreModel));

    CalSubmesh* sm = model->getMesh(0)->getSubmesh(0);
    int numVertices = sm->getVertexCount() * 6;
    scoped_array<float> vertexBuffer(new float[numVertices]);

    int numResultVertices = CalPhysique::calculateVerticesAndNormals(model.get(), sm, vertexBuffer.get());
    CHECK_EQUAL(numResultVertices, 1);
    CHECK_CLOSE(2.82842708, sqrt(pow(vertexBuffer[3], 2) + pow(vertexBuffer[4], 2) + pow(vertexBuffer[5], 2)), 0.0001f);
}

