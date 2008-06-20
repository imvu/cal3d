#include "TestPrologue.h"

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
    coreSubMesh->create();
    
    CalCoreMesh * coreMesh = new CalCoreMesh();
    coreMesh->create();
    coreMesh->addCoreSubmesh(coreSubMesh);
    return coreMesh;
}

shared_ptr<CalModel> createTestModel(shared_ptr<CalCoreModel> coreModel) {
    shared_ptr<CalModel> model(new CalModel(), &caldestroy<CalModel>);
    model->create(coreModel.get());
    CHECK(model->attachMesh(0));
    model->update(0.0f); // This sets up the transforms for the skeleton's bones so that we get vertex data out of the pipeline
    return model;
}

TEST(getVerticesAndNormals_on_mesh_with_one_bone_generates_vertices) {
    CalCoreMesh * coreMesh = createTestCoreMesh();
    CalCoreSubmesh * coreSubMesh = coreMesh->getCoreSubmesh(0);
    coreSubMesh->reserve(1, 0, 1, 0);
    coreSubMesh->setVertex(0, createVertex(1, 2, 3, 0, 1, 0, 1));

    shared_ptr<CalCoreModel> coreModel(new CalCoreModel(), &caldestroy<CalCoreModel>);
    coreModel->createWithName("test");
    coreModel->addCoreMesh(coreMesh);
    coreModel->setCoreSkeleton(createTestCoreSkeleton(1)); // No coreSkeleton == model->create() returns false

    shared_ptr<CalModel> model(createTestModel(coreModel));
    CalRenderer * r = model->getRenderer();

    CHECK(r->beginRendering());
    int numVertices = r->getVertexCount() * 6;
    CHECK_EQUAL(numVertices, 6);
    scoped_array<float> vertexBuffer(new float[numVertices]);

    int numResultVertices = r->getVerticesAndNormals(vertexBuffer.get());
    CHECK_EQUAL(numResultVertices, 1);
    CHECK_EQUAL(vertexBuffer[0], 1);
    CHECK_EQUAL(vertexBuffer[1], 2);
    CHECK_EQUAL(vertexBuffer[2], 3);
    CHECK_EQUAL(vertexBuffer[3], 0);
    CHECK_EQUAL(vertexBuffer[4], 1);
    CHECK_EQUAL(vertexBuffer[5], 0);

    r->endRendering();
}

TEST(getVerticesAndNormals_on_mesh_with_two_bones_generates_normals_that_are_unit_vectors) {
    CalCoreMesh * coreMesh = createTestCoreMesh();
    CalCoreSubmesh * coreSubMesh = coreMesh->getCoreSubmesh(0);
    coreSubMesh->reserve(1, 0, 1, 0);
    coreSubMesh->setVertex(0, createVertex(1, 2, 3, 1, 1, 0, 2));

    shared_ptr<CalCoreModel> coreModel(new CalCoreModel(), &caldestroy<CalCoreModel>);
    coreModel->createWithName("test");
    coreModel->addCoreMesh(coreMesh);
    coreModel->setCoreSkeleton(createTestCoreSkeleton(2)); // No coreSkeleton == model->create() returns false

    shared_ptr<CalModel> model(createTestModel(coreModel));
    CalRenderer * r = model->getRenderer();

    CHECK(r->beginRendering());
    int numVertices = r->getVertexCount() * 6;
    scoped_array<float> vertexBuffer(new float[numVertices]);

    int numResultVertices = r->getVerticesAndNormals(vertexBuffer.get());
    CHECK_EQUAL(numResultVertices, 1);
    CHECK_CLOSE(1.0f, sqrt(pow(vertexBuffer[3], 2) + pow(vertexBuffer[4], 2) + pow(vertexBuffer[5], 2)), 0.0001f);

    r->endRendering();
}

