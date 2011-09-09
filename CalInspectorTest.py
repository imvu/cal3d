import imvu
import logging
logger = logging.getLogger("imvu." + __name__)
import mmap

import cal3d.CalInspector
import os
import tempfile
import imvu.test
import imvu.fs
import imvu.cfl

class CalInspectorTest(imvu.test.TestCase):
    def setUp(self):
        self.cfl = imvu.cfl.CFL(imvu.fs.getSourceFileSystem(), 'TestData/product191.cfl')

    def test_getMaterialMaps(self):
        entryname = 'Male03_Anime01_eyebrows02_alpha.xrf'
        data = self.cfl.getContents(entryname)

        result = cal3d.CalInspector.getMaterialInfo(data)['maps']
        logger.info("result: %r", result)

        expected = {'Opacity': 'eyebrows001_alpha.tga', 'Diffuse Color': 'eyebrows001_brunette001.tga'}
        self.assertEqual(result, expected)

    def assertMeshInfo(self, result):
        expected = {
            0: {'faceCount': 1430,
                'materialId': 2,
                'vertexCount': 959
            },
            1: {'faceCount': 336,
                'materialId': 3,
                'vertexCount': 172
            },
            2: {'faceCount': 66,
                'materialId': 5,
                'vertexCount': 66
            },
            3: {'faceCount': 64,
                'materialId': 6,
                'vertexCount': 48
            }
        }
        self.assertEqual(result, expected)

    def test_getMeshInfo(self):
        entryname = 'Male03_Anime01_HeadMorphFile.xmf'
        data = self.cfl.getContents(entryname)

        result = cal3d.CalInspector.getMeshInfo(data)
        logger.info("result: %r", result)

        self.assertMeshInfo(result)

    def test_getMeshInfo_with_mmap(self):
        entryname = 'Male03_Anime01_HeadMorphFile.xmf'
        data = self.cfl.getContents(entryname)

        with open(os.path.join(tempfile.gettempdir(), 'mmap.test.bin'), 'wb') as f:
            f.write(data)
        with open(os.path.join(tempfile.gettempdir(), 'mmap.test.bin'), 'r+b') as f:
            map = mmap.mmap(f.fileno(), 0)
            result = cal3d.CalInspector.getMeshInfo(map)
        self.assertMeshInfo(result)

class LocaleTest(imvu.test.TestCase):
    def test_convert_to_binary_matches_gold_standard(self):
        meshData = imvu.fs.getSourceFileSystem().file('TestData/DN_flor_dress.xmf', 'rb').read()
        coreMesh = cal3d.loadCoreMeshFromBuffer(meshData)
        binaryMeshData = cal3d.saveCoreMeshToBuffer(coreMesh)

        binaryFileData = imvu.fs.getSourceFileSystem().file('TestData/DN_flor_dress.cmf', 'rb').read()
        self.assertEqual(len(binaryMeshData), len(binaryFileData))

    def test_can_load_mesh_on_serbian_locale(self):
        meshData = imvu.fs.getSourceFileSystem().file('TestData/DN_flor_dress.xmf', 'rb').read()
        coreMesh = cal3d.loadCoreMeshFromBuffer(meshData)
        [sm1, sm2, sm3] = coreMesh.submeshes
        self.assertEqual([8221, 10438, 10700], [len(sm.vertices) for sm in coreMesh.submeshes])

        def assertSubmeshStats(submesh, (sumx, sumy, sumz), (sumBoneId, sumWeight)):
            self.assertEqual(sumx, sum(vertex.position.x for vertex in submesh.vertices))
            self.assertEqual(sumy, sum(vertex.position.y for vertex in submesh.vertices))
            self.assertEqual(sumz, sum(vertex.position.z for vertex in submesh.vertices))

            self.assertEqual(sumBoneId, sum(inf.boneId for inf in submesh.influences))
            self.assertEqual(sumWeight, sum(inf.weight for inf in submesh.influences))

        assertSubmeshStats(sm1, (-982824.2527006492, -180497.75814134791, 6765443.3099365234), (669991, 8221.0529946999159))
        assertSubmeshStats(sm2, (251854.15499719395,  152439.8440515846,  3896401.3394775391), (103119, 10438.036999161588))
        assertSubmeshStats(sm3, (-19384.503997981548, 6021.3888615714386, 6454644.2445678711), (148316, 10700.017000436899))
        
if __name__ == "__main__":
    imvu.test.main()
