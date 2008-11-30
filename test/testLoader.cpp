#include "TestPrologue.h"
#include <sstream>


const char* animationText =
"<HEADER MAGIC=\"XAF\" VERSION=\"919\" />\n"
"<ANIMATION NUMTRACKS=\"1\" DURATION=\"40\">\n"
"    <TRACK BONEID=\"0\" TRANSLATIONREQUIRED=\"0\" TRANSLATIONISDYNAMIC=\"0\" HIGHRANGEREQUIRED=\"1\" NUMKEYFRAMES=\"2\">\n"
"        <KEYFRAME TIME=\"0\">\n"
"            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>\n"
"        </KEYFRAME>\n"
"        <KEYFRAME TIME=\"40\">\n"
"            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>\n"
"        </KEYFRAME>\n"
"    </TRACK>\n"
"</ANIMATION>\n"
;


TEST(LoadSimpleXmlAnimation) {

  // Load animation.

  CalCoreAnimation* anim = CalLoader::loadXmlCoreAnimation(animationText, 0);
  CHECK(anim);

  // Check animation.

  CHECK_EQUAL(anim->numCoreTracks(), 1);
  CHECK_EQUAL(anim->getDuration(), 40);

  // Check tracks.

  CalCoreTrack* track1 = anim->nthCoreTrack(0);
  CalCoreTrack* track2 = anim->getCoreTrack(/*boneid*/ 0);
  CHECK(track1);
  CHECK_EQUAL(track1, track2);

  CHECK_EQUAL(track1->getTranslationRequired(), false);
  CHECK_EQUAL(track1->getTranslationIsDynamic(), false);
  CHECK_EQUAL(track1->getHighRangeRequired(), true);
  CHECK_EQUAL(track1->getCoreKeyframeCount(), 2);

  // Save the animation back to an XML file.

  std::ostringstream ss;
  CalSaver::saveXmlCoreAnimation(ss, anim);

  CHECK_EQUAL(ss.str(), animationText);

  anim->destroy();
  delete anim;
}


#if 0
TEST(Simple) {
  CalLoader loader;
  CalCoreAnimation* ca1 = loader.loadCoreAnimation();
  CalCoreAnimation* ca2 = loader.loadCoreAnimation();

  CHECK(ca1);
  CHECK(ca2);
  CHECK_EQUAL(*ca1, *ca2);

  ca1->Destroy();
  delete ca1;

  ca2->Destroy();
  delete ca2;
}
#endif


TEST(CalVectorFromDataSrc) {
  char buf[7];
  CalBufferSource bs(buf, sizeof(buf)/sizeof(char));

  CalVector vec;
  CHECK_EQUAL(false, CalVectorFromDataSrc(bs, &vec));
}


TEST(loading_mesh_without_vertex_colors_defaults_to_white) {
    CalCoreSubmesh* sm = new CalCoreSubmesh();
    sm->reserve(1, 0, 0);
    CalCoreSubmesh::Vertex v;
    v.influenceStart = 0;
    v.influenceCount = 0;
    sm->getVectorVertex()[0] = v;
    sm->getVertexColors()[0] = CalMakeColor(CalVector(0, 0, 0));
    sm->setHasNonWhiteVertexColors(false);

    CalCoreMesh cm;
    cm.addCoreSubmesh(sm);

    const char* path = tmpnam(NULL);
    CalSaver().saveCoreMesh(path, &cm);

    CalCoreMesh* loaded = CalLoader().loadCoreMesh(path);
	CHECK(loaded);
    CHECK_EQUAL(1, cm.getCoreSubmeshCount());
    CHECK_EQUAL(1, loaded->getCoreSubmeshCount());

    CHECK_EQUAL(cm.getCoreSubmesh(0)->hasNonWhiteVertexColors(),
                loaded->getCoreSubmesh(0)->hasNonWhiteVertexColors());

    _unlink(path);
    delete loaded;
}

#if 0
TEST(converting_xml_to_binary_then_back_to_xml_does_not_modify_animation) {
    CalCoreAnimation* anim1 = CalLoader::loadXmlCoreAnimation(animationText, 0);
    CHECK(anim1);

    std::stringstream buf1;
    CalSaver::saveXmlCoreAnimation(buf1, anim1);

    const char* path = tmpnam(NULL);
    CalSaver::saveCoreAnimation(path, anim1);

    CalCoreAnimation* anim2 = CalLoader::loadCoreAnimation(path, 0);
    CHECK(anim2);

    _unlink(path);

    std::stringstream buf2;
    CalSaver::saveXmlCoreAnimation(buf2, anim2);

    CHECK_EQUAL_STR(buf1.str().c_str(), buf2.str().c_str());
}
#endif
