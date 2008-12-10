#include <intrin.h>
#include "TestPrologue.h"
#include <cal3d/renderer.h>
#include <cal3d/bone.h>
#include <cal3d/model.h>
#include <cal3d/submesh.h>
#include <cal3d/skeleton.h>
#include <cal3d/physique.h>
#include <cal3d/mixer.h>

template <class T> void caldestroy(T* ptr) {
    ptr->destroy();
    delete ptr;
}

void setVertex(CalCoreSubmesh* submesh, unsigned vertexId, float x, float y, float z, float nx, float ny, float nz, int numInfluences, CalColor32 color, const CalCoreSubmesh::LodData& ld) {
    CalCoreSubmesh::Vertex myVertex;
    myVertex.position.setAsPoint(CalVector(x, y, z));
    myVertex.normal.setAsVector(CalVector(nx, ny, nz));

    std::vector<CalCoreSubmesh::Influence> influences;

    for (int i = 0; i < numInfluences; i++) {
        CalCoreSubmesh::Influence myInfluence;
        myInfluence.boneId = i;
        myInfluence.weight = 1.0f / numInfluences;
        influences.push_back(myInfluence);
    }

    submesh->setVertex(vertexId, myVertex, color, ld, influences);
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
    CalCoreSubmesh::LodData ld = {0, 0};
    setVertex(coreSubMesh, 0, 1, 2, 3, 0, 1, 0, 1, CalMakeColor(CalVector(1, 1, 1)), ld);

    shared_ptr<CalCoreModel> coreModel(new CalCoreModel(), &caldestroy<CalCoreModel>);
    coreModel->createWithName("test");
    coreModel->addCoreMesh(coreMesh);
    coreModel->setCoreSkeleton(createTestCoreSkeleton(1)); // No coreSkeleton == model->create() returns false

    shared_ptr<CalModel> model(createTestModel(coreModel));

    CalSubmesh* sm = model->getMesh(0)->getSubmesh(0);
    CHECK_EQUAL(1, sm->getVertexCount());
    float vertexBuffer[8];

    CalPhysique::calculateVerticesAndNormals(model->getSkeleton()->boneTransforms.data, sm, vertexBuffer);
    CHECK_EQUAL(vertexBuffer[0], 1);
    CHECK_EQUAL(vertexBuffer[1], 2);
    CHECK_EQUAL(vertexBuffer[2], 3);
    CHECK_EQUAL(vertexBuffer[4], 0);
    CHECK_EQUAL(vertexBuffer[5], 1);
    CHECK_EQUAL(vertexBuffer[6], 0);
}

TEST(getVerticesAndNormals_on_mesh_with_two_translation_bones) {
    CalCoreMesh * coreMesh = createTestCoreMesh();
    CalCoreSubmesh * coreSubMesh = coreMesh->getCoreSubmesh(0);
    coreSubMesh->reserve(1, 0, 1);
    CalCoreSubmesh::LodData ld = {0, 0};
    setVertex(coreSubMesh, 0, 1, 2, 3, 1, 1, 0, 2, CalMakeColor(CalVector(1, 1, 1)), ld);

    CalCoreBone* coreBone1 = new CalCoreBone;
    coreBone1->createWithName("bone1");
    coreBone1->setTranslation(CalVector(1, 0, 0));
    coreBone1->setTranslationBoneSpace(CalVector(-1, 0, 0));

    CalCoreBone* coreBone2 = new CalCoreBone;
    coreBone2->createWithName("bone2");
    coreBone1->setTranslation(CalVector(0, 1, 0));
    coreBone1->setTranslationBoneSpace(CalVector(0, -1, 0));

    CalCoreSkeleton* coreSkeleton = new CalCoreSkeleton;
    coreSkeleton->addCoreBone(coreBone1);
    coreSkeleton->addCoreBone(coreBone2);

    shared_ptr<CalCoreModel> coreModel(new CalCoreModel(), &caldestroy<CalCoreModel>);
    coreModel->createWithName("test");
    coreModel->addCoreMesh(coreMesh);
    coreModel->setCoreSkeleton(createTestCoreSkeleton(2)); // No coreSkeleton == model->create() returns false

    shared_ptr<CalModel> model(new CalModel(), &caldestroy<CalModel>);
    model->create(coreModel.get());
    CHECK(model->attachMesh(0));

    model->getMixer()->updateAnimation(0.0f);
    model->getMixer()->updateSkeleton();

    CalSubmesh* sm = model->getMesh(0)->getSubmesh(0);
    CHECK_EQUAL(1, sm->getVertexCount());
    float vertexBuffer[8];
    CalPhysique::calculateVerticesAndNormals(
      model->getSkeleton()->boneTransforms.data,
      sm,
      vertexBuffer);

    CHECK_EQUAL(1.0f, vertexBuffer[0]);
    CHECK_EQUAL(2.0f, vertexBuffer[1]);
    CHECK_EQUAL(3.0f, vertexBuffer[2]);

    CHECK_EQUAL(1.0f, vertexBuffer[4]);
    CHECK_EQUAL(1.0f, vertexBuffer[5]);
    CHECK_EQUAL(0.0f, vertexBuffer[6]);
}

TEST(getVerticesAndNormals_on_mesh_with_two_translated_bones) {
    CalCoreMesh * coreMesh = createTestCoreMesh();
    CalCoreSubmesh * coreSubMesh = coreMesh->getCoreSubmesh(0);
    coreSubMesh->reserve(1, 0, 1);
    CalCoreSubmesh::LodData ld = {0, 0};
    setVertex(coreSubMesh, 0, 1, 2, 3, 1, 1, 0, 2, CalMakeColor(CalVector(1, 1, 1)), ld);

    CalCoreBone* coreBone1 = new CalCoreBone;
    coreBone1->createWithName("bone1");
    coreBone1->setTranslation(CalVector(2, 0, 0));
    coreBone1->setTranslationBoneSpace(CalVector(-1, 0, 0));

    CalCoreBone* coreBone2 = new CalCoreBone;
    coreBone2->createWithName("bone2");
    coreBone1->setTranslation(CalVector(0, 2, 0));
    coreBone1->setTranslationBoneSpace(CalVector(0, -1, 0));

    CalCoreSkeleton* coreSkeleton = new CalCoreSkeleton;
    coreSkeleton->addCoreBone(coreBone1);
    coreSkeleton->addCoreBone(coreBone2);

    shared_ptr<CalCoreModel> coreModel(new CalCoreModel(), &caldestroy<CalCoreModel>);
    coreModel->createWithName("test");
    coreModel->addCoreMesh(coreMesh);
    coreModel->setCoreSkeleton(createTestCoreSkeleton(2)); // No coreSkeleton == model->create() returns false

    shared_ptr<CalModel> model(new CalModel(), &caldestroy<CalModel>);
    model->create(coreModel.get());
    CHECK(model->attachMesh(0));

    model->getSkeleton()->getBone(0)->clearState();
    model->getSkeleton()->getBone(1)->clearState();
    model->getSkeleton()->getBone(0)->blendState(1.0f, CalVector(1, 0, 0), CalQuaternion(), 1.0f, true, 1.0f);
    model->getSkeleton()->getBone(1)->blendState(1.0f, CalVector(0, 1, 0), CalQuaternion(), 1.0f, true, 1.0f);
    model->getSkeleton()->lockState();
    model->getSkeleton()->calculateState();
    //model->getMixer()->updateSkeleton();

    CalSubmesh* sm = model->getMesh(0)->getSubmesh(0);
    CHECK_EQUAL(1, sm->getVertexCount());
    float vertexBuffer[8];
    CalPhysique::calculateVerticesAndNormals(
      model->getSkeleton()->boneTransforms.data,
      sm,
      vertexBuffer);

    CHECK_EQUAL(1.5f, vertexBuffer[0]);
    CHECK_EQUAL(2.5f, vertexBuffer[1]);
    CHECK_EQUAL(3.0f, vertexBuffer[2]);

    CHECK_EQUAL(1.0f, vertexBuffer[4]);
    CHECK_EQUAL(1.0f, vertexBuffer[5]);
    CHECK_EQUAL(0.0f, vertexBuffer[6]);
}

TEST(getVerticesAndNormals_on_mesh_with_three_translated_bones) {
    CalCoreSubmesh * coreSubMesh = new CalCoreSubmesh;
    coreSubMesh->reserve(1, 0, 1);
    CalCoreSubmesh::LodData ld = {0, 0};
    setVertex(coreSubMesh, 0, 1, 2, 3, 1, 1, 0, 3, CalMakeColor(CalVector(1, 1, 1)), ld);

    CalSkeleton::BoneTransform transforms[3];
    transforms[0].rowx.set(1, 0, 0, 1);
    transforms[0].rowy.set(0, 1, 0, 0);
    transforms[0].rowz.set(0, 0, 1, 0);

    transforms[1].rowx.set(1, 0, 0, 0);
    transforms[1].rowy.set(0, 1, 0, 1);
    transforms[1].rowz.set(0, 0, 1, 0);

    transforms[2].rowx.set(1, 0, 0, 0);
    transforms[2].rowy.set(0, 1, 0, 0);
    transforms[2].rowz.set(0, 0, 1, 1);

    CalSubmesh* sm = new CalSubmesh(coreSubMesh);
    CHECK_EQUAL(1, sm->getVertexCount());
    float vertexBuffer[8];
    CalPhysique::calculateVerticesAndNormals(
      transforms,
      sm,
      vertexBuffer);

    CHECK_EQUAL(4.0f / 3.0f,  vertexBuffer[0]);
    CHECK_EQUAL(7.0f / 3.0f,  vertexBuffer[1]);
    CHECK_EQUAL(10.0f / 3.0f, vertexBuffer[2]);

    CHECK_EQUAL(1.0f, vertexBuffer[4]);
    CHECK_EQUAL(1.0f, vertexBuffer[5]);
    CHECK_EQUAL(0.0f, vertexBuffer[6]);
}


TEST(two_rotated_bones) {
  CalSkeleton::BoneTransform bt[] = {
    { CalVector4(0, -1, 0, 0),
      CalVector4(1,  0, 0, 0),
      CalVector4(0,  0, 1, 0), },
    { CalVector4(1,  0, 0, 0),
      CalVector4(0,  0, 1, 0),
      CalVector4(0, -1, 0, 0), },
  };

  CalCoreSubmesh::Vertex v[] = {
    { CalPoint4(1, 1, 1), CalVector4(1, 1, 1) },
  };

  CalCoreSubmesh::Influence i[] = {
    CalCoreSubmesh::Influence(0, 0.5f, false),
    CalCoreSubmesh::Influence(1, 0.5f, true),
  };

  CalVector4 output[2];
  CalPhysique::calculateVerticesAndNormals_x87(bt, 1, v, i, output);
  CHECK_EQUAL(output[0].x, 0);
  CHECK_EQUAL(output[0].y, 1);
  CHECK_EQUAL(output[0].z, 0);
  CHECK_EQUAL(output[1].x, 0);
  CHECK_EQUAL(output[1].y, 1);
  CHECK_EQUAL(output[1].z, 0);
}


TEST(calculateVerticesAndNormals_10000_vertices_1_influence_cycle_count) {
  const int N = 10000;
  const int TrialCount = 10;
  
  CalCoreSubmesh::Influence influence;
  influence.boneId = 0;
  influence.weight = 1.0f;

  std::vector<CalCoreSubmesh::Influence> influences;
  influences.push_back(influence);

  CalCoreSubmesh* coreSubMesh = new CalCoreSubmesh;
  coreSubMesh->reserve(N, 0, 0);
  for (int i = 0; i < N; ++i) {
    CalCoreSubmesh::Vertex v;
    v.position.setAsPoint(CalVector(1.0f, 2.0f, 3.0f));
    v.normal.setAsVector(CalVector(0.0f, 0.0f, 1.0f));
    coreSubMesh->setVertex(i, v, 0, CalCoreSubmesh::LodData(), influences);
  }

  CalSubmesh* submesh = new CalSubmesh(coreSubMesh);

  CalSkeleton::BoneTransform bt;
  ZeroMemory(&bt, sizeof(bt));

  __declspec(align(16)) float output[N * 8];

  __int64 min = 99999999999999;
  for (int t = 0; t < TrialCount; ++t) {
    unsigned __int64 start = __rdtsc();
    CalPhysique::calculateVerticesAndNormals(&bt, submesh, &output[0]);
    unsigned __int64 end = __rdtsc();
    unsigned __int64 elapsed = end - start;
    if (elapsed < min) {
      min = elapsed;
    }
  }

  printf("Cycles per vertex: %d\n", (int)(min / N));
}
