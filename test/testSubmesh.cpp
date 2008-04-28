#include "TestPrologue.h"


TEST(CalSubmesh_getFaces_succeeds_if_face_list_is_empty) {
    CalCoreSubmesh csm;
    CHECK(csm.create());

    CalSubmesh sm;
    sm.create(&csm);

    CHECK_EQUAL(0, sm.getFaces(0));
}

TEST(CalRenderer_getTextureCoordinates_when_there_are_no_texture_coordinates) {
    CalCoreSubmesh* coreSubmesh = new CalCoreSubmesh;
    coreSubmesh->create();
    coreSubmesh->reserve(0, 1, 0, 0);

    CalCoreMesh* coreMesh = new CalCoreMesh;
    CHECK(coreMesh->create());
    coreMesh->addCoreSubmesh(coreSubmesh);

    CalCoreModel coreModel;
    CHECK(coreModel.createInternal("model"));
    coreModel.addCoreMesh(coreMesh);

    CalCoreSkeleton* coreSkeleton = new CalCoreSkeleton;
    coreModel.setCoreSkeleton(coreSkeleton);

    CalModel model;
    model.create(&coreModel);
    CHECK(model.attachMesh(0));

    CalRenderer renderer;
    renderer.create(&model);

    CHECK(renderer.beginRendering());
    renderer.getTextureCoordinates(0, 0);
    renderer.endRendering();

    renderer.destroy();
    model.destroy();
    coreModel.destroy();
}

TEST(CalRenderer_getNormals_when_there_are_no_normals) {
    CalCoreSubmesh* coreSubmesh = new CalCoreSubmesh;
    coreSubmesh->create();
    coreSubmesh->reserve(0, 0, 0, 0);

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

    CalRenderer renderer;
    renderer.create(&model);

    CHECK(renderer.beginRendering());
    renderer.getNormals(0);
    renderer.endRendering();

    renderer.destroy();
    model.destroy();
    coreModel.destroy();
}
