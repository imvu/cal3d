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
        self.cfl = imvu.cfl.CFL(imvu.fs.getSourceFileSystem(), 'Source/test_data/product191.cfl')

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
                'lodCount': 0,
                'materialId': 2,
                'vertexCount': 959
            },
            1: {'faceCount': 336,
                'lodCount': 0,
                'materialId': 3,
                'vertexCount': 172
            },
            2: {'faceCount': 66,
                'lodCount': 0,
                'materialId': 5,
                'vertexCount': 66
            },
            3: {'faceCount': 64,
                'lodCount': 0,
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

if __name__ == "__main__":
    imvu.test.main()
