#include "TestPrologue.h"
#include <cal3d/bone.h>
#include <cal3d/coremorphtarget.h>
#include <cal3d/submesh.h>
#include <cal3d/skeleton.h>
#include <cal3d/physique.h>
#include <cal3d/mixer.h>

#if defined(_MSC_VER)
#   include <intrin.h>
#else

inline cal3d_uint64 __rdtsc(void) {
    cal3d_uint64 myvar;
    __asm__ volatile("rdtsc \n\t" : "=A"(myvar));
    return myvar;
}

#endif

FIXTURE(skin_x87) {
    CalPhysique::SkinRoutine skin;
    SETUP(skin_x87) {
        skin = CalPhysique::calculateVerticesAndNormals_x87;
    }
};

#ifndef IMVU_NO_INTRINSICS
FIXTURE(skin_SSE_intrinsics) {
    CalPhysique::SkinRoutine skin;
    SETUP(skin_SSE_intrinsics) {
        skin = CalPhysique::calculateVerticesAndNormals_SSE_intrinsics;
    }
};
#endif


#ifdef IMVU_NO_ASM_BLOCKS
#ifdef IMVU_NO_INTRINSICS
FIXTURE(skin_SSE) {
    CalPhysique::SkinRoutine skin;
    SETUP(skin_SSE) {
        skin = CalPhysique::calculateVerticesAndNormals_x87;
    }
};
#else
FIXTURE_EXTENDS(skin_SSE, skin_SSE_intrinsics) {
};
#endif
#else
FIXTURE(skin_SSE) {
    CalPhysique::SkinRoutine skin;
    SETUP(skin_SSE) {
        skin = CalPhysique::calculateVerticesAndNormals_SSE;
    }
};
#endif

#if defined(IMVU_NO_INTRINSICS)
#define APPLY_SKIN_FIXTURES(test)         \
  APPLY_TEST_F(skin_x87, test)            \
  APPLY_TEST_F(skin_SSE, test)
#else
#define APPLY_SKIN_FIXTURES(test)         \
  APPLY_TEST_F(skin_x87, test)            \
  APPLY_TEST_F(skin_SSE_intrinsics, test) \
  APPLY_TEST_F(skin_SSE, test)
#endif

ABSTRACT_TEST(single_identity_bone) {
    BoneTransform bt;
    bt.rowx = CalVector4(1, 0, 0, 0);
    bt.rowy = CalVector4(0, 1, 0, 0);
    bt.rowz = CalVector4(0, 0, 1, 0);

    CalCoreSubmesh::Vertex v[] = {
        { CalPoint4(1, 2, 3), CalVector4(0, 1, 0) },
    };

    CalCoreSubmesh::Influence i[] = {
        CalCoreSubmesh::Influence(0, 1.0f, true),
    };

    CalVector4 output[2];
    this->skin(&bt, 1, v, i, output);
    CHECK_EQUAL(output[0].x, 1);
    CHECK_EQUAL(output[0].y, 2);
    CHECK_EQUAL(output[0].z, 3);
    CHECK_EQUAL(output[1].x, 0);
    CHECK_EQUAL(output[1].y, 1);
    CHECK_EQUAL(output[1].z, 0);
}
APPLY_SKIN_FIXTURES(single_identity_bone);

ABSTRACT_TEST(two_translated_bones) {
    BoneTransform bt[2];
    bt[0].rowx = CalVector4(1, 0, 0, 1);
    bt[0].rowy = CalVector4(0, 1, 0, 0);
    bt[0].rowz = CalVector4(0, 0, 1, 0);
    bt[1].rowx = CalVector4(1, 0, 0, 0);
    bt[1].rowy = CalVector4(0, 1, 0, 1);
    bt[1].rowz = CalVector4(0, 0, 1, 0);

    CalCoreSubmesh::Vertex v[] = {
        { CalPoint4(1, 2, 3), CalVector4(1, 1, 0) },
    };

    CalCoreSubmesh::Influence i[] = {
        CalCoreSubmesh::Influence(0, 0.5f, false),
        CalCoreSubmesh::Influence(1, 0.5f, true),
    };

    CalVector4 output[2];
    this->skin(bt, 1, v, i, output);
    CHECK_EQUAL(output[0].x, 1.5);
    CHECK_EQUAL(output[0].y, 2.5);
    CHECK_EQUAL(output[0].z, 3);
    CHECK_EQUAL(output[1].x, 1);
    CHECK_EQUAL(output[1].y, 1);
    CHECK_EQUAL(output[1].z, 0);
}
APPLY_SKIN_FIXTURES(two_translated_bones);

ABSTRACT_TEST(three_translated_bones) {
    BoneTransform bt[] = {
        BoneTransform(
            CalVector4(1, 0, 0, 1),
            CalVector4(0, 1, 0, 0),
            CalVector4(0, 0, 1, 0)
        ),
        BoneTransform(
            CalVector4(1, 0, 0, 0),
            CalVector4(0, 1, 0, 1),
            CalVector4(0, 0, 1, 0)
        ),
        BoneTransform(
            CalVector4(1, 0, 0, 0),
            CalVector4(0, 1, 0, 0),
            CalVector4(0, 0, 1, 1)
        ),
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
    this->skin(bt, 1, v, i, output);

    CHECK_CLOSE(output[0].x, 4.0f / 3.0f, 1.e-5);
    CHECK_CLOSE(output[0].y, 7.0f / 3.0f, 1.e-5);
    CHECK_CLOSE(output[0].z, 10.0f / 3.0f, 1.e-5);
    CHECK_CLOSE(output[1].x, 1, 1.e-5);
    CHECK_CLOSE(output[1].y, 1, 1.e-5);
    CHECK_CLOSE(output[1].z, 0, 1.e-5);
}
APPLY_SKIN_FIXTURES(three_translated_bones);

ABSTRACT_TEST(two_rotated_bones) {
    BoneTransform bt[] = {
        // 90 degree rotation about z
        BoneTransform(
            CalVector4(0, -1, 0, 0),
            CalVector4(1,  0, 0, 0),
            CalVector4(0,  0, 1, 0)
        ),
        // -90 degree rotation about x
        BoneTransform(
            CalVector4(1,  0, 0, 0),
            CalVector4(0,  0, 1, 0),
            CalVector4(0, -1, 0, 0)
        ),
    };

    CalCoreSubmesh::Vertex v[] = {
        { CalPoint4(1, 1, 1), CalVector4(1, 1, 1) },
    };

    CalCoreSubmesh::Influence i[] = {
        CalCoreSubmesh::Influence(0, 0.5f, false),
        CalCoreSubmesh::Influence(1, 0.5f, true),
    };

    CalVector4 output[2];
    this->skin(bt, 1, v, i, output);
    CHECK_EQUAL(output[0].x, 0);
    CHECK_EQUAL(output[0].y, 1);
    CHECK_EQUAL(output[0].z, 0);
    CHECK_EQUAL(output[1].x, 0);
    CHECK_EQUAL(output[1].y, 1);
    CHECK_EQUAL(output[1].z, 0);
}
APPLY_SKIN_FIXTURES(two_rotated_bones);

ABSTRACT_TEST(skin_10000_vertices_1_influence_cycle_count) {
    const int N = 10000;
    const int TrialCount = 10;

    CalCoreSubmesh::Vertex v[N];
    CalCoreSubmesh::Influence i[N];
    for (int k = 0; k < N; ++k) {
        v[k].position.setAsPoint(CalVector(1.0f, 2.0f, 3.0f));
        v[k].normal.setAsVector(CalVector(0.0f, 0.0f, 1.0f));
        i[k].boneId = 0;
        i[k].weight = 1.0f;
        i[k].lastInfluenceForThisVertex = true;
    }

    BoneTransform bt;
    memset(&bt, 0, sizeof(bt));

    CAL3D_ALIGN_HEAD(16) CalVector4 output[N * 2] CAL3D_ALIGN_TAIL(16);

    cal3d_int64 min = 99999999999999LL;
    for (int t = 0; t < TrialCount; ++t) {
        cal3d_int64 start = __rdtsc();
        this->skin(&bt, N, v, i, output);
        cal3d_int64 end = __rdtsc();
        cal3d_int64 elapsed = end - start;
        if (elapsed < min) {
            min = elapsed;
        }
    }

    printf("Cycles per vertex: %d\n", (int)(min / N));
}
APPLY_SKIN_FIXTURES(skin_10000_vertices_1_influence_cycle_count);

static CalCoreSubmeshPtr djinnCoreSubmesh(int N) {
    CalCoreSubmeshPtr coreSubmesh(new CalCoreSubmesh(N, 0, 0));
    for (int k = 0; k < N; ++k) {
        CalCoreSubmesh::Vertex v;
        v.position.setAsPoint(CalVector(1.0f, 2.0f, 3.0f));
        v.normal.setAsVector(CalVector(0.0f, 0.0f, 1.0f));
        std::vector<CalCoreSubmesh::Influence> inf(1);
        inf[0].boneId = 0;
        inf[0].weight = 1.0f;
        inf[0].lastInfluenceForThisVertex = true;
        coreSubmesh->addVertex(v, 0, inf);
    }
    return coreSubmesh;
}

static CalCoreMorphTargetPtr djinnMorphTarget(int N, const char* name) {
    CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget(name));
    morphTarget->reserve(N);
    for (int k = 0; k < N; ++k) {
        CalCoreMorphTarget::BlendVertex bv;
        bv.position.set(1.0f, 2.0f, 3.0f, 1.0f);
        bv.normal.set(0.0f, 0.0f, 1.0f, 0.0f);
        morphTarget->setBlendVertex(k, bv);
    }
    return morphTarget;
}

TEST(morph_targets_performance_test) {
    const int N = 10000;
    const int TrialCount = 10;

    CalCoreSubmeshPtr coreSubmesh(djinnCoreSubmesh(N));

    coreSubmesh->addCoreSubMorphTarget(djinnMorphTarget(N, "foo1"));
    coreSubmesh->addCoreSubMorphTarget(djinnMorphTarget(N, "foo2"));

    CalSubmesh submesh(coreSubmesh);
    submesh.setMorphTargetWeight("foo1", 0.25f);
    submesh.setMorphTargetWeight("foo2", 0.25f);

    BoneTransform bt;
    memset(&bt, 0, sizeof(bt));

    CAL3D_ALIGN_HEAD(16) CalVector4 output[N * 2] CAL3D_ALIGN_TAIL(16);

    cal3d_int64 min = 99999999999999LL;
    for (int t = 0; t < TrialCount; ++t) {
        cal3d_int64 start = __rdtsc();
        CalPhysique::calculateVerticesAndNormals(&bt, &submesh, &output[0].x);
        cal3d_int64 end = __rdtsc();
        cal3d_int64 elapsed = end - start;
        if (elapsed < min) {
            min = elapsed;
        }
    }

    printf("Cycles per vertex: %d\n", (int)(min / N));
}

TEST(bunch_of_unweighted_morph_targets_performance_test) {
    const int N = 10000;
    const int M = 30;
    const int TrialCount = 10;

    CalCoreSubmeshPtr coreSubmesh(djinnCoreSubmesh(N));

    for (int i = 0; i < M; ++i) {
        char buf[80];
        sprintf(buf, "%d", i);
        coreSubmesh->addCoreSubMorphTarget(djinnMorphTarget(N, buf));
    }

    CalSubmesh submesh(coreSubmesh);
    submesh.setMorphTargetWeight("0", 0.25f);

    BoneTransform bt;
    memset(&bt, 0, sizeof(bt));

    CAL3D_ALIGN_HEAD(16) CalVector4 output[N * 2] CAL3D_ALIGN_TAIL(16);

    cal3d_int64 min = 99999999999999LL;
    for (int t = 0; t < TrialCount; ++t) {
        cal3d_int64 start = __rdtsc();
        CalPhysique::calculateVerticesAndNormals(&bt, &submesh, &output[0].x);
        cal3d_int64 end = __rdtsc();
        cal3d_int64 elapsed = end - start;
        if (elapsed < min) {
            min = elapsed;
        }
    }

    printf("Cycles per vertex: %d\n", (int)(min / N));
}

TEST(single_morph_target) {
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    inf[0].lastInfluenceForThisVertex = true;

    CalCoreSubmeshPtr coreSubmesh(new CalCoreSubmesh(3, 0, 1));
    CalCoreSubmesh::Vertex v;

    v.position.setAsPoint(CalVector(0, 0, 0));
    v.normal.setAsVector(CalVector(0, 0, 0));
    coreSubmesh->addVertex(v, 0, inf);
    coreSubmesh->addVertex(v, 0, inf);
    coreSubmesh->addVertex(v, 0, inf);

    CalCoreMorphTargetPtr morphTarget(new CalCoreMorphTarget("foo"));
    CalCoreMorphTarget::BlendVertex bv;
    bv.position.setAsPoint(CalVector(1, 1, 1));
    bv.normal.setAsVector(CalVector(1, 1, 1));
    morphTarget->reserve(3);
    morphTarget->setBlendVertex(0, bv);
    morphTarget->setBlendVertex(1, bv);
    morphTarget->setBlendVertex(2, bv);
    coreSubmesh->addCoreSubMorphTarget(morphTarget);

    CalSubmesh submesh(coreSubmesh);
    submesh.setMorphTargetWeight("foo", 0.5f);
    
    BoneTransform bt;
    bt.rowx.set(1, 0, 0, 0);
    bt.rowy.set(0, 1, 0, 0);
    bt.rowz.set(0, 0, 1, 0);

    CAL3D_ALIGN_HEAD(16) CalVector4 output[3 * 2] CAL3D_ALIGN_TAIL(16);

    CalPhysique::calculateVerticesAndNormals(&bt, &submesh, &output[0].x);

    CHECK_EQUAL(0.5, output[0].x);
    CHECK_EQUAL(0.5, output[0].y);
    CHECK_EQUAL(0.5, output[0].z);
    //CHECK_EQUAL(1.0, output[0].w);
    CHECK_EQUAL(0.5, output[1].x);
    CHECK_EQUAL(0.5, output[1].y);
    CHECK_EQUAL(0.5, output[1].z);
    //CHECK_EQUAL(0.0, output[1].w);
}

TEST(two_morph_targets) {
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    inf[0].lastInfluenceForThisVertex = true;

    CalCoreSubmeshPtr coreSubmesh(new CalCoreSubmesh(3, 0, 1));
    CalCoreSubmesh::Vertex v;

    v.position.setAsPoint(CalVector(0, 0, 0));
    v.normal.setAsVector(CalVector(0, 0, 0));
    coreSubmesh->addVertex(v, 0, inf);
    coreSubmesh->addVertex(v, 0, inf);
    coreSubmesh->addVertex(v, 0, inf);

    CalCoreMorphTarget::BlendVertex bv;
    bv.position.setAsPoint(CalVector(1, 1, 1));
    bv.normal.setAsVector(CalVector(1, 1, 1));

    CalCoreMorphTargetPtr morphTarget1(new CalCoreMorphTarget("foo"));
    morphTarget1->reserve(3);
    morphTarget1->setBlendVertex(0, bv);
    morphTarget1->setBlendVertex(1, bv);
    morphTarget1->setBlendVertex(2, bv);
    coreSubmesh->addCoreSubMorphTarget(morphTarget1);

    CalCoreMorphTargetPtr morphTarget2(new CalCoreMorphTarget("bar"));
    morphTarget2->reserve(3);
    morphTarget2->setBlendVertex(0, bv);
    morphTarget2->setBlendVertex(1, bv);
    morphTarget2->setBlendVertex(2, bv);
    coreSubmesh->addCoreSubMorphTarget(morphTarget2);

    CalSubmesh submesh(coreSubmesh);
    submesh.setMorphTargetWeight("foo", 0.25f);
    submesh.setMorphTargetWeight("bar", 0.25f);
    
    BoneTransform bt;
    bt.rowx.set(1, 0, 0, 0);
    bt.rowy.set(0, 1, 0, 0);
    bt.rowz.set(0, 0, 1, 0);

    CAL3D_ALIGN_HEAD(16) CalVector4 output[3 * 2] CAL3D_ALIGN_TAIL(16);

    CalPhysique::calculateVerticesAndNormals(&bt, &submesh, &output[0].x);

    CHECK_EQUAL(0.5, output[0].x);
    CHECK_EQUAL(0.5, output[0].y);
    CHECK_EQUAL(0.5, output[0].z);
    //CHECK_EQUAL(1.0, output[0].w);
    CHECK_EQUAL(0.5, output[1].x);
    CHECK_EQUAL(0.5, output[1].y);
    CHECK_EQUAL(0.5, output[1].z);
    //CHECK_EQUAL(0.0, output[1].w);
}

TEST(two_disjoint_morph_targets) {
    std::vector<CalCoreSubmesh::Influence> inf(1);
    inf[0].boneId = 0;
    inf[0].weight = 1.0f;
    inf[0].lastInfluenceForThisVertex = true;

    CalCoreSubmeshPtr coreSubmesh(new CalCoreSubmesh(2, 0, 1));
    CalCoreSubmesh::Vertex v;

    v.position.setAsPoint(CalVector(0, 0, 0));
    v.normal.setAsVector(CalVector(0, 0, 0));
    coreSubmesh->addVertex(v, 0, inf);
    coreSubmesh->addVertex(v, 0, inf);

    CalCoreMorphTarget::BlendVertex bv;
    bv.position.setAsPoint(CalVector(1, 1, 1));
    bv.normal.setAsVector(CalVector(1, 1, 1));

    CalCoreMorphTargetPtr morphTarget1(new CalCoreMorphTarget("foo"));
    morphTarget1->reserve(2);
    morphTarget1->setBlendVertex(0, bv);

    coreSubmesh->addCoreSubMorphTarget(morphTarget1);

    CalCoreMorphTargetPtr morphTarget2(new CalCoreMorphTarget("bar"));
    morphTarget2->reserve(2);
    morphTarget2->setBlendVertex(1, bv);
    coreSubmesh->addCoreSubMorphTarget(morphTarget2);

    CalSubmesh submesh(coreSubmesh);
    submesh.setMorphTargetWeight("foo", 1.0f);
    submesh.setMorphTargetWeight("bar", 1.0f);
    
    BoneTransform bt;
    bt.rowx.set(1, 0, 0, 0);
    bt.rowy.set(0, 1, 0, 0);
    bt.rowz.set(0, 0, 1, 0);

    CAL3D_ALIGN_HEAD(16) CalVector4 output[2 * 2] CAL3D_ALIGN_TAIL(16);

    CalPhysique::calculateVerticesAndNormals(&bt, &submesh, &output[0].x);

    // position of vertex 0
    CHECK_EQUAL(1, output[0].x);
    CHECK_EQUAL(1, output[0].y);
    CHECK_EQUAL(1, output[0].z);

    // position of vertex 1
    CHECK_EQUAL(1, output[2].x);
    CHECK_EQUAL(1, output[2].y);
    CHECK_EQUAL(1, output[2].z);
}
