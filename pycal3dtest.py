import re
import os
import sys
import tempfile
import weakref
import imvu.test
import imvu.fs
from cal3d import pycal3d
import cal3d

class Test(imvu.test.TestCase):
    def setUp(self):
        self.cal3d_ = pycal3d.Cal3d()
        
    def testSkel(self):
        self.hereAndBackAgain(origData=skeleton1, calCoreType="CoreSkeleton")

    def testMaterial(self):
        self.hereAndBackAgain(origData=material1, calCoreType="CoreMaterial")

    def testAnimatedMorph(self):
        self.hereAndBackAgain(origData=animmorph1, calCoreType="CoreAnimatedMorph")
        

    def assertEqual(self, a, b):
        f = open(os.path.join(tempfile.gettempdir(), "dataA"), "w")
        f.write(str(a))
        f.close()
        f = open(os.path.join(tempfile.gettempdir(), "dataB"), "w")
        f.write(str(b))
        f.close()
        imvu.test.TestCase.assertEqual(self, a,b)
       
    def hereAndBackAgain(self, origData, calCoreType):
        data = self.cal3d_.convertToBinary(calCoreType=calCoreType, data=origData)
        assert len(data) < len(origData)


        data3 = re.sub("\s+", "", origData)
        data4 = self.cal3d_.convertToXml(calCoreType=calCoreType, data=data)
        data4 = re.sub("POSDIFF=\".+\"", "", data3)
        data4 = re.sub("\s+", "", data3)
        self.assertEqual(len(data3), len(data4))
        self.assertEqual(data3, data4)
        
        data2 = self.cal3d_.convertToBinary(calCoreType=calCoreType, data=data)
        self.assertEqual(repr(data), repr(data2))
        self.assertEqual(len(data), len(data2))

    def testMesh(self):
        self.hereAndBackAgain(origData=mesh1, calCoreType="CoreMesh")
        
    def testXmlExtension(self):
        path = "/foo/bar/baz/test.xsf"
        self.assertEqual(self.cal3d_.cal3dTypeForPath(path), "CoreSkeleton")
        path = "test.csf"
        self.assertEqual(self.cal3d_.cal3dTypeForPath(path), "CoreSkeleton")

    def test_winding_is_not_changed_in_meshes(self):
        test_mesh = os.path.join(imvu.fs.getSourceDirectory(), 'TestData', 'fuzhuangdian.xmf')
        data = file(test_mesh, 'rb').read()
        self.hereAndBackAgain(data, 'CoreMesh')
            
    def test_morph_loader_does_not_crash(self):
        anim1 = os.path.join(imvu.fs.getSourceDirectory(), 'TestData', 'AnimationCEO3K.xaf')
        anim2 = os.path.join(imvu.fs.getSourceDirectory(), 'TestData', 'SkeletalAnimation.xaf')
        self.assertIs(None, cal3d.loadCoreAnimatedMorphFromBuffer(file(anim1, 'rb').read()))
        self.assertIs(None, cal3d.loadCoreAnimatedMorphFromBuffer(file(anim2, 'rb').read()))

    def test_xml_morph_loader_reads_weights(self):
        anim = os.path.join(imvu.fs.getSourceDirectory(), 'TestData', 'gbhello2.XPF')
        morph = cal3d.loadCoreAnimatedMorphFromBuffer(file(anim, 'rb').read())
        self.assertGbHello2(morph)

    def test_binary_morph_loader_reads_weights(self):
        anim = os.path.join(imvu.fs.getSourceDirectory(), 'TestData', 'gbhello2.XPF')
        morph = cal3d.loadCoreAnimatedMorphFromBuffer(file(anim, 'rb').read())
        binaryData = cal3d.saveCoreAnimatedMorphToBuffer(morph)
        morph = cal3d.loadCoreAnimatedMorphFromBuffer(binaryData)
        self.assertGbHello2(morph)

    def test_xml_to_binary_and_back_removes_keyframes_with_corrupt_data(self):
        anim = os.path.join(imvu.fs.getSourceDirectory(), 'TestData', 'HoserAnmationR.xaf')
        origData = file(anim, 'rb').read()
        data = self.cal3d_.convertToBinary(calCoreType="CoreAnimation", data=origData);
        data = self.cal3d_.convertToXml(calCoreType="CoreAnimation", data=data);
        data = re.sub("\s+", "", data)
        origData = re.sub("\s+", "", origData)
        self.assertNotEqual(len(data), len(origData));


    def assertGbHello2(self, morph):
        self.assertEqual(1, morph.duration)
        self.assertEqual(12, len(morph.tracks))

        track = morph.tracks[0]
        self.assertEqual('mouthopen.exclusive', track.name)
        
        self.assertEqual(0, track.keyframes[0].time)
        self.assertEqual(0, track.keyframes[0].weight)

        self.assertEqual(0.033333301544189453, track.keyframes[1].time)
        self.assertEqual(0, track.keyframes[1].weight)

        # test_leftArmUp_has_weights

        [leftArmUp] = [t for t in morph.tracks if t.name == 'lfarmup.exclusive']

        self.assertEqual(0, leftArmUp.keyframes[0].time)
        self.assertEqual(0.57999998331069946, leftArmUp.keyframes[0].weight)

        self.assertEqual(0.033333301544189453, leftArmUp.keyframes[1].time)
        self.assertEqual(0.59440302848815918, leftArmUp.keyframes[1].weight)

        # test_empty_tracks_can_load:
        
        lastTrack = morph.tracks[-1]

        self.assertEqual("eyeseyes.exclusive", lastTrack.name)
        self.assertEqual([], [(k.time, k.weight) for k in lastTrack.keyframes])

        # test_removeZeroScaleTracks_does_what_it_says:
        
        morph.removeZeroScaleTracks()
        self.assertEqual(1, len(morph.tracks))

    def test_get_format(self):
        self.assertEqual('CoreSkeleton,XML', self.cal3d_.getFormat(skeleton1))
        self.assertEqual('CoreMesh,XML', self.cal3d_.getFormat(mesh1))
        self.assertEqual('CoreAnimation,XML', self.cal3d_.getFormat(animation1))
        self.assertEqual('CoreAnimation,XML', self.cal3d_.getFormat(animation2))
        self.assertEqual('CoreMaterial,XML', self.cal3d_.getFormat(material1))
        self.assertEqual('CoreAnimatedMorph,XML', self.cal3d_.getFormat(animmorph1))

    def test_get_format_doesnt_crash_on_malformed_xml(self):
        self.assertEqual(None, self.cal3d_.getFormat("<janky>" + mesh1 + "</janky>"))
        self.assertEqual(None, self.cal3d_.getFormat(xml_that_crashes_due_to_missing_header))
        self.assertEqual(None, self.cal3d_.getFormat(xml_that_crashes_due_to_missing_header_attributes))

skeleton1 = """<HEADER VERSION="910" MAGIC="XSF" />
<SKELETON SCENEAMBIENTCOLOR="1 1 1" NUMBONES="2">
    <BONE NAME="FemaleAnimeRoot" NUMCHILDS="1" ID="0">
        <TRANSLATION>0 1.31134e-006 -30</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 -1.31134e-006 30</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>-1</PARENTID>
        <CHILDID>1</CHILDID>
    </BONE>
    <BONE NAME="PelvisNode" NUMCHILDS="3" ID="1">
        <TRANSLATION>0 -10.3201 519.784</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10.3201 -489.784</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
</SKELETON>"""

mesh1 = """<HEADER VERSION="910" MAGIC="XMF" />
<MESH NUMSUBMESH="1">
    <SUBMESH NUMVERTICES="11" NUMFACES="6" NUMLODSTEPS="0" NUMSPRINGS="0" NUMMORPHS="9" NUMTEXCOORDS="1" MATERIAL="2">
        <VERTEX NUMINFLUENCES="1" ID="0">
            <POS>30.7542 -112.13 825.308</POS>
            <NORM>-0.857233 0.397024 0.327908</NORM>
            <COLOR>1.35632e-019 1.35632e-019 1.35632e-019</COLOR>
            <TEXCOORD>0.571776 0.980153</TEXCOORD>
            <INFLUENCE ID="22">1</INFLUENCE>
        </VERTEX>
        <VERTEX NUMINFLUENCES="1" ID="1">
            <POS>21.5894 -115.955 805.98</POS>
            <NORM>-0.807413 0.586997 -0.0593164</NORM>
            <COLOR>1.35632e-019 1.35632e-019 1.35632e-019</COLOR>
            <TEXCOORD>0.552218 0.978547</TEXCOORD>
            <INFLUENCE ID="22">1</INFLUENCE>
        </VERTEX>
        <VERTEX NUMINFLUENCES="1" ID="2">
            <POS>16.7126 -120.828 804.738</POS>
            <NORM>-0.460269 0.441253 -0.770356</NORM>
            <COLOR>0 3.82554e-043 3.81153e-043</COLOR>
            <TEXCOORD>0.527233 0.977693</TEXCOORD>
            <INFLUENCE ID="22">1</INFLUENCE>
        </VERTEX>
        <VERTEX NUMINFLUENCES="1" ID="3">
            <POS>30.7542 -112.13 825.308</POS>
            <NORM>-0.27356 0.943696 -0.186018</NORM>
            <COLOR>0 0 1.00893e-043</COLOR>
            <TEXCOORD>0.571776 0.980153</TEXCOORD>
            <INFLUENCE ID="22">1</INFLUENCE>
        </VERTEX>
        <VERTEX NUMINFLUENCES="1" ID="4">
            <POS>9.46755 -122.028 806.4</POS>
            <NORM>-0.186905 0.971857 -0.143389</NORM>
            <COLOR>1.4013e-043 0 0</COLOR>
            <TEXCOORD>0.521052 0.976423</TEXCOORD>
            <INFLUENCE ID="22">1</INFLUENCE>
        </VERTEX>
        <VERTEX NUMINFLUENCES="1" ID="5">
            <POS>0.340274 -121.767 824.44</POS>
            <NORM>3.52695e-009 0.991284 -0.131742</NORM>
            <COLOR>2.30786e-011 1.03696e-043 0</COLOR>
            <TEXCOORD>0.522246 0.974824</TEXCOORD>
            <INFLUENCE ID="22">1</INFLUENCE>
        </VERTEX>
        <VERTEX NUMINFLUENCES="1" ID="6">
            <POS>0.340274 -122.581 804.669</POS>
            <NORM>-3.02535e-008 0.999153 -0.041138</NORM>
            <COLOR>1.66755e-043 0 0</COLOR>
            <TEXCOORD>0.518241 0.974824</TEXCOORD>
            <INFLUENCE ID="22">1</INFLUENCE>
        </VERTEX>
        <VERTEX NUMINFLUENCES="1" ID="7">
            <POS>16.7126 -120.828 804.738</POS>
            <NORM>-0.214234 0.943525 -0.252715</NORM>
            <COLOR>1.96935e-020 7.84727e-043 2.38221e-044</COLOR>
            <TEXCOORD>0.527233 0.977693</TEXCOORD>
            <INFLUENCE ID="22">1</INFLUENCE>
        </VERTEX>
        <VERTEX NUMINFLUENCES="1" ID="8">
            <POS>0.340268 -109.433 792.513</POS>
            <NORM>3.95365e-007 0.697319 0.716761</NORM>
            <COLOR>-332.537 1.56945e-043 2.38221e-044</COLOR>
            <TEXCOORD>0.585726 0.974824</TEXCOORD>
            <INFLUENCE ID="22">1</INFLUENCE>
        </VERTEX>
        <VERTEX NUMINFLUENCES="1" ID="9">
            <POS>0.340274 -120.596 803.374</POS>
            <NORM>8.04736e-008 0.445458 0.895303</NORM>
            <COLOR>2.09443e-038 0 0</COLOR>
            <TEXCOORD>0.528441 0.974824</TEXCOORD>
            <INFLUENCE ID="22">1</INFLUENCE>
        </VERTEX>
        <VERTEX NUMINFLUENCES="1" ID="10">
            <POS>20.2896 -106.929 794.81</POS>
            <NORM>-0.129956 0.645329 0.75277</NORM>
            <COLOR>-1.4013e-043 0 0</COLOR>
            <TEXCOORD>0.59854 0.97832</TEXCOORD>
            <INFLUENCE ID="22">1</INFLUENCE>
        </VERTEX>
        
        
        
        
        <MORPH NAME="left.Eye.Down.Clamped" NUMBLENDVERTS="0" MORPHID="0" />
        <MORPH NAME="left.Eye.Left.Clamped" NUMBLENDVERTS="0" MORPHID="1" />
        <MORPH NAME="left.Eye.Right.Clamped" NUMBLENDVERTS="0" MORPHID="2" />
        <MORPH NAME="left.Eye.Up.Clamped" NUMBLENDVERTS="0" MORPHID="3" />
        <MORPH NAME="right.Eye.Down.Clamped" NUMBLENDVERTS="0" MORPHID="4" />
        <MORPH NAME="right.Eye.Left.Clamped" NUMBLENDVERTS="0" MORPHID="5" />
        <MORPH NAME="right.Eye.Right.Clamped" NUMBLENDVERTS="0" MORPHID="6" />
        <MORPH NAME="right.Eye.Up.Clamped" NUMBLENDVERTS="0" MORPHID="7" />
        <MORPH NAME="eyes.Blink.Clamped" NUMBLENDVERTS="10" MORPHID="8">
            <BLENDVERTEX VERTEXID="0">
                <POSITION>7.69704 -121.396 862.701</POSITION>
                <NORMAL>0.475756 -0.861712 -0.176376</NORMAL>
                <TEXCOORD>0.508674 0.275276</TEXCOORD>
            </BLENDVERTEX>
            <BLENDVERTEX VERTEXID="1">
                <POSITION>71.2067 -85.9199 860.71</POSITION>
                <NORMAL>0.865095 -0.4979 -0.06088</NORMAL>
                <TEXCOORD>0.636769 0.276661</TEXCOORD>
            </BLENDVERTEX>
            <BLENDVERTEX VERTEXID="2">
                <POSITION>65.5313 -89.114 852.661</POSITION>
                <NORMAL>0.714048 -0.680395 -0.164917</NORMAL>
                <TEXCOORD>0.626629 0.291508</TEXCOORD>
            </BLENDVERTEX>
            <BLENDVERTEX VERTEXID="3">
                <POSITION>66.4918 -87.6711 848.23</POSITION>
                <NORMAL>0.721416 -0.689776 -0.0613941</NORMAL>
                <TEXCOORD>0.629534 0.300306</TEXCOORD>
            </BLENDVERTEX>
            <BLENDVERTEX VERTEXID="4">
                <POSITION>72.3876 -80.8385 848.623</POSITION>
                <NORMAL>0.903905 -0.422648 -0.065763</NORMAL>
                <TEXCOORD>0.647298 0.300101</TEXCOORD>
            </BLENDVERTEX>
            <BLENDVERTEX VERTEXID="5">
                <POSITION>56.955 -104.788 853.335</POSITION>
                <NORMAL>0.558053 -0.504662 -0.658706</NORMAL>
                <TEXCOORD>0.601702 0.300145</TEXCOORD>
            </BLENDVERTEX>
            <BLENDVERTEX VERTEXID="6">
                <POSITION>50.0733 -108.024 852.259</POSITION>
                <NORMAL>0.297636 -0.624358 -0.722212</NORMAL>
                <TEXCOORD>0.587854 0.304354</TEXCOORD>
            </BLENDVERTEX>
            <BLENDVERTEX VERTEXID="7">
                <POSITION>49.1971 -103.946 848.372</POSITION>
                <NORMAL>0.386808 -0.762641 -0.518419</NORMAL>
                <TEXCOORD>0.587099 0.307948</TEXCOORD>
            </BLENDVERTEX>
            <BLENDVERTEX VERTEXID="8">
                <POSITION>57.8752 -99.3678 850.163</POSITION>
                <NORMAL>0.700746 -0.568581 -0.430895</NORMAL>
                <TEXCOORD>0.604766 0.30405</TEXCOORD>
            </BLENDVERTEX>
            <BLENDVERTEX VERTEXID="9">
                <POSITION>14.8428 -112.115 862.01</POSITION>
                <NORMAL>0.589296 -0.79844 -0.123384</NORMAL>
                <TEXCOORD>0.526391 0.269252</TEXCOORD>
            </BLENDVERTEX>
        </MORPH>
        <FACE VERTEXID="0 1 2" />
        <FACE VERTEXID="3 4 5" />
        <FACE VERTEXID="4 6 5" />
        <FACE VERTEXID="7 4 3" />
        <FACE VERTEXID="8 9 5" />
        <FACE VERTEXID="9 3 5" />
    </SUBMESH>
</MESH>"""

animation1 = """<HEADER VERSION="910" MAGIC="XAF" />
<ANIMATION NUMTRACKS="5" DURATION="4">
    <TRACK NUMKEYFRAMES="32" BONEID="56">
        <KEYFRAME TIME="0">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569964 -0.437345 0.517176 -0.465187</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.564503 -0.461901 0.501256 -0.465538</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.03333">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.564902 -0.460634 0.501926 -0.465589</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.06667">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.565299 -0.459325 0.502633 -0.465637</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.1">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.56569 -0.45799 0.503371 -0.465679</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.13333">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.566074 -0.45663 0.504142 -0.465715</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.16667">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.566443 -0.455268 0.504932 -0.465744</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.2">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.566798 -0.453906 0.50574 -0.465765</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.23333">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.567136 -0.45255 0.506564 -0.465777</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.26667">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.567455 -0.451218 0.507392 -0.46578</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.3">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.567756 -0.449899 0.508229 -0.465775</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.33333">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.568038 -0.448611 0.509065 -0.465761</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.36667">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.5683 -0.447359 0.509895 -0.465739</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.4">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.568542 -0.446143 0.510717 -0.465708</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.43333">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.568762 -0.444987 0.511515 -0.46567</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.46667">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.568962 -0.443888 0.512286 -0.465627</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.5">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569142 -0.442853 0.513026 -0.465578</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.53333">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569302 -0.441893 0.513723 -0.465526</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.56667">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569443 -0.441004 0.514378 -0.465472</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.6">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569567 -0.440197 0.514982 -0.465418</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.63333">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569674 -0.439474 0.515529 -0.465366</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.66667">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569762 -0.438849 0.516006 -0.465317</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.7">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569835 -0.438325 0.516411 -0.465274</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.73333">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569891 -0.437906 0.516737 -0.465237</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.76667">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569931 -0.4376 0.516976 -0.46521</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.8">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569956 -0.437411 0.517125 -0.465193</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.83333">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569964 -0.437345 0.517176 -0.465187</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.86667">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569964 -0.437345 0.517176 -0.465187</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.9">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569964 -0.437345 0.517176 -0.465187</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.93333">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569964 -0.437345 0.517176 -0.465187</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.96667">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569964 -0.437345 0.517176 -0.465187</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="4">
            <TRANSLATION>95.4475 -0.29209 -0.0204049</TRANSLATION>
            <ROTATION>0.569964 -0.437345 0.517176 -0.465187</ROTATION>
        </KEYFRAME>
    </TRACK>
    <TRACK NUMKEYFRAMES="32" BONEID="57">
        <KEYFRAME TIME="0">
            <TRANSLATION>-94.9605 -9.48851e-005 2.88643e-006</TRANSLATION>
            <ROTATION>0.322539 0.588375 -0.579296 0.462817</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3">
            <TRANSLATION>-94.9605 -4.55434e-005 3.9704e-006</TRANSLATION>
            <ROTATION>-0.0611443 -0.627387 0.771113 0.0896201</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.03333">
            <TRANSLATION>-94.9605 -2.97193e-005 1.22749e-006</TRANSLATION>
            <ROTATION>-0.0589109 -0.627065 0.771595 0.0892196</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.06667">
            <TRANSLATION>-94.9605 -7.32982e-005 -1.84994e-007</TRANSLATION>
            <ROTATION>-0.0568738 -0.627006 0.771925 0.0880897</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.1">
            <TRANSLATION>-94.9605 -6.6281e-005 4.31833e-006</TRANSLATION>
            <ROTATION>-0.0564557 -0.627169 0.772342 0.0834265</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.13333">
            <TRANSLATION>-94.9605 -8.00995e-005 3.72933e-006</TRANSLATION>
            <ROTATION>-0.0589326 -0.627472 0.772993 0.0727196</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.16667">
            <TRANSLATION>-94.9605 -9.89769e-005 -4.52926e-006</TRANSLATION>
            <ROTATION>-0.0640326 -0.627875 0.773604 0.0566621</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.2">
            <TRANSLATION>-94.9605 -9.90162e-005 -1.67586e-006</TRANSLATION>
            <ROTATION>-0.0715313 -0.628315 0.773837 0.0357753</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.23333">
            <TRANSLATION>-94.9605 -6.51164e-005 -1.31578e-006</TRANSLATION>
            <ROTATION>-0.0812016 -0.62871 0.773316 0.0106469</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.26667">
            <TRANSLATION>-94.9605 -4.34946e-005 5.4158e-006</TRANSLATION>
            <ROTATION>-0.0927371 -0.628963 0.771676 -0.0179216</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.3">
            <TRANSLATION>-94.9605 -4.5916e-005 5.50144e-006</TRANSLATION>
            <ROTATION>-0.106049 -0.628982 0.76855 -0.0496512</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.33333">
            <TRANSLATION>-94.9605 -8.82345e-005 3.12064e-006</TRANSLATION>
            <ROTATION>-0.120829 -0.628668 0.763656 -0.0837072</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.36667">
            <TRANSLATION>-94.9605 -5.3489e-005 1.87621e-006</TRANSLATION>
            <ROTATION>-0.136822 -0.627934 0.756783 -0.119403</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.4">
            <TRANSLATION>-94.9605 -0.000115604 1.81096e-006</TRANSLATION>
            <ROTATION>-0.153868 -0.626703 0.747761 -0.156274</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.43333">
            <TRANSLATION>-94.9605 -9.36538e-005 -2.70465e-006</TRANSLATION>
            <ROTATION>-0.171474 -0.624942 0.736703 -0.193166</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.46667">
            <TRANSLATION>-94.9605 -6.3082e-005 3.34916e-006</TRANSLATION>
            <ROTATION>-0.189449 -0.622628 0.723679 -0.229635</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.5">
            <TRANSLATION>-94.9605 -6.76616e-005 4.43534e-006</TRANSLATION>
            <ROTATION>-0.207488 -0.619773 0.708929 -0.265046</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.53333">
            <TRANSLATION>-94.9605 -3.58452e-005 5.96365e-006</TRANSLATION>
            <ROTATION>-0.225168 -0.616452 0.692907 -0.298608</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.56667">
            <TRANSLATION>-94.9605 -6.49661e-005 3.50357e-006</TRANSLATION>
            <ROTATION>-0.242397 -0.612707 0.675861 -0.330221</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.6">
            <TRANSLATION>-94.9605 -3.82724e-005 6.78129e-006</TRANSLATION>
            <ROTATION>-0.258742 -0.608682 0.658426 -0.359213</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.63333">
            <TRANSLATION>-94.9605 -4.56537e-005 -6.61357e-008</TRANSLATION>
            <ROTATION>-0.273978 -0.604508 0.6411 -0.385353</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.66667">
            <TRANSLATION>-94.9605 -8.23829e-005 -7.94134e-007</TRANSLATION>
            <ROTATION>-0.287602 -0.600424 0.624752 -0.407996</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.7">
            <TRANSLATION>-94.9605 -8.16911e-005 -3.21283e-007</TRANSLATION>
            <ROTATION>-0.299407 -0.596609 0.609952 -0.42705</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.73333">
            <TRANSLATION>-94.9605 -7.6184e-005 -4.3729e-006</TRANSLATION>
            <ROTATION>-0.309095 -0.593285 0.597374 -0.442287</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.76667">
            <TRANSLATION>-94.9605 -9.15017e-005 3.17584e-006</TRANSLATION>
            <ROTATION>0.316335 0.590684 -0.587727 0.453434</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.8">
            <TRANSLATION>-94.9605 -4.14264e-005 4.11736e-006</TRANSLATION>
            <ROTATION>0.320929 0.588981 -0.581497 0.460399</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.83333">
            <TRANSLATION>-94.9605 -9.46325e-005 9.96234e-007</TRANSLATION>
            <ROTATION>0.322539 0.588375 -0.579296 0.462817</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.86667">
            <TRANSLATION>-94.9605 -9.48851e-005 2.88643e-006</TRANSLATION>
            <ROTATION>0.322539 0.588375 -0.579296 0.462817</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.9">
            <TRANSLATION>-94.9605 -9.48851e-005 2.88643e-006</TRANSLATION>
            <ROTATION>0.322539 0.588375 -0.579296 0.462817</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.93333">
            <TRANSLATION>-94.9605 -9.48851e-005 2.88643e-006</TRANSLATION>
            <ROTATION>0.322539 0.588375 -0.579296 0.462817</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.96667">
            <TRANSLATION>-94.9605 -9.48851e-005 2.88643e-006</TRANSLATION>
            <ROTATION>0.322539 0.588375 -0.579296 0.462817</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="4">
            <TRANSLATION>-94.9605 -9.48851e-005 2.88643e-006</TRANSLATION>
            <ROTATION>0.322539 0.588375 -0.579296 0.462817</ROTATION>
        </KEYFRAME>
    </TRACK>
    <TRACK NUMKEYFRAMES="32" BONEID="58">
        <KEYFRAME TIME="0">
            <TRANSLATION>43.9686 -8.88883e-007 -3.60704e-005</TRANSLATION>
            <ROTATION>0.0188448 0.0488381 -0.00770525 0.998599</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3">
            <TRANSLATION>43.9686 -8.05016e-006 -2.83851e-005</TRANSLATION>
            <ROTATION>-0.297082 0.043915 -0.0227158 0.953571</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.03333">
            <TRANSLATION>43.9686 -1.42594e-005 -8.21696e-005</TRANSLATION>
            <ROTATION>-0.288796 0.0441105 -0.0223339 0.956113</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.06667">
            <TRANSLATION>43.9686 -6.11722e-006 -3.55719e-005</TRANSLATION>
            <ROTATION>-0.279745 0.0443196 -0.0219159 0.9588</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.1">
            <TRANSLATION>43.9686 -1.61471e-006 -3.77367e-005</TRANSLATION>
            <ROTATION>-0.26952 0.0445505 -0.0214428 0.961725</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.13333">
            <TRANSLATION>43.9686 -4.48747e-007 -8.98891e-006</TRANSLATION>
            <ROTATION>-0.257705 0.0448101 -0.0208948 0.964958</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.16667">
            <TRANSLATION>43.9686 -1.05368e-006 -2.51094e-006</TRANSLATION>
            <ROTATION>-0.244613 0.045089 -0.020286 0.96836</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.2">
            <TRANSLATION>43.9686 5.88961e-006 -4.22187e-005</TRANSLATION>
            <ROTATION>-0.230345 0.0453824 -0.0196207 0.971852</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.23333">
            <TRANSLATION>43.9686 -7.08612e-006 -3.04502e-005</TRANSLATION>
            <ROTATION>-0.215076 0.0456846 -0.0189066 0.975345</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.26667">
            <TRANSLATION>43.9686 -1.40071e-005 -5.90604e-005</TRANSLATION>
            <ROTATION>-0.199084 0.0459879 -0.0181563 0.978734</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.3">
            <TRANSLATION>43.9686 2.61801e-006 -2.64273e-005</TRANSLATION>
            <ROTATION>-0.182348 0.0462911 -0.0173687 0.98199</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.33333">
            <TRANSLATION>43.9686 -3.02582e-006 -5.65601e-005</TRANSLATION>
            <ROTATION>-0.16515 0.0465877 -0.0165566 0.985029</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.36667">
            <TRANSLATION>43.9686 7.24104e-006 -4.53921e-006</TRANSLATION>
            <ROTATION>-0.147674 0.0468737 -0.0157288 0.987799</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.4">
            <TRANSLATION>43.9686 1.5131e-005 1.16243e-005</TRANSLATION>
            <ROTATION>-0.130002 0.0471472 -0.0148888 0.99028</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.43333">
            <TRANSLATION>43.9686 1.01785e-005 7.51126e-007</TRANSLATION>
            <ROTATION>-0.112544 0.0474021 -0.0140562 0.992416</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.46667">
            <TRANSLATION>43.9686 1.50875e-005 -2.42128e-005</TRANSLATION>
            <ROTATION>-0.0953821 0.0476379 -0.0132352 0.994212</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.5">
            <TRANSLATION>43.9686 -4.13826e-006 -5.32784e-005</TRANSLATION>
            <ROTATION>-0.0787105 0.047853 -0.0124352 0.995671</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.53333">
            <TRANSLATION>43.9686 -2.53503e-006 -3.4705e-005</TRANSLATION>
            <ROTATION>-0.0628196 0.0480452 -0.0116703 0.996799</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.56667">
            <TRANSLATION>43.9686 2.36464e-006 -3.76053e-005</TRANSLATION>
            <ROTATION>-0.0477021 0.0482166 -0.0109407 0.997637</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.6">
            <TRANSLATION>43.9685 -2.01711e-006 -3.96523e-005</TRANSLATION>
            <ROTATION>-0.0336526 0.0483658 -0.0102608 0.99821</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.63333">
            <TRANSLATION>43.9686 -1.19226e-006 -4.43825e-005</TRANSLATION>
            <ROTATION>-0.0207872 0.048494 -0.00963673 0.998561</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.66667">
            <TRANSLATION>43.9686 -9.47661e-007 -3.86478e-005</TRANSLATION>
            <ROTATION>-0.00945659 0.0486003 -0.00908592 0.998732</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.7">
            <TRANSLATION>43.9685 2.0907e-009 -3.8424e-005</TRANSLATION>
            <ROTATION>0.000233916 0.0486862 -0.00861396 0.998777</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.73333">
            <TRANSLATION>43.9686 -1.14782e-006 -3.36582e-005</TRANSLATION>
            <ROTATION>0.00809632 0.0487524 -0.00823042 0.998744</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.76667">
            <TRANSLATION>43.9685 -2.29865e-007 -3.90786e-005</TRANSLATION>
            <ROTATION>0.0139137 0.0487995 -0.00794631 0.99868</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.8">
            <TRANSLATION>43.9685 1.30403e-007 -1.3182e-005</TRANSLATION>
            <ROTATION>0.0175731 0.0488283 -0.00776744 0.998622</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.83333">
            <TRANSLATION>43.9686 3.28427e-007 -4.18209e-005</TRANSLATION>
            <ROTATION>0.0188448 0.0488382 -0.00770526 0.998599</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.86667">
            <TRANSLATION>43.9686 -8.88883e-007 -3.60704e-005</TRANSLATION>
            <ROTATION>0.0188448 0.0488381 -0.00770525 0.998599</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.9">
            <TRANSLATION>43.9686 -8.88883e-007 -3.60704e-005</TRANSLATION>
            <ROTATION>0.0188448 0.0488381 -0.00770525 0.998599</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.93333">
            <TRANSLATION>43.9686 -8.88883e-007 -3.60704e-005</TRANSLATION>
            <ROTATION>0.0188448 0.0488381 -0.00770525 0.998599</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.96667">
            <TRANSLATION>43.9686 -8.88883e-007 -3.60704e-005</TRANSLATION>
            <ROTATION>0.0188448 0.0488381 -0.00770525 0.998599</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="4">
            <TRANSLATION>43.9686 -8.88883e-007 -3.60704e-005</TRANSLATION>
            <ROTATION>0.0188448 0.0488381 -0.00770525 0.998599</ROTATION>
        </KEYFRAME>
    </TRACK>
    <TRACK NUMKEYFRAMES="32" BONEID="59">
        <KEYFRAME TIME="0">
            <TRANSLATION>91.2128 4.06705e-005 2.5501e-005</TRANSLATION>
            <ROTATION>-0.00596945 0.0368511 -0.237566 0.970654</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3">
            <TRANSLATION>91.2129 1.21172e-005 3.55895e-005</TRANSLATION>
            <ROTATION>0.00984135 0.0357772 -0.619065 0.784463</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.03333">
            <TRANSLATION>91.2129 1.80665e-005 4.19845e-005</TRANSLATION>
            <ROTATION>0.0100391 0.0357458 -0.62321 0.781173</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.06667">
            <TRANSLATION>91.2129 6.52349e-005 5.14624e-005</TRANSLATION>
            <ROTATION>0.0101684 0.0357268 -0.625906 0.779013</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.1">
            <TRANSLATION>91.2129 4.41837e-005 3.74079e-005</TRANSLATION>
            <ROTATION>0.0102148 0.0357204 -0.626871 0.778237</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.13333">
            <TRANSLATION>91.2129 3.69847e-005 5.62322e-005</TRANSLATION>
            <ROTATION>0.0101209 0.0356645 -0.624819 0.779889</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.16667">
            <TRANSLATION>91.2129 3.45264e-006 2.82499e-005</TRANSLATION>
            <ROTATION>0.00984967 0.0355076 -0.618897 0.784607</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.2">
            <TRANSLATION>91.2129 2.84497e-005 4.64944e-005</TRANSLATION>
            <ROTATION>0.00941573 0.0352718 -0.609427 0.792002</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.23333">
            <TRANSLATION>91.2129 5.05568e-005 7.08403e-005</TRANSLATION>
            <ROTATION>0.00883264 0.0349823 -0.596705 0.801649</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.26667">
            <TRANSLATION>91.2129 4.21172e-005 1.93249e-005</TRANSLATION>
            <ROTATION>0.00811944 0.0346693 -0.581132 0.81303</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.3">
            <TRANSLATION>91.2129 4.4828e-005 7.69265e-005</TRANSLATION>
            <ROTATION>0.00728216 0.0343575 -0.562803 0.825845</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.33333">
            <TRANSLATION>91.2129 5.36816e-005 1.98742e-005</TRANSLATION>
            <ROTATION>0.00634202 0.0340756 -0.542119 0.839587</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.36667">
            <TRANSLATION>91.2128 4.86168e-005 8.52224e-005</TRANSLATION>
            <ROTATION>0.00531756 0.0338476 -0.519412 0.853837</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.4">
            <TRANSLATION>91.2128 4.28391e-005 4.34281e-005</TRANSLATION>
            <ROTATION>0.00422266 0.0336924 -0.494888 0.868293</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.43333">
            <TRANSLATION>91.2128 4.03538e-005 4.56777e-005</TRANSLATION>
            <ROTATION>0.00309354 0.0336268 -0.469243 0.882423</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.46667">
            <TRANSLATION>91.2129 4.39868e-005 2.11678e-005</TRANSLATION>
            <ROTATION>0.00194702 0.0336575 -0.442755 0.896009</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.5">
            <TRANSLATION>91.2128 4.88321e-005 7.06339e-005</TRANSLATION>
            <ROTATION>0.000807158 0.0337861 -0.415875 0.908793</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.53333">
            <TRANSLATION>91.2128 4.75387e-005 5.76888e-005</TRANSLATION>
            <ROTATION>-0.000295912 0.0340065 -0.389249 0.920504</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.56667">
            <TRANSLATION>91.2128 4.03115e-005 5.26926e-005</TRANSLATION>
            <ROTATION>-0.00135409 0.0343098 -0.36304 0.931141</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.6">
            <TRANSLATION>91.2128 4.24457e-005 4.26678e-005</TRANSLATION>
            <ROTATION>-0.00233977 0.0346774 -0.337933 0.940528</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.63333">
            <TRANSLATION>91.2128 4.66312e-005 5.37676e-005</TRANSLATION>
            <ROTATION>-0.0032403 0.0350896 -0.314315 0.948664</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.66667">
            <TRANSLATION>91.2129 3.77102e-005 4.34942e-005</TRANSLATION>
            <ROTATION>-0.00402876 0.0355151 -0.293017 0.955439</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.7">
            <TRANSLATION>91.2128 4.18511e-005 5.85303e-005</TRANSLATION>
            <ROTATION>-0.00469832 0.0359271 -0.27442 0.960927</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.73333">
            <TRANSLATION>91.2129 4.11344e-005 2.44448e-005</TRANSLATION>
            <ROTATION>-0.00523753 0.0362951 -0.259056 0.965166</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.76667">
            <TRANSLATION>91.2128 3.60566e-005 4.89386e-005</TRANSLATION>
            <ROTATION>-0.00563409 0.0365878 -0.24751 0.968178</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.8">
            <TRANSLATION>91.2128 3.87415e-005 4.48362e-005</TRANSLATION>
            <ROTATION>-0.00588286 0.0367817 -0.24015 0.970021</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.83333">
            <TRANSLATION>91.2129 3.73856e-005 3.61495e-005</TRANSLATION>
            <ROTATION>-0.00596943 0.0368511 -0.237567 0.970654</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.86667">
            <TRANSLATION>91.2128 4.06705e-005 2.5501e-005</TRANSLATION>
            <ROTATION>-0.00586253 0.0368 -0.240454 0.969945</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.9">
            <TRANSLATION>91.2128 4.06705e-005 2.5501e-005</TRANSLATION>
            <ROTATION>-0.00555145 0.036657 -0.24881 0.967842</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.93333">
            <TRANSLATION>91.2128 4.06705e-005 2.5501e-005</TRANSLATION>
            <ROTATION>-0.00506137 0.0364481 -0.261848 0.964407</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.96667">
            <TRANSLATION>91.2128 4.06705e-005 2.5501e-005</TRANSLATION>
            <ROTATION>-0.00441131 0.0362021 -0.278906 0.959626</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="4">
            <TRANSLATION>91.2128 4.06705e-005 2.5501e-005</TRANSLATION>
            <ROTATION>-0.00362262 0.0359497 -0.29928 0.953481</ROTATION>
        </KEYFRAME>
    </TRACK>
    <TRACK NUMKEYFRAMES="32" BONEID="60">
        <KEYFRAME TIME="0">
            <TRANSLATION>72.7357 -1.61703e-006 1.28109e-005</TRANSLATION>
            <ROTATION>0.00467341 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3">
            <TRANSLATION>72.7357 1.07692e-005 3.9353e-006</TRANSLATION>
            <ROTATION>0.00467339 -0.028265 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.03333">
            <TRANSLATION>72.7357 1.61303e-005 1.66317e-005</TRANSLATION>
            <ROTATION>0.00467342 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.06667">
            <TRANSLATION>72.7357 -6.21167e-006 -3.46275e-006</TRANSLATION>
            <ROTATION>0.00467342 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.1">
            <TRANSLATION>72.7357 1.7472e-007 2.02526e-005</TRANSLATION>
            <ROTATION>0.0046734 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.13333">
            <TRANSLATION>72.7357 9.85208e-006 4.60304e-005</TRANSLATION>
            <ROTATION>0.00467341 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.16667">
            <TRANSLATION>72.7357 1.60431e-005 -8.25728e-007</TRANSLATION>
            <ROTATION>0.00467337 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.2">
            <TRANSLATION>72.7357 -1.18258e-005 -1.15562e-005</TRANSLATION>
            <ROTATION>0.00467339 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.23333">
            <TRANSLATION>72.7357 2.94209e-007 1.06317e-005</TRANSLATION>
            <ROTATION>0.00467339 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.26667">
            <TRANSLATION>72.7357 -8.77417e-006 2.06404e-005</TRANSLATION>
            <ROTATION>0.00467345 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.3">
            <TRANSLATION>72.7357 6.7073e-006 5.26405e-005</TRANSLATION>
            <ROTATION>0.00467343 -0.028265 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.33333">
            <TRANSLATION>72.7357 6.88124e-006 2.26269e-005</TRANSLATION>
            <ROTATION>0.0046734 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.36667">
            <TRANSLATION>72.7357 -3.57982e-005 3.16057e-005</TRANSLATION>
            <ROTATION>0.00467337 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.4">
            <TRANSLATION>72.7357 -9.59668e-006 2.49264e-005</TRANSLATION>
            <ROTATION>0.00467338 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.43333">
            <TRANSLATION>72.7357 -1.50613e-005 -1.64708e-005</TRANSLATION>
            <ROTATION>0.00467342 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.46667">
            <TRANSLATION>72.7357 -2.89419e-005 -1.81675e-005</TRANSLATION>
            <ROTATION>0.00467343 -0.028265 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.5">
            <TRANSLATION>72.7357 -1.51273e-005 4.92059e-007</TRANSLATION>
            <ROTATION>0.00467343 -0.0282649 -0.0312174 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.53333">
            <TRANSLATION>72.7357 -1.24491e-005 2.30548e-005</TRANSLATION>
            <ROTATION>0.00467343 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.56667">
            <TRANSLATION>72.7357 2.32299e-005 4.81231e-005</TRANSLATION>
            <ROTATION>0.00467342 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.6">
            <TRANSLATION>72.7357 2.05929e-005 6.7274e-005</TRANSLATION>
            <ROTATION>0.00467344 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.63333">
            <TRANSLATION>72.7357 1.63031e-005 2.6265e-005</TRANSLATION>
            <ROTATION>0.00467345 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.66667">
            <TRANSLATION>72.7357 1.04166e-005 2.48224e-005</TRANSLATION>
            <ROTATION>0.0046734 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.7">
            <TRANSLATION>72.7357 2.136e-005 4.03411e-005</TRANSLATION>
            <ROTATION>0.0046734 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.73333">
            <TRANSLATION>72.7357 1.59724e-005 6.50903e-005</TRANSLATION>
            <ROTATION>0.00467339 -0.028265 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.76667">
            <TRANSLATION>72.7357 8.35969e-006 2.68285e-005</TRANSLATION>
            <ROTATION>0.00467339 -0.028265 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.8">
            <TRANSLATION>72.7357 6.29816e-007 2.22037e-005</TRANSLATION>
            <ROTATION>0.0046734 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.83333">
            <TRANSLATION>72.7357 4.87364e-006 4.01586e-005</TRANSLATION>
            <ROTATION>0.00467339 -0.028265 -0.0312174 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.86667">
            <TRANSLATION>72.7357 9.66629e-006 4.99856e-005</TRANSLATION>
            <ROTATION>0.00467341 -0.0282648 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.9">
            <TRANSLATION>72.7357 4.53278e-006 3.51966e-005</TRANSLATION>
            <ROTATION>0.00467341 -0.028265 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.93333">
            <TRANSLATION>72.7357 1.23556e-005 3.35502e-005</TRANSLATION>
            <ROTATION>0.00467338 -0.028265 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="3.96667">
            <TRANSLATION>72.7357 -1.708e-006 2.2571e-005</TRANSLATION>
            <ROTATION>0.0046734 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="4">
            <TRANSLATION>72.7357 1.84476e-005 4.1089e-005</TRANSLATION>
            <ROTATION>0.00467341 -0.0282649 -0.0312175 0.999102</ROTATION>
        </KEYFRAME>
    </TRACK>
</ANIMATION>"""

animation2 = '''\
<HEADER MAGIC="XAF" VERSION="919" />
<ANIMATION NUMTRACKS="1" DURATION="40">
    <TRACK BONEID="0" TRANSLATIONREQUIRED="0" TRANSLATIONISDYNAMIC="0" HIGHRANGEREQUIRED="1" NUMKEYFRAMES="2">
        <KEYFRAME TIME="0">
            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>
        </KEYFRAME>
        <KEYFRAME TIME="40">
            <ROTATION>0.5 0.5 0.5 -0.5</ROTATION>
        </KEYFRAME>
    </TRACK>
</ANIMATION>
'''

material1 = """<HEADER VERSION="910" MAGIC="XRF" />
<MATERIAL NUMMAPS="2">
    <AMBIENT>255 255 255 255 </AMBIENT>
    <DIFFUSE>255 255 255 255</DIFFUSE>
    <SPECULAR>255 255 255 255</SPECULAR>
    <SHININESS>0</SHININESS>
    <MAP TYPE="Diffuse Color">Female01_Anime01_hair_Marla_Brunette.tga</MAP>
    <MAP TYPE="Opacity">Female01_Anime01_hair_Marla_Brunette_whatever.tga</MAP>
  
</MATERIAL>"""

animmorph1 = """<HEADER VERSION="910" MAGIC="XPF" />
<ANIMATION NUMTRACKS="1" DURATION="0.166667">
    <TRACK NUMKEYFRAMES="6" MORPHNAME="eyes.Blink.Clamped">
        <KEYFRAME TIME="0">
            <WEIGHT>0</WEIGHT>
        </KEYFRAME>
        <KEYFRAME TIME="0.0333333">
            <WEIGHT>0.5</WEIGHT>
        </KEYFRAME>
        <KEYFRAME TIME="0.0666667">
            <WEIGHT>1</WEIGHT>
        </KEYFRAME>
        <KEYFRAME TIME="0.1">
            <WEIGHT>0.740741</WEIGHT>
        </KEYFRAME>
        <KEYFRAME TIME="0.133333">
            <WEIGHT>0.259259</WEIGHT>
        </KEYFRAME>
        <KEYFRAME TIME="0.166667">
            <WEIGHT>0</WEIGHT>
        </KEYFRAME>
    </TRACK>
</ANIMATION>"""

xml_that_crashes_due_to_missing_header = """<imvu>
  <settings>
    <bodyMesh>true</bodyMesh>
    <hideBody>false</hideBody>
    <gizmo>false</gizmo>
    <centerBone>true</centerBone>
    <zoom>true</zoom>
    <freeJoints>false</freeJoints>
    <idle>0</idle>
    <play>0</play>
    <duration>120</duration>
  </settings>
  <skeletalAnimation>
    <skel frame="0" bone="0">
      <n11>1</n11>
      <n12>0</n12>
      <n13>0</n13>
      <n14>0</n14>
      <n21>0</n21>
      <n22>1</n22>
      <n23>0</n23>
      <n24>0</n24>
      <n31>0</n31>
      <n32>0</n32>
      <n33>1</n33>
      <n34>0</n34>
    </skel>
    <skel frame="0" bone="1">
      <n11>1</n11>
      <n12>1.9883080000000318e-9</n12>
      <n13>3.898439999838548e-11</n13>
      <n14>0</n14>
      <n21>-1.988307999999968e-9</n21>
      <n22>1</n22>
      <n23>-1.6240160387564974e-12</n23>
      <n24>663.894</n24>
      <n31>-3.898440000161452e-11</n31>
      <n32>1.6240159612435028e-12</n32>
      <n33>1</n33>
      <n34>-0.0000282434</n34>
    </skel>
    <skel frame="0" bone="2">
      <n11>0.9960595035055613</n11>
      <n12>-0.06742756175732632</n12>
      <n13>0.05761066806835778</n13>
      <n14>-0.0356773</n14>
      <n21>-0.05747911715612632</n21>
      <n22>0.0038910768046263033</n22>
      <n23>0.9983390859931347</n23>
      <n24>-14.380199999999999</n24>
      <n31>-0.06753973830844222</n31>
      <n32>-0.9977165446388654</n32>
      <n33>6.731030377515879e-8</n33>
      <n34>0.306373</n34>
    </skel>
    <skel frame="0" bone="3">
      <n11>0.02797635889014949</n11>
      <n12>0.06977912669673628</n12>
      <n13>0.997170533680301</n13>
      <n14>56.6081</n14>
      <n21>-0.0012101931176637382</n21>
      <n22>0.9975621886800516</n22>
      <n23>-0.06977261172209154</n23>
      <n24>-0.00000107494</n24>
      <n31>-0.9996082908796993</n31>
      <n32>0.000745245828308462</n32>
      <n33>0.027992602918298637</n33>
      <n34>-0.0000171907</n34>
    </skel>
    <skel frame="0" bone="4">
      <n11>0.9796870264844714</n11>
      <n12>0.1953782450484099</n12>
      <n13>-0.04517378916841214</n13>
      <n14>278.212</n14>
      <n21>-0.19704521179359002</n21>
      <n22>0.9797353234896956</n22>
      <n23>-0.035942734236857544</n23>
      <n24>0.0000107337</n24>
      <n31>0.03723592855358786</n31>
      <n32>0.044113909337142425</n32>
      <n33>0.9983323337473433</n33>
      <n34>-0.00000219748</n34>
    </skel>
    <skel frame="0" bone="5">
      <n11>0.6110434077287943</n11>
      <n12>-0.7914401293669548</n12>
      <n13>0.01576433632798982</n13>
      <n14>264.186</n14>
      <n21>0.7910863546330453</n21>
      <n22>0.6112433193589326</n22>
      <n23>0.023749164903061545</n23>
      <n24>0.00000278183</n24>
      <n31>-0.028431886776010183</n31>
      <n32>-0.0020408203769384566</n32>
      <n33>0.9995936488929297</n33>
      <n34>-0.0000125063</n34>
    </skel>
    <skel frame="0" bone="6">
      <n11>0.6860307798919781</n11>
      <n12>-0.72664014248879</n12>
      <n13>0.03682234319751979</n13>
      <n14>88.527</n14>
      <n21>0.7273724080472097</n21>
      <n22>0.6861501128195087</n22>
      <n23>-0.011287837634962097</n23>
      <n24>-0.0000211857</n24>
      <n31>-0.01706345938808021</n31>
      <n32>0.03452736003143791</n32>
      <n33>0.9992580746697676</n33>
      <n34>-0.0000208006</n34>
    </skel>
    <skel frame="0" bone="7">
      <n11>0.9961766346142145</n11>
      <n12>-0.02982032835603225</n12>
      <n13>0.08211492053262454</n13>
      <n14>76.2554</n14>
      <n21>0.07904713298636776</n21>
      <n22>0.7078877526241949</n22>
      <n23>-0.7018877708798649</n23>
      <n24>0.000023072199999999996</n24>
      <n31>-0.03719761986977546</n31>
      <n32>0.705695146760135</n32>
      <n33>0.7075384507782798</n33>
      <n34>0.0000398289</n34>
    </skel>
    <skel frame="0" bone="8">
      <n11>-0.9959634471459999</n11>
      <n12>0.0682475779088</n12>
      <n13>0.0583169211658</n13>
      <n14>-0.0356773</n14>
      <n21>-0.05818048226320001</n21>
      <n22>0.0039863788290200874</n22>
      <n23>-0.9982985676273999</n23>
      <n24>-14.380199999999999</n24>
      <n31>-0.06836392812219999</n31>
      <n32>-0.9976608976765999</n32>
      <n33>-5.004409799891363e-7</n33>
      <n34>0.306373</n34>
    </skel>
    <skel frame="0" bone="9">
      <n11>0.033008334849911836</n11>
      <n12>0.07402591326933897</n12>
      <n13>-0.9967101102562768</n13>
      <n14>55.9252</n14>
      <n21>-0.0015245024258610448</n21>
      <n22>0.9972558910389917</n22>
      <n23>0.07401597733137717</n23>
      <n24>-0.00000457332</n24>
      <n31>0.9994541288197236</n31>
      <n32>-0.0009236734302228433</n32>
      <n33>0.033030608451897536</n33>
      <n34>-0.0000461224</n34>
    </skel>
    <skel frame="0" bone="10">
      <n11>0.9798621352527521</n11>
      <n12>0.1967768397985313</n12>
      <n13>0.033897958630405636</n13>
      <n14>278.143</n14>
      <n21>-0.19699333218226864</n21>
      <n22>0.9803998102872963</n22>
      <n23>0.003136787630015019</n23>
      <n24>0.000012581999999999999</n24>
      <n31>-0.03261630506559436</n31>
      <n32>-0.00975129118694498</n32>
      <n33>0.9994203765058348</n33>
      <n34>-0.0000018577800000000001</n34>
    </skel>
    <skel frame="0" bone="11">
      <n11>0.6111041562726488</n11>
      <n12>-0.7907844856039008</n12>
      <n13>0.03480636572434493</n13>
      <n14>264.197</n14>
      <n21>0.7915375812760994</n21>
      <n22>0.6107508571376727</n22>
      <n23>-0.02124907999462748</n23>
      <n24>0.0000132952</n24>
      <n31>-0.004454570051655074</n31>
      <n32>0.04053595072537253</n32>
      <n33>0.9991681506324706</n33>
      <n34>0.00000805068</n34>
    </skel>
    <skel frame="0" bone="12">
      <n11>0.6856721825554539</n11>
      <n12>-0.7278734027633269</n12>
      <n13>-0.007340528076833571</n13>
      <n14>88.5239</n14>
      <n21>0.727668584640673</n21>
      <n22>0.6851496063696565</n22>
      <n23>0.03268582890978738</n23>
      <n24>-0.000023206</n24>
      <n31>-0.01876178908715357</n31>
      <n32>-0.027753234662212613</n32>
      <n33>0.9994387190246141</n33>
      <n34>-0.0000119906</n34>
    </skel>
    <skel frame="0" bone="13">
      <n11>0.9944873816562673</n11>
      <n12>-0.10373064254148069</n12>
      <n13>0.015323234440680664</n13>
      <n14>76.2679</n14>
      <n21>0.0814136534597193</n21>
      <n22>0.671766038388594</n22>
      <n23>-0.7362759034918612</n23>
      <n24>0.0000379585</n24>
      <n31>0.06608074414028067</n31>
      <n32>0.7334646159361385</n32>
      <n33>0.6765079391976259</n33>
      <n34>0.0000541818</n34>
    </skel>
    <skel frame="0" bone="14">
      <n11>-0.00000699449999985724</n11>
      <n12>-0.00003261922199998546</n12>
      <n13>-0.9999999238459998</n13>
      <n14>0.346647</n14>
      <n21>0.9920539844059999</n21>
      <n22>-0.12581224820999992</n22>
      <n23>-0.0000029100179999574927</n23>
      <n24>21.7772</n24>
      <n31>-0.12581232364200007</n31>
      <n32>-0.9920539838739999</n32>
      <n33>0.000033325134000117274</n33>
      <n34>1.60017</n34>
    </skel>
    <skel frame="0" bone="15">
      <n11>0.9931235491645751</n11>
      <n12>-0.00001181340155251287</n12>
      <n13>0.11707096389077987</n13>
      <n14>40.0697</n14>
      <n21>-0.11707096388155254</n21>
      <n22>0.0000018623601817369106</n22>
      <n23>0.9931231588528476</n23>
      <n24>0.0000254352</n24>
      <n31>-0.00001190449322015924</n31>
      <n32>-0.9999996095471527</n32>
      <n33>4.6923134566778657e-7</n33>
      <n34>0.0000238999</n34>
    </skel>
    <skel frame="0" bone="16">
      <n11>0.9894641618663698</n11>
      <n12>0.000004269627687112338</n12>
      <n13>0.14477801097107548</n13>
      <n14>48.395999999999994</n14>
      <n21>-0.000004315194282647661</n21>
      <n22>0.9999999999906896</n22>
      <n23>7.073402014711244e-10</n23>
      <n24>0.0000384988</n24>
      <n31>-0.1447780109697245</n31>
      <n32>-6.254451301025287e-7</n32>
      <n33>0.9894641618755816</n33>
      <n34>0.0000030747</n34>
    </skel>
    <skel frame="0" bone="17">
      <n11>0.9269799609101484</n11>
      <n12>0.000007948975345563272</n12>
      <n13>0.3751108494069168</n13>
      <n14>58.60159999999999</n14>
      <n21>-0.000008576978406436724</n21>
      <n22>0.9999999999632178</n22>
      <n23>4.563320942116692e-9</n23>
      <n24>0.000028543199999999998</n24>
      <n31>-0.3751108493930831</n31>
      <n32>-0.000003221547821157883</n32>
      <n33>0.9269799609442302</n33>
      <n34>9.83102e-7</n34>
    </skel>
    <skel frame="0" bone="18">
      <n11>0.9476628734050045</n11>
      <n12>4.7025874596396355e-8</n12>
      <n13>-0.3192727336860004</n13>
      <n14>92.1706</n14>
      <n21>-5.291108769680362e-8</n21>
      <n22>0.9999999999999986</n22>
      <n23>-9.759677174125897e-9</n23>
      <n24>-0.0000605364</n24>
      <n31>0.3192727336859995</n31>
      <n32>2.61419513090741e-8</n32>
      <n33>0.9476628734050057</n33>
      <n34>0.00000925558</n34>
    </skel>
    <skel frame="0" bone="19">
      <n11>0.9726991842046947</n11>
      <n12>-6.624147628499052e-8</n12>
      <n13>-0.23206959417000012</n13>
      <n14>19.1467</n14>
      <n21>6.722490852180948e-8</n21>
      <n22>0.9999999999999978</n22>
      <n23>-3.6707205000748322e-9</n23>
      <n24>0.00000851217</n24>
      <n31>0.23206959416999984</n31>
      <n32>-1.2030350549354833e-8</n32>
      <n33>0.9726991842046969</n33>
      <n34>0.000361696</n34>
    </skel>
    <skel frame="0" bone="20">
      <n11>0.9994798913100649</n11>
      <n12>-6.739862000964319e-9</n12>
      <n13>0.032248207188</n13>
      <n14>13.8536</n14>
      <n21>6.7699015014356816e-9</n21>
      <n22>1</n22>
      <n23>-8.223221748862358e-10</n23>
      <n24>0.0000166514</n24>
      <n31>-0.032248207188</n31>
      <n32>1.0402116642337643e-9</n32>
      <n33>0.9994798913100649</n33>
      <n34>-0.00010751</n34>
    </skel>
    <skel frame="0" bone="21">
      <n11>0.9987510290599021</n11>
      <n12>-3.075607711484383e-8</n12>
      <n13>0.04996380642720003</n13>
      <n14>14.980399999999998</n14>
      <n21>3.0817505931556184e-8</n21>
      <n22>0.9999999999999996</n22>
      <n23>-4.591052808065057e-10</n23>
      <n24>0.0000332843</n24>
      <n31>-0.04996380642719999</n31>
      <n32>1.9982917720254944e-9</n32>
      <n33>0.9987510290599027</n33>
      <n34>0.000340901</n34>
    </skel>
    <skel frame="0" bone="22">
      <n11>0.9992848997226995</n11>
      <n12>0.000005286463737377236</n12>
      <n13>-0.037811230580793616</n13>
      <n14>19.7749</n14>
      <n21>-0.0000053262762420627644</n21>
      <n22>0.999999999985362</n22>
      <n23>-9.521954875907986e-7</n23>
      <n24>-0.0000493426</n24>
      <n31>0.03781123057520639</n31>
      <n32>0.000001152907631489201</n32>
      <n33>0.9992848997362292</n33>
      <n34>0.00000133178</n34>
    </skel>
    <skel frame="0" bone="23">
      <n11>0.9999999999971935</n11>
      <n12>-0.000002369179999274673</n12>
      <n13>5.104370882982117e-10</n13>
      <n14>75.8795</n14>
      <n21>0.000002369180000725327</n21>
      <n22>0.9999999999931009</n22>
      <n23>-0.000002860979999399356</n23>
      <n24>-0.0000115313</n24>
      <n31>-5.036589117017883e-10</n31>
      <n32>0.0000028609800006006443</n32>
      <n33>0.9999999999959074</n33>
      <n34>0.0000796435</n34>
    </skel>
    <skel frame="0" bone="24">
      <n11>-0.20943852837591792</n11>
      <n12>-0.29740253445851794</n12>
      <n13>0.9314974224237004</n13>
      <n14>86.0676</n14>
      <n21>0.9770265114134821</n21>
      <n22>-0.025232291358027403</n22>
      <n23>0.21161925179933527</n23>
      <n24>-0.0198744</n24>
      <n31>-0.03943236588829968</n31>
      <n32>0.9544187991833352</n32>
      <n33>0.29585464014283747</n33>
      <n34>-0.449461</n34>
    </skel>
    <skel frame="0" bone="25">
      <n11>0.2219792112955017</n11>
      <n12>0.9585099370269632</n12>
      <n13>-0.17884284715490073</n13>
      <n14>116.259</n14>
      <n21>-0.18854075591703684</n21>
      <n22>-0.13776287994298952</n22>
      <n23>-0.972355383240423</n23>
      <n24>0.00000396065</n24>
      <n31>-0.9566495945309007</n31>
      <n32>0.24956223387957688</n32>
      <n33>0.1501371216085895</n33>
      <n34>0.00000731981</n34>
    </skel>
    <skel frame="0" bone="26">
      <n11>0.9967109332550343</n11>
      <n12>-0.08039885577509866</n12>
      <n13>0.010165605048526361</n13>
      <n14>19.3987</n14>
      <n21>0.08042227941210135</n21>
      <n22>0.996759040769711</n22>
      <n23>-0.0019161482166202683</n23>
      <n24>-0.0000825031</n24>
      <n31>-0.009978602613073637</n31>
      <n32>0.0027273870060997314</n32>
      <n33>0.9999464929935542</n33>
      <n34>-0.00000223841</n34>
    </skel>
    <skel frame="0" bone="27">
      <n11>0.9372005881629543</n11>
      <n12>-0.07231884840940746</n12>
      <n13>0.3412111393966237</n13>
      <n14>133.55</n14>
      <n21>0.05585617416819254</n21>
      <n22>0.9967619693160753</n22>
      <n23>0.05784171789601744</n23>
      <n24>-0.000104766</n24>
      <n31>-0.3442893336873763</n31>
      <n32>-0.03515054319078255</n32>
      <n33>0.9382053580960313</n33>
      <n34>-0.00000592445</n34>
    </skel>
    <skel frame="0" bone="28">
      <n11>0.9995572842239033</n11>
      <n12>-0.0002658744173938321</n12>
      <n13>0.029751720383962777</n13>
      <n14>94.29159999999999</n14>
      <n21>0.0004233949969341678</n21>
      <n22>0.9999859270589604</n22>
      <n23>-0.005288328725658082</n23>
      <n24>-8.9286e-7</n24>
      <n31>-0.029749895658437223</n31>
      <n32>0.005298584228661918</n32>
      <n33>0.9995433300830748</n33>
      <n34>-0.00000734028</n34>
    </skel>
    <skel frame="0" bone="29">
      <n11>0.9884288363350733</n11>
      <n12>0.11437285705447164</n12>
      <n13>-0.09963576207770394</n13>
      <n14>58.9403</n14>
      <n21>-0.05459072227792838</n21>
      <n22>0.8810697358870843</n22>
      <n23>0.46982547152337756</n23>
      <n24>-3.9502700000000006</n24>
      <n31>0.14152133604789607</n31>
      <n32>-0.4589498558886225</n32>
      <n33>0.8771184306438017</n33>
      <n34>1.24541</n34>
    </skel>
    <skel frame="0" bone="30">
      <n11>0.9991358609135628</n11>
      <n12>0.006138824203945268</n12>
      <n13>0.04110773982456839</n13>
      <n14>15.1326</n14>
      <n21>-0.005887557378214732</n21>
      <n22>0.9999632570083192</n22>
      <n23>-0.0062306742343335424</n23>
      <n24>-0.0606808</n24>
      <n31>-0.0411444784170316</n31>
      <n32>0.005983265888226457</n32>
      <n33>0.9991352923526816</n33>
      <n34>0.265579</n34>
    </skel>
    <skel frame="0" bone="31">
      <n11>0.874579844895575</n11>
      <n12>0.48468703565041904</n12>
      <n13>-0.013732583906833361</n13>
      <n14>59.3251</n14>
      <n21>-0.4791310040135809</n21>
      <n22>0.8682098232687785</n22>
      <n23>0.12901626192776877</n23>
      <n24>-0.00000225802</n24>
      <n31>0.07445527101076664</n31>
      <n32>-0.10625531667303122</n32>
      <n33>0.991547387204219</n33>
      <n34>0.00000117555</n34>
    </skel>
    <skel frame="0" bone="32">
      <n11>0.8636279670748025</n11>
      <n12>0.5039622000080176</n12>
      <n13>0.012993135456004704</n13>
      <n14>27.876899999999996</n14>
      <n21>-0.5039927919279822</n21>
      <n22>0.863707223936747</n22>
      <n23>-0.0010407361999710493</n23>
      <n24>0.433149</n24>
      <n31>-0.011746756596475295</n31>
      <n32>-0.005649638066211049</n32>
      <n33>0.9999150440365105</n33>
      <n34>0.449058</n34>
    </skel>
    <skel frame="0" bone="33">
      <n11>0.86368005842238</n11>
      <n12>-0.49763699939532</n12>
      <n13>0.08009028825900001</n13>
      <n14>22.5618</n14>
      <n21>-0.50399373698868</n21>
      <n22>-0.85479905914762</n22>
      <n23>0.12374450128760003</n23>
      <n24>0.747478</n24>
      <n31>0.006881171177400003</n31>
      <n32>-0.14724063758200004</n32>
      <n33>-0.989078834434</n33>
      <n34>0.363798</n34>
    </skel>
    <skel frame="0" bone="34">
      <n11>0.9999896552427545</n11>
      <n12>-0.00004596285320999997</n12>
      <n13>-0.0045483286577176</n13>
      <n14>22.4711</n14>
      <n21>-0.000587850494714</n21>
      <n22>-0.9928686811003753</n22>
      <n23>-0.11921066761643724</n23>
      <n24>-1.03081</n24>
      <n31>-0.0045104143887624</n31>
      <n32>0.11921210814836276</n32>
      <n33>-0.9928584371328704</n33>
      <n34>-0.159869</n34>
    </skel>
    <skel frame="0" bone="35">
      <n11>0.23126786403457655</n11>
      <n12>-0.41283171216467035</n12>
      <n13>0.8809567088275869</n13>
      <n14>15.1326</n14>
      <n21>-0.3812797765774703</n21>
      <n22>0.7946105450879564</n22>
      <n23>0.4724613199084672</n23>
      <n24>-0.0606808</n24>
      <n31>-0.8950645101724128</n31>
      <n32>-0.4451560897795327</n32>
      <n33>0.02636350065916837</n33>
      <n34>0.265579</n34>
    </skel>
    <skel frame="0" bone="36">
      <n11>0.6891311178078751</n11>
      <n12>0.4528604925740001</n12>
      <n13>-0.5656992150102013</n13>
      <n14>22.8164</n14>
      <n21>-0.366709929842</n21>
      <n22>0.8912686845044316</n22>
      <n23>0.2667656896783704</n23>
      <n24>-0.0803448</n24>
      <n31>0.6249976439577988</n31>
      <n32>0.02361101649517039</n32>
      <n33>0.7802694483485623</n33>
      <n34>0.127993</n34>
    </skel>
    <skel frame="0" bone="37">
      <n11>0.9642510168017426</n11>
      <n12>0.25982332031140853</n12>
      <n13>0.05207513320140824</n13>
      <n14>19.2826</n14>
      <n21>-0.2609933042325915</n21>
      <n22>0.9651914027088471</n22>
      <n23>0.016972093082769573</n23>
      <n24>-0.355036</n24>
      <n31>-0.045852725342591764</n31>
      <n32>-0.02995661897163043</n32>
      <n33>0.9984989376994583</n33>
      <n34>0.8286910000000001</n34>
    </skel>
    <skel frame="0" bone="38">
      <n11>0.9623554642291989</n11>
      <n12>0.2600997906880361</n12>
      <n13>0.07886734900528188</n13>
      <n14>24.913500000000003</n14>
      <n21>-0.2631657566399638</n21>
      <n22>0.9642468421135667</n22>
      <n23>0.031173890097531916</n23>
      <n24>-0.588844</n24>
      <n31>-0.06793926984591812</n31>
      <n32>-0.050755549211268076</n32>
      <n33>0.9963975761604366</n33>
      <n34>1.02982</n34>
    </skel>
    <skel frame="0" bone="39">
      <n11>0.922435665872756</n11>
      <n12>0.17953167929324199</n12>
      <n13>-0.34187837724802883</n13>
      <n14>19.3879</n14>
      <n21>-0.33363191638675793</n21>
      <n22>0.8163119198290459</n22>
      <n23>-0.47151310587019685</n23>
      <n24>-0.385906</n24>
      <n31>0.19442786083197117</n31>
      <n32>0.5490020407698032</n32>
      <n33>0.8128927244957382</n33>
      <n34>0.839987</n34>
    </skel>
    <skel frame="0" bone="40">
      <n11>0.9357163699467445</n11>
      <n12>0.06551669947733461</n12>
      <n13>-0.34661569072480297</n13>
      <n14>15.1327</n14>
      <n21>-0.08246046758226537</n21>
      <n22>0.9960023248582255</n22>
      <n23>-0.034345890004816096</n23>
      <n24>-0.060719199999999994</n24>
      <n31>0.342979804439197</n31>
      <n32>0.0607201034559839</n32>
      <n33>0.9373782175502583</n33>
      <n34>0.265571</n34>
    </skel>
    <skel frame="0" bone="41">
      <n11>0.8585620360807886</n11>
      <n12>0.46041909570940903</n12>
      <n13>0.22557810946178336</n13>
      <n14>58.7181</n14>
      <n21>-0.48527318908659095</n21>
      <n22>0.871737381077845</n22>
      <n23>0.06770426596407661</n23>
      <n24>-0.00000285662</n24>
      <n31>-0.16547253281421664</n31>
      <n32>-0.16759532207712338</n32>
      <n33>0.9718696663120905</n33>
      <n34>0.0000034354099999999996</n34>
    </skel>
    <skel frame="0" bone="42">
      <n11>0.8598268296373054</n11>
      <n12>0.506815948802348</n12>
      <n13>-0.061930576952476</n13>
      <n14>21.0153</n14>
      <n21>-0.510381758877652</n21>
      <n22>0.8565657722214011</n22>
      <n23>-0.07619394246293189</n23>
      <n24>0.38992499999999997</n24>
      <n31>0.014431303719523992</n31>
      <n32>0.09712183432506812</n32>
      <n33>0.9951678682796343</n33>
      <n34>0.0424326</n34>
    </skel>
    <skel frame="0" bone="43">
      <n11>0.8595055374354765</n11>
      <n12>0.5105825559374476</n12>
      <n13>0.02357335070580957</n13>
      <n14>17.396</n14>
      <n21>-0.5103827372625523</n21>
      <n22>0.8598287101370606</n22>
      <n23>-0.014285267235267476</n23>
      <n24>0.659931</n24>
      <n31>-0.027562851730190435</n31>
      <n32>0.00024683594073252133</n32>
      <n33>0.9996200419787843</n33>
      <n34>-0.0497939</n34>
    </skel>
    <skel frame="0" bone="44">
      <n11>0.999999999997342</n11>
      <n12>6.947541849373791e-7</n12>
      <n13>-0.0000021984880964931047</n13>
      <n14>19.3961</n14>
      <n21>-3.0293216661462076e-7</n21>
      <n22>0.9848454353367364</n22>
      <n23>0.1734343346213635</n23>
      <n24>0.914351</n24>
      <n31>0.000002285665196466895</n31>
      <n32>-0.17343433462023652</n32>
      <n33>0.9848454353343291</n33>
      <n34>-0.0235857</n34>
    </skel>
    <skel frame="0" bone="45">
      <n11>0.9628023821483094</n11>
      <n12>0.05075212975578468</n12>
      <n13>0.2653974282728731</n13>
      <n14>15.1327</n14>
      <n21>-0.05552522447221531</n21>
      <n22>0.998401989086356</n22>
      <n23>0.010507981009237867</n23>
      <n24>-0.060719199999999994</n24>
      <n31>-0.2644400178791269</n31>
      <n32>-0.024853360795522125</n32>
      <n33>0.9640818368048221</n33>
      <n34>0.265571</n34>
    </skel>
    <skel frame="0" bone="46">
      <n11>0.9146415772664148</n11>
      <n12>0.3561235233581188</n12>
      <n13>-0.19132907008846933</n13>
      <n14>61.5416</n14>
      <n21>-0.31403641868188126</n21>
      <n22>0.9239296208605805</n22>
      <n23>0.21848381208432655</n23>
      <n24>-0.0000502851</n24>
      <n31>0.2545818231755307</n31>
      <n32>-0.13975007875647355</n32>
      <n33>0.9569002072645854</n33>
      <n34>0.000014637100000000001</n34>
    </skel>
    <skel frame="0" bone="47">
      <n11>0.9231879220269112</n11>
      <n12>-0.38415899784137997</n12>
      <n13>-0.01208198836668</n13>
      <n14>23.2843</n14>
      <n21>-0.38386546231062</n21>
      <n22>-0.919995377359089</n22>
      <n23>-0.07910150649284</n23>
      <n24>0.336002</n24>
      <n31>0.019272179701320002</n31>
      <n32>0.07766341392116</n32>
      <n33>-0.9967939920100002</n33>
      <n34>0.668869</n34>
    </skel>
    <skel frame="0" bone="48">
      <n11>0.9225583078058114</n11>
      <n12>-0.38507575458543725</n12>
      <n13>-0.02455274418209999</n13>
      <n14>19.4202</n14>
      <n21>0.38386528656656277</n21>
      <n22>0.9223926972079625</n22>
      <n23>-0.04288541985221686</n23>
      <n24>-0.738948</n24>
      <n31>0.03916140683870001</n31>
      <n32>0.030139354624583142</n32>
      <n33>0.9987782554707151</n33>
      <n34>-0.513733</n34>
    </skel>
    <skel frame="0" bone="49">
      <n11>0.999308659545555</n11>
      <n12>-0.036359743185602006</n12>
      <n13>-0.007757088430040001</n13>
      <n14>18.0525</n14>
      <n21>-0.03631946302239801</n21>
      <n22>-0.9993268865484453</n22>
      <n23>0.0052719060429800004</n23>
      <n24>-0.729436</n24>
      <n31>-0.00794354777156</n31>
      <n32>-0.004986528154780001</n32>
      <n33>-0.9999565778100001</n33>
      <n34>-0.452573</n34>
    </skel>
    <skel frame="0" bone="50">
      <n11>0.9846719710579575</n11>
      <n12>0.01927813878550702</n12>
      <n13>-0.1733478086468301</n13>
      <n14>15.1327</n14>
      <n21>-0.02500787712169298</n21>
      <n22>0.9992086530426638</n22>
      <n23>-0.03093014331925847</n23>
      <n24>-0.060719199999999994</n24>
      <n31>0.17261435478996992</n31>
      <n32>0.03479110588714154</n32>
      <n33>0.9843748592158137</n33>
      <n34>0.265571</n34>
    </skel>
    <skel frame="0" bone="51">
      <n11>0.869206968297648</n11>
      <n12>0.49443044257130403</n12>
      <n13>0.004218738856291653</n13>
      <n14>59.410399999999996</n14>
      <n21>-0.49444131663669594</n21>
      <n22>0.8691170057058238</n22>
      <n23>0.012783929018474945</n23>
      <n24>-0.0000224637</n24>
      <n31>0.0026541856309876526</n31>
      <n32>-0.013197798957525054</n32>
      <n33>0.9999093825997621</n33>
      <n34>0.00000755867</n34>
    </skel>
    <skel frame="0" bone="52">
      <n11>0.9554067139041207</n11>
      <n12>0.29529027958429666</n12>
      <n13>0.0012895007900379598</n13>
      <n14>25.6046</n14>
      <n21>-0.2952786823357034</n21>
      <n22>0.9553078284028469</n22>
      <n23>0.014051823898703747</n23>
      <n24>0.36117</n24>
      <n31>0.0029174967609819596</n31>
      <n32>-0.013805968992336251</n32>
      <n33>0.9999004367611158</n33>
      <n34>0.201343</n34>
    </skel>
    <skel frame="0" bone="53">
      <n11>0.86915459930968</n11>
      <n12>-0.49334613588144005</n12>
      <n13>0.0343475730672</n13>
      <n14>20.4208</n14>
      <n21>-0.49444059083856007</n21>
      <n22>-0.8682749629223199</n22>
      <n23>0.040316513381599986</n23>
      <n24>0.7427700000000002</n24>
      <n31>0.0099331614288</n31>
      <n32>-0.05202412258639999</n32>
      <n33>-0.9985955321679998</n33>
      <n34>0.154036</n34>
    </skel>
    <skel frame="0" bone="54">
      <n11>0.9999999999971617</n11>
      <n12>-0.0000023422345185912</n12>
      <n13>-4.3651661172879994e-7</n13>
      <n14>20.5295</n14>
      <n21>-0.0000023351451558088003</n21>
      <n22>-0.9998800018001035</n22>
      <n23>0.015588772323331786</n23>
      <n24>-0.899507</n24>
      <n31>-4.729761026711999e-7</n31>
      <n32>-0.015588772322268213</n32>
      <n33>-0.999880001802735</n33>
      <n34>-0.0859665</n34>
    </skel>
    <skel frame="0" bone="55">
      <n11>-0.20931703186600004</n11>
      <n12>-0.29743617768599995</n12>
      <n13>-0.931514721048</n13>
      <n14>86.0676</n14>
      <n21>-0.977052294138</n21>
      <n22>0.02517827181400001</n22>
      <n23>0.2115091933440001</n23>
      <n24>-0.0198744</n24>
      <n31>-0.039455922815999966</n31>
      <n32>0.954410312472</n32>
      <n33>-0.29588151937600005</n33>
      <n34>-0.449461</n34>
    </skel>
    <skel frame="0" bone="56">
      <n11>0.22848330967270458</n11>
      <n12>0.9608347907985855</n12>
      <n13>0.15685953936957175</n13>
      <n14>116.25899999999999</n14>
      <n21>-0.19310941345741361</n21>
      <n22>-0.11319846823285218</n22>
      <n23>0.9746351452183315</n23>
      <n24>-0.00000903036</n24>
      <n31>0.9542102308895715</n31>
      <n32>-0.25298501270166757</n32>
      <n33>0.15967346086991474</n33>
      <n34>0.00000990724</n34>
    </skel>
    <skel frame="0" bone="57">
      <n11>0.6362938728162362</n11>
      <n12>0.05797716760892385</n12>
      <n13>-0.7692650748875568</n13>
      <n14>19.3986</n14>
      <n21>0.1984339109816508</n21>
      <n22>0.951318777817199</n22>
      <n23>0.23583164745951907</n23>
      <n24>0.00000583116</n24>
      <n31>0.7454891618113827</n31>
      <n32>-0.30270650968717433</n32>
      <n33>0.5938136733039286</n33>
      <n34>0.00000270719</n34>
    </skel>
    <skel frame="0" bone="58">
      <n11>0.7200040285797683</n11>
      <n12>0.059534238426337066</n12>
      <n13>-0.6914115080819168</n13>
      <n14>133.503</n14>
      <n21>-0.41017179747427046</n21>
      <n22>0.8401681051903406</n22>
      <n23>-0.35479099721608814</n23>
      <n24>0.00000160413</n24>
      <n31>0.5597796848849024</n31>
      <n32>0.5390484482899994</n32>
      <n33>0.6293436859801732</n33>
      <n34>0.00000780019</n34>
    </skel>
    <skel frame="0" bone="59">
      <n11>0.9986511639034623</n11>
      <n12>0.019597230760861905</n12>
      <n13>-0.04808119570856786</n13>
      <n14>94.0833</n14>
      <n21>-0.021501079340733326</n21>
      <n22>0.9989919981555586</n22>
      <n23>-0.03940420292745342</n23>
      <n24>-0.332509</n24>
      <n31>0.04726051651689354</n31>
      <n32>0.04038485071991682</n32>
      <n33>0.998065883301742</n33>
      <n34>-1.27727</n34>
    </skel>
    <skel frame="0" bone="60">
      <n11>0.20210260794304424</n11>
      <n12>-0.9691673226267402</n12>
      <n13>0.13344506923895694</n13>
      <n14>64.8218</n14>
      <n21>0.38426447795644847</n21>
      <n22>-0.04584899237062112</n22>
      <n23>-0.920617379867517</n23>
      <n24>0.0799684</n24>
      <n31>0.8996853320647903</n31>
      <n32>0.23645348355604784</n32>
      <n33>0.36471161367335936</n33>
      <n34>0.2182</n34>
    </skel>
    <skel frame="0" bone="61">
      <n11>0.9991944089997378</n11>
      <n12>0.009602040696115602</n12>
      <n13>-0.038965803445261335</n13>
      <n14>15.1326</n14>
      <n21>-0.009368850495084402</n21>
      <n22>0.9999371210121846</n22>
      <n23>0.0061626830410474245</n23>
      <n24>-0.06066269999999999</n24>
      <n31>0.03902252764833869</n31>
      <n32>-0.005792653652152577</n32>
      <n33>0.9992215407503987</n33>
      <n34>-0.265564</n34>
    </skel>
    <skel frame="0" bone="62">
      <n11>0.9333983258945564</n11>
      <n12>0.3575596303213915</n12>
      <n13>0.03030966812205645</n13>
      <n14>61.3737</n14>
      <n21>-0.3508836007786795</n21>
      <n22>0.9271260571584166</n22>
      <n23>-0.13159776913943722</n23>
      <n24>-0.00000899927</n24>
      <n31>-0.07515493278439934</n31>
      <n32>0.1121979719171351</n32>
      <n33>0.9908397202251503</n33>
      <n34>0.0000137974</n34>
    </skel>
    <skel frame="0" bone="63">
      <n11>0.9936945446887809</n11>
      <n12>-0.11157606210111781</n12>
      <n13>-0.011042382974880636</n13>
      <n14>27.876999999999995</n14>
      <n21>0.11150416629844137</n21>
      <n22>0.9937398215501348</n22>
      <n23>-0.006927334523153838</n23>
      <n24>0.433203</n24>
      <n31>0.011746180393896759</n31>
      <n32>0.005652382817330057</n32>
      <n33>0.9999150352978199</n33>
      <n34>-0.449036</n34>
    </skel>
    <skel frame="0" bone="64">
      <n11>0.9528495025506283</n11>
      <n12>-0.2991004306517698</n12>
      <n13>-0.051155165918046835</n13>
      <n14>22.5617</n14>
      <n21>-0.30336540005686524</n21>
      <n22>-0.9427953830811384</n22>
      <n23>-0.13823941881031812</n23>
      <n24>0.747526</n24>
      <n31>-0.006881327234999998</n31>
      <n32>0.14724005993140007</n32>
      <n33>-0.989078834434</n33>
      <n34>-0.363771</n34>
    </skel>
    <skel frame="0" bone="65">
      <n11>0.9999896591067864</n11>
      <n12>-0.00004670490768199988</n12>
      <n13>0.00454747149939</n13>
      <n14>22.4711</n14>
      <n21>-0.000588485974182</n21>
      <n22>-0.9928686770170483</n22>
      <n23>0.11921086583678046</n23>
      <n24>-1.03086</n24>
      <n31>0.00450947476369</n31>
      <n32>-0.11921230921321958</n32>
      <n33>-0.9928584373521656</n33>
      <n34>0.159828</n34>
    </skel>
    <skel frame="0" bone="66">
      <n11>0.2312682827646837</n11>
      <n12>-0.41283149588752177</n12>
      <n13>-0.8809567126698623</n13>
      <n14>15.1326</n14>
      <n21>-0.38127956030032184</n21>
      <n22>0.7946106569640501</n22>
      <n23>-0.4724613124718573</n23>
      <n24>-0.06066269999999999</n24>
      <n31>0.8950645063301375</n31>
      <n32>0.4451560972161426</n32>
      <n33>0.02636403100113882</n33>
      <n34>-0.265564</n34>
    </skel>
    <skel frame="0" bone="67">
      <n11>0.5487682727281442</n11>
      <n12>0.35228161323670504</n12>
      <n13>0.7581233502739595</n13>
      <n14>22.8165</n14>
      <n21>-0.22693304692118796</n21>
      <n22>0.9355961727683327</n22>
      <n23>-0.27048325072323076</n23>
      <n24>-0.0803393</n24>
      <n31>-0.8045835826502639</n31>
      <n32>-0.02361062646531728</n32>
      <n33>0.5933698396246809</n33>
      <n34>-0.127965</n34>
    </skel>
    <skel frame="0" bone="68">
      <n11>0.9642510268638071</n11>
      <n12>0.2598233226822133</n12>
      <n13>-0.05207493499535348</n13>
      <n14>19.2826</n14>
      <n21>-0.2609933018617867</n21>
      <n22>0.9651914028839947</n22>
      <n23>-0.016972119394050633</n23>
      <n24>-0.355146</n24>
      <n31>0.04585252716784652</n31>
      <n32>0.02995659266034937</n32>
      <n33>0.9984989475891964</n33>
      <n34>-0.828729</n34>
    </skel>
    <skel frame="0" bone="69">
      <n11>0.962355464541777</n11>
      <n12>0.2600997907007651</n12>
      <n13>-0.07886734895991171</n13>
      <n14>24.9134</n14>
      <n21>-0.2631657566272348</n21>
      <n22>0.9642468424104399</n22>
      <n23>-0.03117389017882921</n23>
      <n24>-0.588824</n24>
      <n31>0.06793926989128829</n31>
      <n32>0.050755549129970795</n32>
      <n33>0.996397576190349</n33>
      <n34>-1.0299</n34>
    </skel>
    <skel frame="0" bone="70">
      <n11>0.922435665872756</n11>
      <n12>0.17953167929324199</n12>
      <n13>0.34187837724802883</n13>
      <n14>19.3878</n14>
      <n21>-0.33363191638675793</n21>
      <n22>0.8163119198290459</n22>
      <n23>0.47151310587019685</n23>
      <n24>-0.3859020000000001</n24>
      <n31>-0.19442786083197117</n31>
      <n32>-0.5490020407698032</n32>
      <n33>0.8128927244957382</n33>
      <n34>-0.8399089999999999</n34>
    </skel>
    <skel frame="0" bone="71">
      <n11>0.9271285744813667</n11>
      <n12>0.04103985599618317</n12>
      <n13>0.37248937783507463</n13>
      <n14>15.1326</n14>
      <n21>-0.07880055482407755</n21>
      <n22>0.993111609206727</n22>
      <n23>0.08671680470498586</n23>
      <n24>-0.0606627</n24>
      <n31>-0.3663646802566617</n31>
      <n32>-0.10974999716919204</n32>
      <n33>0.9239761139671284</n33>
      <n34>-0.265564</n34>
    </skel>
    <skel frame="0" bone="72">
      <n11>0.9998694094575087</n11>
      <n12>0.011920694582218004</n12>
      <n13>-0.010911510975587124</n13>
      <n14>60.7423</n14>
      <n21>-0.012054854205990504</n21>
      <n22>0.9998515220948352</n22>
      <n23>-0.012313174030834408</n23>
      <n24>-0.0000162952</n24>
      <n31>0.01076310927033601</n31>
      <n32>0.012443102720735698</n32>
      <n33>0.9998646531773768</n33>
      <n34>0.00000293244</n34>
    </skel>
    <skel frame="0" bone="73">
      <n11>0.9878018243298317</n11>
      <n12>0.15294437347711304</n12>
      <n13>0.02925020465483376</n13>
      <n14>21.0153</n14>
      <n21>-0.15504619077524195</n21>
      <n22>0.9834505456211812</n22>
      <n23>0.09373208118615704</n23>
      <n24>0.389774</n24>
      <n31>-0.014430335295604788</n31>
      <n32>-0.09712385360504604</n32>
      <n33>0.9951676856109035</n33>
      <n34>-0.0424223</n34>
    </skel>
    <skel frame="0" bone="74">
      <n11>0.9888432087074874</n11>
      <n12>0.14357424802402002</n12>
      <n13>-0.03969312153783922</n13>
      <n14>17.3959</n14>
      <n21>-0.14607754451802357</n21>
      <n22>0.9868147089431176</n22>
      <n23>-0.06969993688013638</n23>
      <n24>0.659873</n24>
      <n31>0.029162640152519365</n31>
      <n32>0.07472058295976643</n32>
      <n33>0.9967779967984282</n33>
      <n34>0.0498085</n34>
    </skel>
    <skel frame="0" bone="75">
      <n11>0.9999999999931338</n11>
      <n12>-0.000002397947163323166</n12>
      <n13>0.0000028252622630902695</n13>
      <n14>19.396</n14>
      <n21>0.0000028516095869168337</n21>
      <n22>0.9848450844825081</n22>
      <n23>-0.17343632702536713</n23>
      <n24>0.9145230000000001</n24>
      <n31>-0.0000023665545033097317</n31>
      <n32>0.17343632703223286</n32>
      <n33>0.984845084482584</n33>
      <n34>0.023592600000000002</n34>
    </skel>
    <skel frame="0" bone="76">
      <n11>0.9637115118316273</n11>
      <n12>0.05454876194617554</n12>
      <n13>-0.2613131350887343</n13>
      <n14>15.1326</n14>
      <n21>-0.059428304681024446</n21>
      <n22>0.9981741350340212</n22>
      <n23>-0.010801516266834376</n23>
      <n24>-0.0606627</n24>
      <n31>0.2602468032512657</n31>
      <n32>0.02593894217316562</n32>
      <n33>0.9651936451881069</n33>
      <n34>-0.265564</n34>
    </skel>
    <skel frame="0" bone="77">
      <n11>0.9818870023506546</n11>
      <n12>0.07625786467845537</n12>
      <n13>0.17344351440606817</n13>
      <n14>63.5726</n14>
      <n21>-0.04980785576019879</n21>
      <n22>0.987118946114969</n22>
      <n23>-0.15203737608050236</n23>
      <n24>0.00000669457</n24>
      <n31>-0.18280342480220882</n31>
      <n32>0.14064467389686416</n32>
      <n33>0.9730375036888496</n33>
      <n34>0.0000188241</n34>
    </skel>
    <skel frame="0" bone="78">
      <n11>0.953952804528541</n11>
      <n12>-0.2999163798571939</n12>
      <n13>0.004923783907865462</n13>
      <n14>23.2844</n14>
      <n21>-0.2993370393964353</n21>
      <n22>-0.9507995641640572</n22>
      <n23>0.07986785208944797</n23>
      <n24>0.336163</n24>
      <n31>-0.01927214051404</n31>
      <n32>-0.07766403315151996</n32>
      <n33>-0.9967939920099995</n33>
      <n34>-0.668896</n34>
    </skel>
    <skel frame="0" bone="79">
      <n11>0.9915237793374595</n11>
      <n12>-0.1250936371247201</n12>
      <n13>0.035102378256604616</n13>
      <n14>19.4204</n14>
      <n21>0.12388291308386937</n21>
      <n22>0.9916871040519241</n22>
      <n23>0.03478090716129922</n23>
      <n24>-0.73887</n24>
      <n31>-0.0391614460179316</n31>
      <n32>-0.030137511642756652</n32>
      <n33>0.998778309505051</n33>
      <n34>0.513678</n34>
    </skel>
    <skel frame="0" bone="80">
      <n11>0.999308637737775</n11>
      <n12>-0.036360330831350006</n12>
      <n13>0.007757143591700001</n13>
      <n14>18.0523</n14>
      <n21>-0.03632007516545</n21>
      <n22>-0.9993268865484453</n22>
      <n23>-0.00526878894761</n23>
      <n24>-0.729443</n24>
      <n31>0.0079434926099</n31>
      <n32>0.004983406348389999</n32>
      <n33>-0.9999565996177802</n33>
      <n34>0.452609</n34>
    </skel>
    <skel frame="0" bone="81">
      <n11>0.9845292432219781</n11>
      <n12>0.020918144497308725</n12>
      <n13>0.17396723965034397</n13>
      <n14>15.1326</n14>
      <n21>-0.026700392367491266</n21>
      <n22>0.9991638205460558</n22>
      <n23>0.0309636683749158</n23>
      <n24>-0.0606627</n24>
      <n31>-0.17317406932965604</n31>
      <n32>-0.03512963055028418</n32>
      <n33>0.9842645227626665</n33>
      <n34>-0.265564</n34>
    </skel>
    <skel frame="0" bone="82">
      <n11>0.976890819213879</n11>
      <n12>0.20915389671174173</n12>
      <n13>0.044033791864096256</n13>
      <n14>61.4674</n14>
      <n21>-0.2094217170541045</n21>
      <n22>0.9778242534199149</n22>
      <n23>0.001507929009189138</n23>
      <n24>-0.0000444905</n24>
      <n31>-0.04274192042652125</n31>
      <n32>-0.010694714305685245</n32>
      <n33>0.99902890414851</n33>
      <n34>0.00000944973</n34>
    </skel>
    <skel frame="0" bone="83">
      <n11>0.9919182557021813</n11>
      <n12>0.12687496000473855</n12>
      <n13>0.0009583989451746713</n13>
      <n14>25.6045</n14>
      <n21>-0.12685081306119692</n21>
      <n22>0.9918309089065925</n22>
      <n23>-0.01342830455552117</n23>
      <n24>0.361201</n24>
      <n31>-0.002654285300300911</n31>
      <n32>0.013198206746317814</n32>
      <n33>0.9999093769478442</n33>
      <n34>-0.20137</n34>
    </skel>
    <skel frame="0" bone="84">
      <n11>0.9891339807657489</n11>
      <n12>-0.14597831165616137</n12>
      <n13>-0.017443931786009798</n13>
      <n14>20.4207</n14>
      <n21>-0.14668092363635032</n21>
      <n22>-0.9879180350395746</n22>
      <n23>-0.05000857943907924</n23>
      <n24>0.742829</n24>
      <n31>-0.009933019245199997</n31>
      <n32>0.052023878194000006</n32>
      <n33>-0.998595532168</n33>
      <n34>-0.154055</n34>
    </skel>
    <skel frame="1" bone="59">
      <n11>0.9986511639034638</n11>
      <n12>0.019597230760853655</n12>
      <n13>-0.04808119570854187</n13>
      <n14>94.0833</n14>
      <n21>-0.04732749481174482</n21>
      <n22>0.7244187560347483</n22>
      <n23>-0.6877336505798681</n23>
      <n24>-0.332509</n24>
      <n31>0.021353244931427203</n31>
      <n32>0.6890815731476019</n32>
      <n33>0.7243691217047595</n33>
      <n34>-1.2772699999999997</n34>
    </skel>
    <skel frame="1" bone="60">
      <n11>0.162929277717219</n11>
      <n12>-0.9726297990108314</n12>
      <n13>0.16566630477768957</n13>
      <n14>64.8218</n14>
      <n21>0.9150498525174352</n21>
      <n22>0.08616962393030725</n22>
      <n23>-0.39402863261383503</n23>
      <n24>0.0799684</n24>
      <n31>0.3689685865630901</n31>
      <n32>0.21579172826560886</n32>
      <n33>0.904044308727054</n33>
      <n34>0.2182</n34>
    </skel>
    <skel frame="1" bone="62">
      <n11>0.9413408912235873</n11>
      <n12>0.3013220141598959</n12>
      <n13>0.15192883298770443</n13>
      <n14>61.37369999999999</n14>
      <n21>-0.27665766360282684</n21>
      <n22>0.9468984418723254</n22>
      <n23>-0.16384101424731043</n23>
      <n24>-0.00000899927</n24>
      <n31>-0.19323007964653788</n31>
      <n32>0.11219797040225728</n32>
      <n33>0.9747172675998957</n33>
      <n34>0.000013797399999999998</n34>
    </skel>
    <skel frame="1" bone="63">
      <n11>0.973200115774743</n11>
      <n12>-0.22973646434546183</n12>
      <n13>-0.010133686697018678</n13>
      <n14>27.87700000000001</n14>
      <n21>0.22965966537835883</n21>
      <n22>0.9732364602286141</n22>
      <n23>-0.008199425588263604</n23>
      <n24>0.43320300000000006</n24>
      <n31>0.011746180414383644</n31>
      <n32>0.005652382835898094</n32>
      <n33>0.9999150352974743</n33>
      <n34>-0.44903599999999994</n34>
    </skel>
    <skel frame="1" bone="64">
      <n11>0.9497682569832211</n11>
      <n12>-0.3085132625785384</n12>
      <n13>-0.05253497932938088</n13>
      <n14>22.5617</n14>
      <n21>-0.3128785780066726</n21>
      <n22>-0.9397572892974968</n22>
      <n23>-0.137720963763615</n23>
      <n24>0.747526</n24>
      <n31>-0.006881327234999998</n31>
      <n32>0.14724005993140007</n32>
      <n33>-0.9890788344340005</n33>
      <n34>-0.363771</n34>
    </skel>
    <skel frame="1" bone="71">
      <n11>0.9271285744813667</n11>
      <n12>0.041039855996183164</n12>
      <n13>0.3724893778350747</n13>
      <n14>15.1326</n14>
      <n21>-0.07880055482407755</n21>
      <n22>0.993111609206727</n22>
      <n23>0.08671680470498586</n23>
      <n24>-0.0606627</n24>
      <n31>-0.36636468025666175</n31>
      <n32>-0.10974999716919204</n32>
      <n33>0.9239761139671283</n33>
      <n34>-0.265564</n34>
    </skel>
    <skel frame="1" bone="72">
      <n11>0.9999237944102831</n11>
      <n12>-0.008077472295508056</n12>
      <n13>0.00933594202302005</n13>
      <n14>60.7423</n14>
      <n21>0.008192670483373224</n21>
      <n22>0.9998899558615076</n22>
      <n23>-0.012367550995376805</n23>
      <n24>-0.0000162952</n24>
      <n31>-0.00923501610679467</n31>
      <n32>0.01244309481550633</n32>
      <n33>0.9998799347266248</n33>
      <n34>0.00000293244</n34>
    </skel>
    <skel frame="1" bone="73">
      <n11>0.9722520884491649</n11>
      <n12>0.2310473063804558</n12>
      <n13>0.03664721982051361</n13>
      <n14>21.0153</n14>
      <n21>-0.233490132280762</n21>
      <n22>0.9680826929636203</n22>
      <n23>0.0910947732409341</n23>
      <n24>0.389774</n24>
      <n31>-0.014430337270816327</n31>
      <n32>-0.09712384773391516</n32>
      <n33>0.9951676861552579</n33>
      <n34>-0.0424223</n34>
    </skel>
    <skel frame="1" bone="74">
      <n11>0.9959381852498039</n11>
      <n12>0.08575668316318796</n12>
      <n13>-0.02743943246817823</n13>
      <n14>17.3959</n14>
      <n21>-0.08571740379731652</n21>
      <n22>0.9963160802234486</n22>
      <n23>0.002606717175019483</n23>
      <n24>0.659873</n24>
      <n31>0.027561891219125546</n31>
      <n32>-0.00024409225990436908</n32>
      <n33>0.999620069111957</n33>
      <n34>0.0498085</n34>
    </skel>
    <skel frame="1" bone="77">
      <n11>0.9677135915315889</n11>
      <n12>0.007029167325980974</n12>
      <n13>0.25195435215875234</n13>
      <n14>63.5726</n14>
      <n21>0.02870301780118887</n21>
      <n22>0.9900351845962794</n22>
      <n23>-0.1378639547906404</n23>
      <n24>0.00000669457</n24>
      <n31>-0.2504127423557712</n31>
      <n32>0.14064467308829887</n32>
      <n33>0.9578687459134199</n33>
      <n34>0.0000188241</n34>
    </skel>
    <skel frame="1" bone="78">
      <n11>0.953952804528541</n11>
      <n12>-0.2999163798571939</n12>
      <n13>0.004923783907865467</n13>
      <n14>23.2844</n14>
      <n21>-0.2993370393964353</n21>
      <n22>-0.9507995641640583</n22>
      <n23>0.07986785208944824</n23>
      <n24>0.336163</n24>
      <n31>-0.019272140514040064</n31>
      <n32>-0.07766403315152023</n32>
      <n33>-0.9967939920100006</n33>
      <n34>-0.668896</n34>
    </skel>
    <skel frame="1" bone="79">
      <n11>0.9964761985599025</n11>
      <n12>-0.07537359621461759</n12>
      <n13>0.03679682998957222</n13>
      <n14>19.4204</n14>
      <n21>0.07417254812496163</n21>
      <n22>0.9966998301375932</n22>
      <n23>0.0329830518348904</n23>
      <n24>-0.73887</n24>
      <n31>-0.03916144543113735</n31>
      <n32>-0.030137511466088226</n32>
      <n33>0.9987783095333896</n33>
      <n34>0.513678</n34>
    </skel>
    <skel frame="1" bone="82">
      <n11>0.9545067595739908</n11>
      <n12>0.2961928202277155</n12>
      <n13>0.034447919721108736</n13>
      <n14>61.467400000000005</n14>
      <n21>-0.296385400521834</n21>
      <n22>0.9550682888314395</n22>
      <n23>0.0005079626936660411</n23>
      <n24>-0.00004449049999999999</n24>
      <n31>-0.0327496608390347</n31>
      <n32>-0.010694714308400445</n32>
      <n33>0.9994063651992566</n33>
      <n34>0.00000944973</n34>
    </skel>
    <skel frame="1" bone="83">
      <n11>0.9730194174351006</n11>
      <n12>-0.23065460231919285</n12>
      <n13>0.005627408216274459</n13>
      <n14>25.6045</n14>
      <n21>0.23070797139215873</n21>
      <n22>0.9729461761929039</n22>
      <n23>-0.012229888295555628</n23>
      <n24>0.361201</n24>
      <n31>-0.0026542852846812297</n31>
      <n32>0.01319820671841014</n32>
      <n33>0.9999093769482541</n33>
      <n34>-0.20137</n34>
    </skel>
    <skel frame="1" bone="84">
      <n11>0.9531311168252746</n11>
      <n12>-0.3015071739883931</n12>
      <n13>-0.025188402580052362</n13>
      <n14>20.4207</n14>
      <n21>-0.3023943893654054</n21>
      <n22>-0.9520426293146222</n22>
      <n23>-0.046590698096668125</n23>
      <n24>0.742829</n24>
      <n31>-0.009933019245200004</n31>
      <n32>0.05202387819400001</n32>
      <n33>-0.9985955321679998</n33>
      <n34>-0.154055</n34>
    </skel>
    <skel frame="2" bone="60">
      <n11>0.20379409815510585</n11>
      <n12>-0.9684059330251836</n12>
      <n13>0.14372861384835903</n13>
      <n14>64.8218</n14>
      <n21>0.895385647755559</n21>
      <n22>0.12499551549461829</n22>
      <n23>-0.4273881875995088</n23>
      <n24>0.0799684</n24>
      <n31>0.39591982439694196</n31>
      <n32>0.21579172826560866</n32>
      <n33>0.8925701219857387</n33>
      <n34>0.2182</n34>
    </skel>
    <skel frame="2" bone="81">
      <n11>0.9845292432219781</n11>
      <n12>0.020918144497308718</n12>
      <n13>0.1739672396503439</n13>
      <n14>15.1326</n14>
      <n21>-0.02670039236749126</n21>
      <n22>0.9991638205460558</n22>
      <n23>0.030963668374915796</n23>
      <n24>-0.0606627</n24>
      <n31>-0.17317406932965598</n31>
      <n32>-0.035129630550284174</n32>
      <n33>0.9842645227626665</n33>
      <n34>-0.265564</n34>
    </skel>
    <skel frame="3" bone="60">
      <n11>0.9934353662381595</n11>
      <n12>-0.10955243290964455</n12>
      <n13>-0.03293079942594418</n13>
      <n14>64.8218</n14>
      <n21>0.09968835430138716</n21>
      <n22>0.9702742882585932</n22>
      <n23>-0.22052219289894961</n23>
      <n24>0.0799684</n24>
      <n31>0.05611065071744435</n31>
      <n32>0.21579172826560863</n32>
      <n33>0.9748258946541206</n33>
      <n34>0.2182</n34>
    </skel>
    <skel frame="4" bone="27">
      <n11>0.7813220878275189</n11>
      <n12>-0.07970479641072169</n12>
      <n13>0.6190177215616695</n13>
      <n14>133.55</n14>
      <n21>0.055856173567230945</n21>
      <n22>0.9967619694177926</n22>
      <n23>0.05784171663725582</n23>
      <n24>-0.000104766</n24>
      <n31>-0.6216235855022736</n31>
      <n32>-0.010617049413214216</n32>
      <n33>0.7832441479655918</n33>
      <n34>-0.000005924449999999999</n34>
    </skel>
    <skel frame="4" bone="60">
      <n11>0.9889382150703325</n11>
      <n12>-0.14826542541415766</n12>
      <n13>-0.004309338726954794</n13>
      <n14>64.8218</n14>
      <n21>0.14374052499024403</n21>
      <n22>0.9651172434678084</n22>
      <n23>-0.21883182546564894</n23>
      <n24>0.0799684</n24>
      <n31>0.036604210810148835</n31>
      <n32>0.2157917282656081</n32>
      <n33>0.9757530741755868</n33>
      <n34>0.21820000000000003</n34>
    </skel>
    <skel frame="4" bone="62">
      <n11>0.8835370916088191</n11>
      <n12>0.44205943521837626</n12>
      <n13>0.15474386412985622</n13>
      <n14>61.37369999999999</n14>
      <n21>-0.4123200501351198</n21>
      <n22>0.8908550808617072</n22>
      <n23>-0.19070763267224894</n23>
      <n24>-0.00000899927</n24>
      <n31>-0.22215846598318403</n31>
      <n32>0.10469326930271734</n32>
      <n33>0.9693734756814358</n33>
      <n34>0.000013797399999999998</n34>
    </skel>
    <skel frame="4" bone="64">
      <n11>0.9732382206422474</n11>
      <n12>0.22818349550057643</n12>
      <n13>0.0271976307710773</n13>
      <n14>22.5617</n14>
      <n21>0.22969559437199982</n21>
      <n22>-0.9624222218555538</n22>
      <n23>-0.14486986157775392</n23>
      <n24>0.747526</n24>
      <n31>-0.006881327235000019</n31>
      <n32>0.14724005993140007</n32>
      <n33>-0.9890788344340005</n33>
      <n34>-0.363771</n34>
    </skel>
    <skel frame="4" bone="72">
      <n11>0.9653322281013041</n11>
      <n12>0.25486127813235926</n12>
      <n13>0.05638633077007227</n13>
      <n14>60.7423</n14>
      <n21>-0.25002587105807905</n21>
      <n22>0.9648706095256315</n22>
      <n23>-0.08069554309430671</n23>
      <n24>-0.0000162952</n24>
      <n31>-0.07497168259163339</n31>
      <n32>0.06379996694651563</n32>
      <n33>0.9951426083868598</n33>
      <n34>0.00000293244</n34>
    </skel>
    <skel frame="4" bone="73">
      <n11>0.9963443055055046</n11>
      <n12>-0.08520815670072168</n12>
      <n13>0.006131469515466075</n13>
      <n14>21.0153</n14>
      <n21>0.0842008922339759</n21>
      <n22>0.9916181362980939</n22>
      <n23>0.09799837504620833</n23>
      <n24>0.3897739999999999</n24>
      <n31>-0.014430337271048458</n31>
      <n32>-0.09712384772217474</n32>
      <n33>0.9951676861564003</n33>
      <n34>-0.0424223</n34>
    </skel>
    <skel frame="4" bone="77">
      <n11>0.8593341345859885</n11>
      <n12>0.47715460700657264</n12>
      <n13>0.1840334919186002</n13>
      <n14>63.5726</n14>
      <n21>-0.4304200298269727</n21>
      <n22>0.8691298529773946</n22>
      <n23>-0.24362244680496978</n23>
      <n24>0.00000669457</n24>
      <n31>-0.2761945746373345</n31>
      <n32>0.13014138341010387</n32>
      <n33>0.9522498502310233</n33>
      <n34>0.0000188241</n34>
    </skel>
    <skel frame="4" bone="79">
      <n11>0.9614961540544924</n11>
      <n12>0.27096228689221324</n12>
      <n13>0.04587575416886167</n13>
      <n14>19.4204</n14>
      <n21>-0.27201383591678563</n21>
      <n22>0.9621180642128976</n22>
      <n23>0.01836582655539141</n23>
      <n24>-0.73887</n24>
      <n31>-0.03916144543113736</n31>
      <n32>-0.03013751146608824</n32>
      <n33>0.9987783095333896</n33>
      <n34>0.513678</n34>
    </skel>
    <skel frame="4" bone="82">
      <n11>0.957803933615308</n11>
      <n12>0.26723593826209147</n12>
      <n13>0.10581388402389465</n13>
      <n14>61.467400000000005</n14>
      <n21>-0.25770202974088796</n21>
      <n22>0.9614840706490736</n22>
      <n23>-0.09559312608924243</n23>
      <n24>-0.0000444905</n24>
      <n31>-0.12728428268434833</n31>
      <n32>0.06429101950713596</n32>
      <n33>0.989780468685993</n33>
      <n34>0.00000944973</n34>
    </skel>
    <skel frame="4" bone="84">
      <n11>0.9992581371837048</n11>
      <n12>-0.036644019957676285</n12>
      <n13>-0.011848645492942095</n13>
      <n14>20.4207</n14>
      <n21>-0.03720900062283965</n21>
      <n22>-0.9979724034782946</n22>
      <n23>-0.05162129767501246</n23>
      <n24>0.742829</n24>
      <n31>-0.009933019245199997</n31>
      <n32>0.05202387819400002</n32>
      <n33>-0.9985955321679996</n33>
      <n34>-0.154055</n34>
    </skel>
    <skel frame="5" bone="60">
      <n11>0.982113713803157</n11>
      <n12>-0.1867412248662803</n12>
      <n13>0.02409083010632441</n13>
      <n14>64.8218</n14>
      <n21>0.18751218584187707</n21>
      <n22>0.9584162169681724</n22>
      <n23>-0.21512214022089507</n23>
      <n24>0.0799684</n24>
      <n31>0.017083129706579252</n31>
      <n32>0.21579172826560897</n32>
      <n33>0.9762899654772499</n33>
      <n34>0.2182000000000001</n34>
    </skel>
    <skel frame="5" bone="62">
      <n11>0.9046982877402676</n11>
      <n12>0.38520834178813046</n12>
      <n13>0.1820317048666711</n13>
      <n14>61.37369999999999</n14>
      <n21>-0.3508562543519008</n21>
      <n22>0.9159837055608203</n22>
      <n23>-0.1946117671910483</n23>
      <n24>-0.00000899927</n24>
      <n31>-0.24170415168544843</n31>
      <n32>0.11219797040903809</n32>
      <n33>0.9638416459637498</n33>
      <n34>0.0000137974</n34>
    </skel>
    <skel frame="5" bone="63">
      <n11>0.9903741810325947</n11>
      <n12>0.13073259561736175</n12>
      <n13>0.045474938011664606</n13>
      <n14>27.87700000000001</n14>
      <n21>-0.13607444558391948</n21>
      <n22>0.9797499643834626</n22>
      <n23>0.1468800617838676</n23>
      <n24>0.43320300000000006</n24>
      <n31>-0.0253520571758251</n31>
      <n32>-0.15165419787711515</n32>
      <n33>0.9881084340613648</n33>
      <n34>-0.449036</n34>
    </skel>
    <skel frame="5" bone="72">
      <n11>0.9978729490858952</n11>
      <n12>-0.05190641760412991</n12>
      <n13>0.03943730840364432</n13>
      <n14>60.74230000000001</n14>
      <n21>0.05445762920593314</n21>
      <n22>0.9962901246433645</n22>
      <n23>-0.06663598246726996</n23>
      <n24>-0.0000162952</n24>
      <n31>-0.035832165771658014</n31>
      <n32>0.06864190665777636</n32>
      <n33>0.9969976652663223</n33>
      <n34>0.00000293244</n34>
    </skel>
    <skel frame="5" bone="73">
      <n11>0.9535400786874186</n11>
      <n12>0.2981919314843876</n12>
      <n13>0.04292889859292759</n13>
      <n14>21.0153</n14>
      <n21>-0.3009203942955171</n21>
      <n22>0.9495517522509884</n22>
      <n23>0.0883084712477122</n23>
      <n24>0.3897739999999999</n24>
      <n31>-0.01443033727333058</n31>
      <n32>-0.09712384771356518</n32>
      <n33>0.9951676861572074</n33>
      <n34>-0.0424223</n34>
    </skel>
    <skel frame="5" bone="77">
      <n11>0.8396987936181559</n11>
      <n12>0.4369605762081499</n12>
      <n13>0.32244595025531836</n13>
      <n14>63.5726</n14>
      <n21>-0.3468488325396669</n21>
      <n22>0.8884168676762865</n22>
      <n23>-0.30068481270946446</n23>
      <n24>0.000006694569999999999</n24>
      <n31>-0.41785383013930083</n31>
      <n32>0.14064467308823775</n32>
      <n33>0.8975618377414556</n33>
      <n34>0.0000188241</n34>
    </skel>
    <skel frame="5" bone="78">
      <n11>0.9987592274683075</n11>
      <n12>0.04429370402202519</n12>
      <n13>-0.02276124312064214</n13>
      <n14>23.2844</n14>
      <n21>0.0459193968208953</n21>
      <n22>-0.9959958559851825</n22>
      <n23>0.07671403559294958</n23>
      <n24>0.336163</n24>
      <n31>-0.019272140514039967</n31>
      <n32>-0.07766403315152011</n32>
      <n33>-0.9967939920100006</n33>
      <n34>-0.668896</n34>
    </skel>
    <skel frame="5" bone="79">
      <n11>0.9991307625914365</n11>
      <n12>-0.015471849365857293</n12>
      <n13>0.03870841149743413</n13>
      <n14>19.4204</n14>
      <n21>0.01428637235964821</n21>
      <n22>0.9994260114084642</n22>
      <n23>0.030717214798380677</n23>
      <n24>-0.73887</n24>
      <n31>-0.03916144543113736</n31>
      <n32>-0.030137511466088212</n32>
      <n33>0.9987783095333896</n33>
      <n34>0.513678</n34>
    </skel>
    <skel frame="5" bone="82">
      <n11>0.9473881419808324</n11>
      <n12>0.2836140798137114</n12>
      <n13>0.14838720351003265</n13>
      <n14>61.467400000000005</n14>
      <n21>-0.26511501632763895</n21>
      <n22>0.9550376906919605</n22>
      <n23>-0.1327291884830266</n23>
      <n24>-0.0000444905</n24>
      <n31>-0.17935923882449378</n31>
      <n32>0.08640638338218327</n32>
      <n33>0.9799817347068819</n33>
      <n34>0.00000944973</n34>
    </skel>
    <skel frame="5" bone="83">
      <n11>0.9999144812627291</n11>
      <n12>-0.012769560771111582</n12>
      <n13>0.0028228494115440607</n13>
      <n14>25.6045</n14>
      <n21>0.012805660104613549</n21>
      <n22>0.9998313586086061</n22>
      <n23>-0.013163183967190652</n23>
      <n24>0.361201</n24>
      <n31>-0.0026542852846812397</n31>
      <n32>0.013198206718410054</n32>
      <n33>0.9999093769482541</n33>
      <n34>-0.20137</n34>
    </skel>
    <skel frame="6" bone="59">
      <n11>0.995740102756846</n11>
      <n12>0.008755207975686082</n12>
      <n13>-0.09178776658731946</n13>
      <n14>94.0833</n14>
      <n21>-0.0696203884718799</n21>
      <n22>0.7240777600997805</n22>
      <n23>-0.6861955980898662</n23>
      <n24>-0.332509</n24>
      <n31>0.06045369526183068</n31>
      <n32>0.6896627753200741</n32>
      <n33>0.7216028042261218</n33>
      <n34>-1.2772699999999997</n34>
    </skel>
    <skel frame="6" bone="60">
      <n11>0.9715146241218644</n11>
      <n12>-0.23440869330128777</n12>
      <n13>0.034812348701793096</n13>
      <n14>64.8218</n14>
      <n21>0.23636307198202644</n21>
      <n22>0.947885274976315</n22>
      <n23>-0.21364925388658634</n23>
      <n24>0.0799684</n24>
      <n31>0.017083129706579263</n31>
      <n32>0.21579172826560905</n32>
      <n33>0.9762899654772499</n33>
      <n34>0.2182000000000001</n34>
    </skel>
    <skel frame="6" bone="62">
      <n11>0.27076049640780764</n11>
      <n12>0.9616735667871432</n12>
      <n13>-0.043274756243005544</n13>
      <n14>61.37369999999999</n14>
      <n21>-0.953682839947914</n21>
      <n22>0.26184760971424426</n22>
      <n23>-0.14807049022616964</n23>
      <n24>-0.00000899927</n24>
      <n31>-0.1310640849885238</n31>
      <n32>0.08136203186886837</n32>
      <n33>0.9880295670658293</n33>
      <n34>0.0000137974</n34>
    </skel>
    <skel frame="6" bone="63">
      <n11>-0.31991254961048576</n11>
      <n12>0.9376778839101768</n12>
      <n13>0.13570610419816753</n13>
      <n14>27.877</n14>
      <n21>-0.9471078259037659</n21>
      <n22>-0.31266785952048815</n22>
      <n23>-0.07228814441816318</n23>
      <n24>0.433203</n24>
      <n31>-0.025352057170039695</n31>
      <n32>-0.15165419788885554</n32>
      <n33>0.988108434060118</n33>
      <n34>-0.449036</n34>
    </skel>
    <skel frame="6" bone="64">
      <n11>0.9953641190395366</n11>
      <n12>0.09442096259941386</n12>
      <n13>-0.01830195258367688</n13>
      <n14>22.5617</n14>
      <n21>0.09550742283929158</n21>
      <n22>-0.992799421672419</n22>
      <n23>0.07232952357521348</n23>
      <n24>0.747526</n24>
      <n31>-0.011340715549374962</n31>
      <n32>-0.07374218344896417</n32>
      <n33>-0.9972148247526238</n33>
      <n34>-0.363771</n34>
    </skel>
    <skel frame="6" bone="71">
      <n11>0.9271285744813667</n11>
      <n12>0.041039855996183164</n12>
      <n13>0.3724893778350747</n13>
      <n14>15.132600000000002</n14>
      <n21>-0.07880055482407755</n21>
      <n22>0.993111609206727</n22>
      <n23>0.08671680470498586</n23>
      <n24>-0.06066269999999999</n24>
      <n31>-0.36636468025666175</n31>
      <n32>-0.10974999716919204</n32>
      <n33>0.9239761139671283</n33>
      <n34>-0.265564</n34>
    </skel>
    <skel frame="6" bone="72">
      <n11>0.48316350810627695</n11>
      <n12>0.8600638112635665</n12>
      <n13>-0.16383914364163454</n13>
      <n14>60.74230000000001</n14>
      <n21>-0.8421045658850298</n21>
      <n22>0.4052952297302411</n22>
      <n23>-0.3558028623739509</n23>
      <n24>-0.0000162952</n24>
      <n31>-0.23960994251078416</n31>
      <n32>0.3098806501101667</n32>
      <n33>0.9200875274327325</n33>
      <n34>0.0000029324400000000003</n34>
    </skel>
    <skel frame="6" bone="73">
      <n11>-0.14437267375439822</n11>
      <n12>0.9850444444663535</n12>
      <n13>0.0940424026435448</n13>
      <n14>21.0153</n14>
      <n21>-0.9894181605545564</n21>
      <n22>-0.14231795609630482</n22>
      <n23>-0.028236553143169897</n23>
      <n24>0.3897739999999999</n24>
      <n31>-0.014430337273804002</n31>
      <n32>-0.09712384771281421</n32>
      <n33>0.9951676861572654</n33>
      <n34>-0.0424223</n34>
    </skel>
    <skel frame="6" bone="74">
      <n11>0.8159695024881086</n11>
      <n12>0.5776624695085687</n12>
      <n13>-0.02235715390139279</n13>
      <n14>17.3959</n14>
      <n21>-0.5774375404853184</n21>
      <n22>0.8162756959142117</n22>
      <n23>0.016120641985841955</n23>
      <n24>0.659873</n24>
      <n31>0.027561891219125514</n31>
      <n32>-0.00024409225990437884</n32>
      <n33>0.999620069111957</n33>
      <n34>0.049808500000000006</n34>
    </skel>
    <skel frame="6" bone="77">
      <n11>0.2207064281249339</n11>
      <n12>0.9695218057057775</n12>
      <n13>0.10637735118595772</n13>
      <n14>63.57260000000001</n14>
      <n21>-0.9662667086594494</n21>
      <n22>0.23219129434562247</n22>
      <n23>-0.11142643567187271</n23>
      <n24>0.000006694569999999999</n24>
      <n31>-0.13273025397687907</n31>
      <n32>-0.07819636239053415</n32>
      <n33>0.9880627553896166</n33>
      <n34>0.000018824100000000005</n34>
    </skel>
    <skel frame="6" bone="78">
      <n11>-0.25051480340928967</n11>
      <n12>-0.9648009242322481</n12>
      <n13>0.08001483665111937</n13>
      <n14>23.2844</n14>
      <n21>-0.9679213718498588</n21>
      <n22>0.2512528447649801</n22>
      <n23>-0.0008621838829359629</n23>
      <n24>0.336163</n24>
      <n31>-0.019272140514039995</n31>
      <n32>-0.07766403315152008</n32>
      <n33>-0.9967939920099993</n33>
      <n34>-0.668896</n34>
    </skel>
    <skel frame="6" bone="82">
      <n11>0.22328576635975528</n11>
      <n12>0.974662645847352</n12>
      <n13>-0.013273783596852107</n13>
      <n14>61.467400000000005</n14>
      <n21>-0.9586830412638525</n21>
      <n22>0.21712175561427527</n22>
      <n23>-0.18380688135123058</n23>
      <n24>-0.0000444905</n24>
      <n31>-0.1762676741045482</n31>
      <n32>0.05376681159241453</n32>
      <n33>0.9828727471229216</n33>
      <n34>0.00000944973</n34>
    </skel>
    <skel frame="6" bone="83">
      <n11>-0.09624805245374324</n11>
      <n12>0.9952672783253236</n12>
      <n13>-0.01339242673129464</n13>
      <n14>25.6045</n14>
      <n21>-0.9953538401837196</n21>
      <n22>-0.09627487748270447</n22>
      <n23>-0.0013714215986544084</n23>
      <n24>0.361201</n24>
      <n31>-0.002654285284680905</n31>
      <n32>0.013198206718409796</n32>
      <n33>0.9999093769482541</n33>
      <n34>-0.20136999999999997</n34>
    </skel>
    <skel frame="6" bone="84">
      <n11>0.8433566578968381</n11>
      <n12>-0.5361252725382444</n12>
      <n13>-0.03631938219831239</n13>
      <n14>20.4207</n14>
      <n21>-0.5372622641747216</n21>
      <n22>-0.8425328097822151</n22>
      <n23>-0.038549335278349195</n23>
      <n24>0.742829</n24>
      <n31>-0.009933019245199999</n31>
      <n32>0.05202387819400006</n32>
      <n33>-0.9985955321679998</n33>
      <n34>-0.15405499999999997</n34>
    </skel>
    <skel frame="7" bone="59">
      <n11>0.9947725533520685</n11>
      <n12>0.008751830736911152</n12>
      <n13>-0.10173972948734726</n13>
      <n14>94.0833</n14>
      <n21>-0.06894825249268136</n21>
      <n22>0.7924871432871073</n22>
      <n23>-0.6059787671221206</n23>
      <n24>-0.332509</n24>
      <n31>0.075324003980216</n31>
      <n32>0.609825822004441</n32>
      <n33>0.7889478824618241</n33>
      <n34>-1.2772699999999997</n34>
    </skel>
    <skel frame="7" bone="62">
      <n11>0.7794377670214538</n11>
      <n12>0.6227925493377824</n12>
      <n13>0.06786904913107175</n13>
      <n14>61.37369999999999</n14>
      <n21>-0.6080600198871426</n21>
      <n22>0.7781450123592102</n22>
      <n23>-0.15733198007821686</n23>
      <n24>-0.00000899927</n24>
      <n31>-0.15079714704017952</n31>
      <n32>0.08136203186886828</n32>
      <n33>0.9852108607880405</n33>
      <n34>0.0000137974</n34>
    </skel>
    <skel frame="7" bone="63">
      <n11>-0.005743871320972094</n11>
      <n12>0.9884319113018712</n12>
      <n13>0.15155647350864576</n13>
      <n14>27.877</n14>
      <n21>-0.9996620834758118</n21>
      <n22>-0.0018332993146581433</n22>
      <n23>-0.025929864527625607</n23>
      <n24>0.433203</n24>
      <n31>-0.02535205717582409</n31>
      <n32>-0.1516541978771165</n32>
      <n33>0.9881084340613646</n33>
      <n34>-0.449036</n34>
    </skel>
    <skel frame="7" bone="69">
      <n11>0.9674259640354129</n11>
      <n12>0.240764121579898</n12>
      <n13>-0.07822813982837838</n13>
      <n14>24.913399999999996</n14>
      <n21>-0.24386729937283577</n21>
      <n22>0.9692556484125932</n22>
      <n23>-0.03274489744044404</n23>
      <n24>-0.588824</n24>
      <n31>0.06793926992357156</n31>
      <n32>0.050755549170958675</n32>
      <n33>0.996397576186138</n33>
      <n34>-1.0299</n34>
    </skel>
    <skel frame="7" bone="72">
      <n11>0.8723997227222935</n11>
      <n12>0.43095039411817815</n12>
      <n13>0.23065229589894032</n13>
      <n14>60.74230000000001</n14>
      <n21>-0.3035216648178739</n21>
      <n22>0.847499699408022</n22>
      <n23>-0.43545247558085914</n23>
      <n24>-0.0000162952</n24>
      <n31>-0.3831361674134296</n31>
      <n32>0.3098806501101666</n32>
      <n33>0.8701612838474537</n33>
      <n34>0.0000029324400000000003</n34>
    </skel>
    <skel frame="7" bone="73">
      <n11>0.31937581780668434</n11>
      <n12>0.9426882396345153</n12>
      <n13>0.09663317160835687</n13>
      <n14>21.0153</n14>
      <n21>-0.9475182596480605</n21>
      <n22>0.3192269428793447</n22>
      <n23>0.017415699050518006</n23>
      <n24>0.3897739999999999</n24>
      <n31>-0.014430337273333538</n31>
      <n32>-0.09712384771355695</n32>
      <n33>0.9951676861572082</n33>
      <n34>-0.0424223</n34>
    </skel>
    <skel frame="7" bone="77">
      <n11>0.8462732815658879</n11>
      <n12>0.49166264022057565</n12>
      <n13>0.20515696702533948</n13>
      <n14>63.57260000000001</n14>
      <n21>-0.4970487439355065</n21>
      <n22>0.8672676502212111</n22>
      <n23>-0.028095711984471063</n23>
      <n24>0.000006694569999999999</n24>
      <n31>-0.19173961265173856</n31>
      <n32>-0.07819636239053476</n32>
      <n33>0.9783257381102929</n33>
      <n34>0.000018824100000000005</n34>
    </skel>
    <skel frame="7" bone="78">
      <n11>0.14618689499354276</n11>
      <n12>-0.9864830991573807</n12>
      <n13>0.07403429519931662</n13>
      <n14>23.2844</n14>
      <n21>-0.9890695457796774</n21>
      <n22>-0.14429200890383664</n22>
      <n23>0.030365121104546068</n23>
      <n24>0.336163</n24>
      <n31>-0.01927214051404</n31>
      <n32>-0.07766403315152004</n32>
      <n33>-0.9967939920099993</n33>
      <n34>-0.668896</n34>
    </skel>
    <skel frame="7" bone="82">
      <n11>0.8812317940105114</n11>
      <n12>0.4086373220535022</n12>
      <n13>0.23758380468785684</n13>
      <n14>61.46740000000001</n14>
      <n21>-0.3785296183223623</n21>
      <n22>0.9111117763458708</n22>
      <n23>-0.16306642528920118</n23>
      <n24>-0.0000444905</n24>
      <n31>-0.28310042966718035</n31>
      <n32>0.05376681159241465</n32>
      <n33>0.9575819947625601</n33>
      <n34>0.00000944973</n34>
    </skel>
    <skel frame="7" bone="83">
      <n11>0.15299854733271467</n11>
      <n12>0.9881456147575287</n12>
      <n13>-0.012636793483487162</n13>
      <n14>25.6045</n14>
      <n21>-0.988222848999003</n21>
      <n22>0.15295114048245395</n22>
      <n23>-0.004642126819686792</n23>
      <n24>0.361201</n24>
      <n31>-0.002654285284681217</n31>
      <n32>0.013198206718410035</n32>
      <n33>0.9999093769482541</n33>
      <n34>-0.20136999999999997</n34>
    </skel>
    <skel frame="8" bone="59">
      <n11>0.9937178265680587</n11>
      <n12>0.01266106641008611</n12>
      <n13>-0.11119612654320722</n13>
      <n14>94.0833</n14>
      <n21>-0.07737315284335636</n21>
      <n22>0.7955961926681644</n22>
      <n23>-0.600866119390168</n23>
      <n24>-0.332509</n24>
      <n31>0.08085960907605343</n31>
      <n32>0.6056949691133983</n32>
      <n33>0.7915777460305382</n33>
      <n34>-1.2772699999999997</n34>
    </skel>
    <skel frame="8" bone="62">
      <n11>-0.23662472088820752</n11>
      <n12>0.964665642255755</n12>
      <n13>-0.11588330387040946</n13>
      <n14>61.37369999999999</n14>
      <n21>-0.9598275688420109</n21>
      <n22>-0.250600116563251</n22>
      <n23>-0.12621655861800588</n23>
      <n24>-0.00000899927</n24>
      <n31>-0.15079714704020342</n31>
      <n32>0.08136203186886029</n32>
      <n33>0.9852108607880379</n33>
      <n34>0.0000137974</n34>
    </skel>
    <skel frame="8" bone="63">
      <n11>0.39409095966690144</n11>
      <n12>0.9186944322197105</n12>
      <n13>0.026322152596711362</n13>
      <n14>27.877</n14>
      <n21>-0.9190641508514962</n21>
      <n22>0.39404025564316625</n22>
      <n23>0.007305036090696888</n23>
      <n24>0.433203</n24>
      <n31>-0.003660891754599308</n31>
      <n32>-0.02707059550826412</n32>
      <n33>0.9996268207338121</n33>
      <n34>-0.449036</n34>
    </skel>
    <skel frame="8" bone="64">
      <n11>0.6197210727635412</n11>
      <n12>-0.7847380783491102</n12>
      <n13>0.011504646530872176</n13>
      <n14>22.5617</n14>
      <n21>-0.7847837329031451</n21>
      <n22>-0.6197705684783346</n22>
      <n23>-0.0008421390882641993</n23>
      <n24>0.747526</n24>
      <n31>0.007791089362830908</n31>
      <n32>-0.008506762990978106</n32>
      <n33>-0.9999345601056646</n33>
      <n34>-0.363771</n34>
    </skel>
    <skel frame="8" bone="72">
      <n11>-0.015069627064563829</n11>
      <n12>0.9477630207769528</n12>
      <n13>-0.31861915006458996</n13>
      <n14>60.74230000000001</n14>
      <n21>-0.9810189812654079</n21>
      <n22>-0.0756256513032898</n22>
      <n23>-0.17855676761729294</n23>
      <n24>-0.0000162952</n24>
      <n31>-0.1933252821984688</n31>
      <n32>0.30988065011016747</n32>
      <n33>0.9309131634852859</n33>
      <n34>0.00000293244</n34>
    </skel>
    <skel frame="8" bone="73">
      <n11>0.13468115419300208</n11>
      <n12>0.9860126913952596</n12>
      <n13>0.09818329345014501</n13>
      <n14>21.0153</n14>
      <n21>-0.9907839078585292</n21>
      <n22>0.13544715062632418</n22>
      <n23>-0.0011477437828927572</n23>
      <n24>0.3897739999999999</n24>
      <n31>-0.014430337273332629</n31>
      <n32>-0.09712384771355757</n32>
      <n33>0.9951676861572082</n33>
      <n34>-0.0424223</n34>
    </skel>
    <skel frame="8" bone="74">
      <n11>0.64520119078803</n11>
      <n12>0.7638098914646942</n12>
      <n13>-0.01760321295673609</n13>
      <n14>17.3959</n14>
      <n21>-0.7635153996863018</n21>
      <n22>0.6454412367672682</n22>
      <n23>0.021209533757394428</n23>
      <n24>0.659873</n24>
      <n31>0.027561891219125546</n31>
      <n32>-0.00024409225990436323</n32>
      <n33>0.999620069111957</n33>
      <n34>0.0498085</n34>
    </skel>
    <skel frame="8" bone="77">
      <n11>-0.15753383229644125</n11>
      <n12>0.9849959420291517</n12>
      <n13>0.07047046096128878</n13>
      <n14>63.572600000000016</n14>
      <n21>-0.9865599952065364</n21>
      <n22>-0.15384512697836095</n22>
      <n23>-0.05505499762135015</n23>
      <n24>0.000006694569999999998</n24>
      <n31>-0.04338741223064134</n31>
      <n32>-0.07819636239053446</n32>
      <n33>0.9959934042797757</n33>
      <n34>0.00001882410000000001</n34>
    </skel>
    <skel frame="8" bone="78">
      <n11>0.23934816968358386</n11>
      <n12>-0.9683476999187413</n12>
      <n13>0.07082010678712333</n13>
      <n14>23.2844</n14>
      <n21>-0.9707426745397467</n21>
      <n22>-0.23721619393819648</n22>
      <n23>0.037250879659482136</n23>
      <n24>0.336163</n24>
      <n31>-0.019272137677886866</n31>
      <n32>-0.07766402172221926</n32>
      <n33>-0.9967936981550676</n33>
      <n34>-0.6688959999999999</n34>
    </skel>
    <skel frame="8" bone="79">
      <n11>0.7679136521776341</n11>
      <n12>-0.6404625919717591</n12>
      <n13>0.010783834384090888</n13>
      <n14>19.4204</n14>
      <n21>0.6393551469965278</n21>
      <n22>0.7673978099313598</n22>
      <n23>0.048224447343655975</n23>
      <n24>-0.73887</n24>
      <n31>-0.039161445431137365</n31>
      <n32>-0.030137511466088222</n32>
      <n33>0.9987783095333896</n33>
      <n34>0.513678</n34>
    </skel>
    <skel frame="8" bone="82">
      <n11>-0.14402875604136356</n11>
      <n12>0.9856696343280847</n12>
      <n13>-0.08781280884190079</n13>
      <n14>61.4674</n14>
      <n21>-0.9636150379592793</n21>
      <n22>-0.15988903006370414</n22>
      <n23>-0.21419980551814616</n23>
      <n24>-0.0000444905</n24>
      <n31>-0.22517054881112075</n31>
      <n32>0.05376681159241768</n32>
      <n33>0.9728347002031146</n33>
      <n34>0.00000944973</n34>
    </skel>
    <skel frame="8" bone="83">
      <n11>0.19239453857331013</n11>
      <n12>0.9812387893913211</n12>
      <n13>-0.012441049835823953</n13>
      <n14>25.6045</n14>
      <n21>-0.9813140660852623</n21>
      <n22>0.19234408109758017</n22>
      <n23>-0.005143750571023471</n23>
      <n24>0.361201</n24>
      <n31>-0.0026542852846812275</n31>
      <n32>0.01319820671841005</n32>
      <n33>0.9999093769482541</n33>
      <n34>-0.2013699999999999</n34>
    </skel>
    <skel frame="8" bone="84">
      <n11>0.333700390462239</n11>
      <n12>-0.9412240655948658</n12>
      <n13>-0.052354282172343364</n13>
      <n14>20.4207</n14>
      <n21>-0.9426266745672438</n21>
      <n22>-0.33375155279676205</n22>
      <n23>-0.008011182193513745</n23>
      <n24>0.742829</n24>
      <n31>-0.009933021242762755</n31>
      <n32>0.05202388865617262</n32>
      <n33>-0.998595934092118</n33>
      <n34>-0.15405499999999997</n34>
    </skel>
    <skel frame="9" bone="58">
      <n11>0.679088153656201</n11>
      <n12>0.021685855699830114</n12>
      <n13>-0.7337363310679093</n13>
      <n14>133.50299999999996</n14>
      <n21>-0.41017179759832245</n21>
      <n22>0.8401681051368584</n22>
      <n23>-0.35479099723135954</n23>
      <n24>0.00000160413</n24>
      <n31>0.6087679166327602</n31>
      <n32>0.5418923130327906</n32>
      <n33>0.5794431336646972</n33>
      <n34>0.00000780019</n34>
    </skel>
    <skel frame="9" bone="59">
      <n11>0.992997845754381</n11>
      <n12>0.021658640361823245</n12>
      <n13>-0.11613002034287145</n13>
      <n14>94.0833</n14>
      <n21>0.08728230631686507</n21>
      <n22>0.5279321186732219</n22>
      <n23>0.844789605213755</n23>
      <n24>-0.332509</n24>
      <n31>0.07960576192190782</n31>
      <n32>-0.849010354101103</n32>
      <n33>0.5223450404645881</n33>
      <n34>-1.2772699999999997</n34>
    </skel>
    <skel frame="9" bone="60">
      <n11>0.9994435588104839</n11>
      <n12>-0.031662879241595374</n12>
      <n13>-0.010489748822880804</n13>
      <n14>64.8218</n14>
      <n21>0.02864855025412589</n21>
      <n22>0.9759259152672777</n22>
      <n23>-0.2162125539326131</n23>
      <n24>0.0799684</n24>
      <n31>0.01708312970657905</n31>
      <n32>0.21579172826560902</n32>
      <n33>0.9762899654772499</n33>
      <n34>0.21820000000000012</n34>
    </skel>
    <skel frame="9" bone="61">
      <n11>0.9991944089997378</n11>
      <n12>0.009602040696115602</n12>
      <n13>-0.038965803445261335</n13>
      <n14>15.132599999999998</n14>
      <n21>-0.009368850495084402</n21>
      <n22>0.9999371210121846</n22>
      <n23>0.0061626830410474245</n23>
      <n24>-0.0606627</n24>
      <n31>0.03902252764833869</n31>
      <n32>-0.005792653652152577</n32>
      <n33>0.9992215407503987</n33>
      <n34>-0.265564</n34>
    </skel>
    <skel frame="9" bone="62">
      <n11>0.9375725990963588</n11>
      <n12>0.2706059837619948</n12>
      <n13>0.21847201874817157</n13>
      <n14>61.37369999999999</n14>
      <n21>-0.24342285422296212</n21>
      <n22>0.959245860727255</n22>
      <n23>-0.14350154256862252</n23>
      <n24>-0.00000899927</n24>
      <n31>-0.24840075576705672</n31>
      <n32>0.08136203186886777</n32>
      <n33>0.9652343157516337</n33>
      <n34>0.0000137974</n34>
    </skel>
    <skel frame="9" bone="63">
      <n11>0.9655996155007405</n11>
      <n12>-0.24117441578769755</n12>
      <n13>-0.09722285592536788</n13>
      <n14>27.876999999999995</n14>
      <n21>0.23644902873431123</n21>
      <n22>0.9699306229519399</n22>
      <n23>-0.0576753280932379</n23>
      <n24>0.433203</n24>
      <n31>0.1082092387711092</n31>
      <n32>0.03270302477638046</n32>
      <n33>0.993590092953353</n33>
      <n34>-0.449036</n34>
    </skel>
    <skel frame="9" bone="64">
      <n11>0.9918710252197944</n11>
      <n12>-0.12598687311389845</n12>
      <n13>0.017865976246067172</n13>
      <n14>22.5617</n14>
      <n21>-0.12551699712192232</n21>
      <n22>-0.9917693739851783</n22>
      <n23>-0.025355599417365027</n23>
      <n24>0.747526</n24>
      <n31>0.020913362659742717</n31>
      <n32>0.022907003106059007</n32>
      <n33>-0.9995208001172833</n33>
      <n34>-0.363771</n34>
    </skel>
    <skel frame="9" bone="66">
      <n11>0.2312682827646837</n11>
      <n12>-0.41283149588752177</n12>
      <n13>-0.8809567126698623</n13>
      <n14>15.132599999999998</n14>
      <n21>-0.38127956030032184</n21>
      <n22>0.7946106569640501</n22>
      <n23>-0.4724613124718573</n23>
      <n24>-0.0606627</n24>
      <n31>0.8950645063301375</n31>
      <n32>0.4451560972161426</n32>
      <n33>0.02636403100113882</n33>
      <n34>-0.265564</n34>
    </skel>
    <skel frame="9" bone="71">
      <n11>0.9271285744813667</n11>
      <n12>0.04103985599618317</n12>
      <n13>0.3724893778350747</n13>
      <n14>15.132599999999998</n14>
      <n21>-0.07880055482407758</n21>
      <n22>0.993111609206727</n22>
      <n23>0.08671680470498587</n23>
      <n24>-0.0606627</n24>
      <n31>-0.36636468025666175</n31>
      <n32>-0.10974999716919205</n32>
      <n33>0.9239761139671283</n33>
      <n34>-0.265564</n34>
    </skel>
    <skel frame="9" bone="72">
      <n11>0.9985373709499882</n11>
      <n12>-0.046288527391639175</n12>
      <n13>-0.02793726987733757</n13>
      <n14>60.74230000000001</n14>
      <n21>0.0366670900090888</n21>
      <n22>0.9595199365094</n22>
      <n23>-0.2792436497957693</n23>
      <n24>-0.0000162952</n24>
      <n31>0.03973214475146169</n31>
      <n32>0.27781084153234614</n32>
      <n33>0.9598137803764533</n33>
      <n34>0.00000293244</n34>
    </skel>
    <skel frame="9" bone="73">
      <n11>0.9954524938108436</n11>
      <n12>-0.09511991248125895</n12>
      <n13>0.005151195524352716</n13>
      <n14>21.0153</n14>
      <n21>0.0941599592818006</n21>
      <n22>0.9907164884339369</n22>
      <n23>0.09805478883348094</n23>
      <n24>0.389774</n24>
      <n31>-0.014430337273332369</n31>
      <n32>-0.0971238477135586</n32>
      <n33>0.9951676861572081</n33>
      <n34>-0.0424223</n34>
    </skel>
    <skel frame="9" bone="74">
      <n11>0.9989776064893611</n11>
      <n12>0.03585445961925638</n12>
      <n13>-0.02753542188161418</n13>
      <n14>17.3959</n14>
      <n21>-0.03583411621921843</n21>
      <n22>0.9993569923427662</n22>
      <n23>0.0012320594173083096</n23>
      <n24>0.659873</n24>
      <n31>0.027561891219125546</n31>
      <n32>-0.00024409225990435922</n32>
      <n33>0.999620069111957</n33>
      <n34>0.04980849999999999</n34>
    </skel>
    <skel frame="9" bone="77">
      <n11>0.9240527255610828</n11>
      <n12>0.09529426917204778</n12>
      <n13>0.3701966540179687</n13>
      <n14>63.572600000000016</n14>
      <n21>-0.11739946202299767</n21>
      <n22>0.9923730806364376</n22>
      <n23>0.03759035973301214</n23>
      <n24>0.000006694569999999997</n24>
      <n31>-0.3637910481304412</n31>
      <n32>-0.07819636239053476</n32>
      <n33>0.9281925458702215</n33>
      <n34>0.000018824100000000012</n34>
    </skel>
    <skel frame="9" bone="78">
      <n11>0.9987592274683075</n11>
      <n12>0.04429370402202506</n12>
      <n13>-0.022761243120642177</n13>
      <n14>23.2844</n14>
      <n21>0.045919396820895166</n21>
      <n22>-0.9959958559851829</n22>
      <n23>0.0767140355929496</n23>
      <n24>0.336163</n24>
      <n31>-0.019272140514040012</n31>
      <n32>-0.07766403315152014</n32>
      <n33>-0.9967939920100011</n33>
      <n34>-0.6688959999999999</n34>
    </skel>
    <skel frame="9" bone="79">
      <n11>0.9562719332730087</n11>
      <n12>-0.29106632854841386</n12>
      <n13>0.02871205356029055</n13>
      <n14>19.4204</n14>
      <n21>0.2898454257462868</n21>
      <n22>0.9562280704873592</n22>
      <n23>0.040218234496003875</n23>
      <n24>-0.73887</n24>
      <n31>-0.039161445431137365</n31>
      <n32>-0.030137511466088236</n32>
      <n33>0.9987783095333896</n33>
      <n34>0.513678</n34>
    </skel>
    <skel frame="9" bone="82">
      <n11>0.9919218530316043</n11>
      <n12>0.111216523560115</n12>
      <n13>0.061007559904906326</n13>
      <n14>61.467400000000005</n14>
      <n21>-0.1075229193200675</n21>
      <n22>0.9923406747979184</n22>
      <n23>-0.060817817804505764</n23>
      <n24>-0.0000444905</n24>
      <n31>-0.06730422943053878</n31>
      <n32>0.05376681159241442</n32>
      <n33>0.9962827262739966</n33>
      <n34>0.00000944973</n34>
    </skel>
    <skel frame="9" bone="83">
      <n11>0.9656613905330645</n11>
      <n12>-0.25973482275751814</n12>
      <n13>0.00599171768934226</n13>
      <n14>25.604500000000005</n14>
      <n21>0.2597903647238975</n21>
      <n22>0.965589783078994</n22>
      <n23>-0.012055588334278113</n23>
      <n24>0.361201</n24>
      <n31>-0.0026542852846812353</n31>
      <n32>0.013198206718410049</n32>
      <n33>0.9999093769482541</n33>
      <n34>-0.2013699999999999</n34>
    </skel>
    <skel frame="9" bone="84">
      <n11>0.9402761636642937</n11>
      <n12>-0.33933756942334997</n12>
      <n13>-0.027031385540784235</n13>
      <n14>20.4207</n14>
      <n21>-0.34026756671048725</n21>
      <n22>-0.9392240252535999</n22>
      <n23>-0.04554616305919905</n23>
      <n24>0.742829</n24>
      <n31>-0.009933019245199999</n31>
      <n32>0.05202387819400005</n32>
      <n33>-0.9985955321679996</n33>
      <n34>-0.15405499999999997</n34>
    </skel>
    <skel frame="10" bone="56">
      <n11>0.4751363550977592</n11>
      <n12>0.8774533129492872</n12>
      <n13>-0.0657361931163557</n13>
      <n14>116.259</n14>
      <n21>-0.17619575832304596</n21>
      <n22>0.1680710619715965</n22>
      <n23>0.9699006970677887</n23>
      <n24>-0.00000903036</n24>
      <n31>0.8620908410603129</n31>
      <n32>-0.4492527028498946</n32>
      <n33>0.23446015638437734</n33>
      <n34>0.00000990724</n34>
    </skel>
  </skeletalAnimation>
  <morphAnimation>
    <morph frame="0">Repose</morph>
    <morph frame="1">Repose</morph>
    <morph frame="2">Repose</morph>
    <morph frame="3">Repose</morph>
    <morph frame="4">Repose</morph>
    <morph frame="5">Repose</morph>
    <morph frame="6">Repose</morph>
    <morph frame="7">Repose</morph>
    <morph frame="8">Repose</morph>
    <morph frame="9">Repose</morph>
    <xpf name="Repose">
      <xml>
        <HEADER MAGIC="XPF" VERSION="919"/>
        <ANIMATION NUMTRACKS="28" DURATION="1">
          <TRACK NUMKEYFRAMES="1" MORPHNAME="blink.Left.Lower.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="blink.Left.Upper.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="blink.Right.Lower.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="blink.Right.Upper.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="eyebrow.Left.Angry.Average">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="eyebrow.Left.Down.Average">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="eyebrow.Right.Angry.Average">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="eyebrow.Right.Down.Average">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="eyebrows.Left.Sad.Average">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="eyebrows.Left.Up.Average">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="eyebrows.Right.Sad.Average">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="eyebrows.Right.Up.Average">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="left.Eye.Down.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="left.Eye.Left.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="left.Eye.Right.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="left.Eye.Up.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="mouth.Frown.Exclusive">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="mouth.Grin.Exclusive">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="mouth.Mad.Exclusive">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="mouth.Open.Exclusive">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="mouth.Pucker.Exclusive">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="mouth.Smile.Exclusive">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="mouth.Surprise.Exclusive">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="mouth.Tongue.Exclusive">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="right.Eye.Down.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="right.Eye.Left.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="right.Eye.Right.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
          <TRACK NUMKEYFRAMES="1" MORPHNAME="right.Eye.Up.Clamped">
            <KEYFRAME TIME="0">
              <WEIGHT>0</WEIGHT>
            </KEYFRAME>
          </TRACK>
        </ANIMATION>
      </xml>
    </xpf>
  </morphAnimation>
</imvu>
"""

xml_that_crashes_due_to_missing_header_attributes = """<HEADER />
<SKELETON SCENEAMBIENTCOLOR="1 1 1" NUMBONES="2">
    <BONE NAME="FemaleAnimeRoot" NUMCHILDS="1" ID="0">
        <TRANSLATION>0 1.31134e-006 -30</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 -1.31134e-006 30</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>-1</PARENTID>
        <CHILDID>1</CHILDID>
    </BONE>
    <BONE NAME="PelvisNode" NUMCHILDS="3" ID="1">
        <TRANSLATION>0 -10.3201 519.784</TRANSLATION>
        <ROTATION>0 0 0 1</ROTATION>
        <LOCALTRANSLATION>0 10.3201 -489.784</LOCALTRANSLATION>
        <LOCALROTATION>0 0 0 1</LOCALROTATION>
        <PARENTID>0</PARENTID>
    </BONE>
</SKELETON>"""

if __name__ == "__main__":
    imvu.test.main()
