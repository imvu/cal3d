#include <intrin.h>
#include "TestPrologue.h"
#include <cal3d/renderer.h>
#include <cal3d/bone.h>
#include <cal3d/model.h>
#include <cal3d/submesh.h>
#include <cal3d/skeleton.h>
#include <cal3d/physique.h>
#include <cal3d/mixer.h>

FIXTURE(skin_x87) {
  CalPhysique::SkinRoutine skin;
  SETUP(skin_x87) {
    skin = CalPhysique::calculateVerticesAndNormals_x87;
  }
};
FIXTURE(skin_SSE_intrinsics) {
  CalPhysique::SkinRoutine skin;
  SETUP(skin_SSE_intrinsics) {
    skin = CalPhysique::calculateVerticesAndNormals_SSE_intrinsics;
  }
};
FIXTURE(skin_SSE) {
  CalPhysique::SkinRoutine skin;
  SETUP(skin_SSE) {
    skin = CalPhysique::calculateVerticesAndNormals_SSE;
  }
};

#define APPLY_SKIN_FIXTURES(test)         \
  APPLY_TEST_F(skin_x87, test)            \
  APPLY_TEST_F(skin_SSE_intrinsics, test) \
  APPLY_TEST_F(skin_SSE, test)

ABSTRACT_TEST(single_identity_bone) {
  CalSkeleton::BoneTransform bt[] = {
    { CalVector4(1, 0, 0, 0),
      CalVector4(0, 1, 0, 0),
      CalVector4(0, 0, 1, 0), },
  };

  CalCoreSubmesh::Vertex v[] = {
    { CalPoint4(1, 2, 3), CalVector4(0, 1, 0) },
  };

  CalCoreSubmesh::Influence i[] = {
    CalCoreSubmesh::Influence(0, 1.0f, true),
  };

  CalVector4 output[2];
  skin(bt, 1, v, i, output);
  CHECK_EQUAL(output[0].x, 1);
  CHECK_EQUAL(output[0].y, 2);
  CHECK_EQUAL(output[0].z, 3);
  CHECK_EQUAL(output[1].x, 0);
  CHECK_EQUAL(output[1].y, 1);
  CHECK_EQUAL(output[1].z, 0);
}
APPLY_SKIN_FIXTURES(single_identity_bone);

ABSTRACT_TEST(two_translated_bones) {
  CalSkeleton::BoneTransform bt[] = {
    { CalVector4(1, 0, 0, 1),
      CalVector4(0, 1, 0, 0),
      CalVector4(0, 0, 1, 0), },
    { CalVector4(1, 0, 0, 0),
      CalVector4(0, 1, 0, 1),
      CalVector4(0, 0, 1, 0), },
  };

  CalCoreSubmesh::Vertex v[] = {
    { CalPoint4(1, 2, 3), CalVector4(1, 1, 0) },
  };

  CalCoreSubmesh::Influence i[] = {
    CalCoreSubmesh::Influence(0, 0.5f, false),
    CalCoreSubmesh::Influence(1, 0.5f, true),
  };

  CalVector4 output[2];
  skin(bt, 1, v, i, output);
  CHECK_EQUAL(output[0].x, 1.5);
  CHECK_EQUAL(output[0].y, 2.5);
  CHECK_EQUAL(output[0].z, 3);
  CHECK_EQUAL(output[1].x, 1);
  CHECK_EQUAL(output[1].y, 1);
  CHECK_EQUAL(output[1].z, 0);
}
APPLY_SKIN_FIXTURES(two_translated_bones);

ABSTRACT_TEST(three_translated_bones) {
  CalSkeleton::BoneTransform bt[] = {
    { CalVector4(1, 0, 0, 1),
      CalVector4(0, 1, 0, 0),
      CalVector4(0, 0, 1, 0), },
    { CalVector4(1, 0, 0, 0),
      CalVector4(0, 1, 0, 1),
      CalVector4(0, 0, 1, 0), },
    { CalVector4(1, 0, 0, 0),
      CalVector4(0, 1, 0, 0),
      CalVector4(0, 0, 1, 1), },
  };

  CalCoreSubmesh::Vertex v[] = {
    { CalPoint4(1, 2, 3), CalVector4(1, 1, 0) },
  };

  CalCoreSubmesh::Influence i[] = {
    CalCoreSubmesh::Influence(0, 1.0f / 3.0f, false),
    CalCoreSubmesh::Influence(1, 1.0f / 3.0f, false),
    CalCoreSubmesh::Influence(2, 1.0f / 3.0f, true),
  };

  CalVector4 output[2];
  skin(bt, 1, v, i, output);

  CHECK_EQUAL(output[0].x, 4.0f / 3.0f);
  CHECK_EQUAL(output[0].y, 7.0f / 3.0f);
  CHECK_EQUAL(output[0].z, 10.0f / 3.0f);
  CHECK_EQUAL(output[1].x, 1);
  CHECK_EQUAL(output[1].y, 1);
  CHECK_EQUAL(output[1].z, 0);
}
APPLY_SKIN_FIXTURES(three_translated_bones);

ABSTRACT_TEST(two_rotated_bones) {
  CalSkeleton::BoneTransform bt[] = {
    // 90 degree rotation about z
    { CalVector4(0, -1, 0, 0),
      CalVector4(1,  0, 0, 0),
      CalVector4(0,  0, 1, 0), },
    // -90 degree rotation about x
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
  skin(bt, 1, v, i, output);
  CHECK_EQUAL(output[0].x, 0);
  CHECK_EQUAL(output[0].y, 1);
  CHECK_EQUAL(output[0].z, 0);
  CHECK_EQUAL(output[1].x, 0);
  CHECK_EQUAL(output[1].y, 1);
  CHECK_EQUAL(output[1].z, 0);
}
APPLY_SKIN_FIXTURES(two_rotated_bones);

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
