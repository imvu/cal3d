#include "TestPrologue.h"
#include <cal3d/renderer.h>
#include <cal3d/model.h>
#include <cal3d/submesh.h>
#include <cal3d/physique.h>


TEST(CalSubmesh_getFaces_succeeds_if_face_list_is_empty) {
    CalCoreSubmesh csm;

    CalSubmesh sm(&csm);

    sm.getVectorFace();
}

TEST(CalRenderer_getTextureCoordinates_when_there_are_no_texture_coordinates) {
    CalCoreSubmesh* coreSubmesh = new CalCoreSubmesh;
    coreSubmesh->reserve(0, 1, 0);

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

    CalSubmesh* submesh = model.getMesh(0)->getSubmesh(0);
    const std::vector<CalCoreSubmesh::TextureCoordinate>& texCoords = submesh->getCoreSubmesh()->getVectorVectorTextureCoordinate()[0];
    CHECK_EQUAL(0, texCoords.size());

    model.destroy();
    coreModel.destroy();
}

TEST(CalRenderer_getNormals_when_there_are_no_normals) {
    CalCoreSubmesh* coreSubmesh = new CalCoreSubmesh;
    coreSubmesh->reserve(0, 0, 0);

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

    CalSubmesh* submesh = model.getMesh(0)->getSubmesh(0);
    CalPhysique::calculateVerticesAndNormals(
      model.getSkeleton()->boneTransforms.data,
      submesh,
      0);

    model.destroy();
    coreModel.destroy();
}
