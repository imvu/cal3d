import imvu
import logging
logger = logging.getLogger("imvu." + __name__)

import cal3d.CalInspector
import os
import pyCFL.CflManager
import imvu.test

class CalInspectorTest(imvu.test.TestCase):
    def setUp(self):
        self.cflManager = pyCFL.CflManager.CflManager()
        self.cflFilename = os.path.dirname(os.path.abspath(__file__)) + "/../test_data/product191.cfl"

    def test_getMaterialMaps(self):
        entryname = 'Male03_Anime01_eyebrows02_alpha.xrf'
        data = self.cflManager.getCflEntryData(self.cflFilename, entryname)

        result = cal3d.CalInspector.getMaterialInfo(data)['maps']
        logger.info("result: %r", result)

        expected = {'Opacity': 'eyebrows001_alpha.tga', 'Diffuse Color': 'eyebrows001_brunette001.tga'}
        self.assertEqual(result, expected)

    def test_getMeshInfo(self):
        entryname = 'Male03_Anime01_HeadMorphFile.xmf'
        data = self.cflManager.getCflEntryData(self.cflFilename, entryname)

        result = cal3d.CalInspector.getMeshInfo(data)
        logger.info("result: %r", result)

        expected = {
            0: {'faceCount': 1430,
                'lodCount': 0,
                'materialId': 2,
                'springCount': 0,
                'tangentsEnabled': 0,
                'vertexCount': 959
            },
            1: {'faceCount': 336,
                'lodCount': 0,
                'materialId': 3,
                'springCount': 0,
                'tangentsEnabled': 0,
                'vertexCount': 172
            },
            2: {'faceCount': 66,
                'lodCount': 0,
                'materialId': 5,
                'springCount': 0,
                'tangentsEnabled': 0,
                'vertexCount': 66
            },
            3: {'faceCount': 64,
                'lodCount': 0,
                'materialId': 6,
                'springCount': 0,
                'tangentsEnabled': 0,
                'vertexCount': 48
            }
        }
        self.assertEqual(result, expected)

if __name__ == "__main__":
    imvu.test.main()
