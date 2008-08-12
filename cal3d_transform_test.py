import imvu
import logging
logger = logging.getLogger('imvu.' + __name__)

import imvu.test
import ctypes
import pyCFL.CflManager
import os.path
import imvu.fs
import cal3d.CalInspector
import pprint
import copy
import cPickle as pickle

import warnings
warnings.simplefilter('ignore', RuntimeWarning)

def sortedlist(l):
    ret = list(l)
    ret.sort()
    return ret

def epsilonEquals(f1, f2):
    return abs(f1-f2)<0.01

def allVertsEqual(av1, av2):
    k1 = av1.keys()
    k1.sort()
    k2 = av2.keys()
    k2.sort()
    if k1 != k2: return False
    for k in k1:
        f1 = av1[k]
        f2 = av2[k]
        if len(f1) != len(f2): return False
        for i in range(len(f1)):
            if not epsilonEquals(f1[i], f2[i]): return False
    return True

class Cal3dTransformTest(imvu.test.TestCase):
    def setUp(self):
        if __debug__:
            self.cal3d  = ctypes.cdll.cal3d_debug
        else:
            self.cal3d = ctypes.cdll.cal3d
        self.cal3d.CalSubmesh_GetBaseWeight.restype = ctypes.c_float
        self.cal3d.CalModel_Update.argtypes = [ctypes.c_int, ctypes.c_float]
        self.cal3d.CalSubmesh_SetMorphTargetWeight.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_float]

        self.loader = self.cal3d.CalLoader_New()

        test_data_dir = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "test_data"))
        self.testDataFs = imvu.fs.FileSystem(test_data_dir)
        self.cflManager = pyCFL.CflManager.CflManager()
        
        self.__tempFiles = []

    def tearDown(self):
        for file in self.__tempFiles:
            os.unlink(file)

    def getFilenameForCflResource(self, cfl_name, resource_name):
        data = self.cflManager.getCflEntryData(cfl_name, resource_name)
        filename = os.tempnam()
        f = file(filename, 'wb')
        f.write(data)
        f.close()

        self.__tempFiles.append(filename)
        return filename

    def loadModelWithEverything(self, cfl_name):
        cfl_name = self.testDataFs.getRealFileSystemPath(cfl_name)
        
        PRINT = logger.info
        PRINT = lambda *args: None

        entry_names = self.cflManager.getCflEntryNames(cfl_name)
        skel_names = [s for s in entry_names if '.xsf' in s]
        morphanim_names = [s for s in entry_names if '.xpf' in s]
        skelanim_names = [s for s in entry_names if '.xaf' in s]
        mesh_names = [s for s in entry_names if '.xmf' in s]

        coreModel = self.cal3d.CalCoreModel_New()
        r = self.cal3d.CalCoreModel_LoadCoreSkeleton(coreModel, self.getFilenameForCflResource(cfl_name, skel_names[0]))
        assert r

        model = self.cal3d.CalModel_New()
        r = self.cal3d.CalModel_Create(model, coreModel)
        assert r

        meshIds = {}
        for name in mesh_names:
            id = self.cal3d.CalCoreModel_LoadCoreMesh(coreModel, self.getFilenameForCflResource(cfl_name, name))
            assert id != -1
            PRINT("          mesh id % 60s: %r", name, id)
            meshIds[name] = id
            r = self.cal3d.CalModel_AttachMesh(model, id)
            assert r

        PRINT("...")

        morphAnimIds = {}
        for name in morphanim_names:
            id = self.cal3d.CalCoreModel_LoadCoreAnimatedMorph(coreModel, self.getFilenameForCflResource(cfl_name, name))
            assert id != -1
            PRINT("animated morph id % 60s: %r", name, id)
            morphAnimIds[name] = id

        PRINT("...")

        skelAnimIds = {}
        for name in skelanim_names:
            id = self.cal3d.CalCoreModel_LoadCoreAnimation(coreModel, self.getFilenameForCflResource(cfl_name, name))
            assert id != -1
            PRINT("     animation id % 60s: %r", name, id)
            skelAnimIds[name] = id

        PRINT("...")

        return model, meshIds, skelAnimIds, morphAnimIds

    def getAllVerts(self, model):
        renderer = self.cal3d.CalModel_GetRenderer(model)
        coreModel = self.cal3d.CalModel_GetCoreModel(model)
        meshCount = self.cal3d.CalCoreModel_GetCoreMeshCount(coreModel)
        result = {}
        for meshId in range(meshCount):
            mesh = self.cal3d.CalModel_GetMesh(model, meshId)
            submeshCount = self.cal3d.CalMesh_GetSubmeshCount(mesh)
            for subMeshId in range(submeshCount):
                r = self.cal3d.CalRenderer_SelectMeshSubmesh(renderer, meshId, subMeshId)
                assert r
                vertexCount = self.cal3d.CalRenderer_GetVertexCount(renderer)
                assert vertexCount > 0
                floatbuf = (ctypes.c_float * (vertexCount * 6))()
                r = self.cal3d.CalRenderer_GetVerticesAndNormals(renderer, floatbuf)
                assert r
                result[(meshId, subMeshId)] = [f for f in floatbuf]
        return result

    def testAllVertsEqual(self):
        model, meshIds, skelAnimIds, morphAnimIds = self.loadModelWithEverything('product80.cfl')
        self.cal3d.CalModel_Update(model, 0.0)

        av1 = self.getAllVerts(model)
        av2 = copy.deepcopy(av1)
        self.assertTrue(allVertsEqual(av1, av2))

    def testMorphAnimChangesVerts(self):
        model, meshIds, skelAnimIds, morphAnimIds = self.loadModelWithEverything('product80.cfl')
        self.cal3d.CalModel_Update(model, 0.0)
        av1 = self.getAllVerts(model)

        headMesh = self.cal3d.CalModel_GetMesh(model, meshIds['Female04_Anime01_HeadMorph.xmf'])
        faceSubMesh = self.cal3d.CalMesh_GetSubmesh(headMesh, 0)
        self.cal3d.CalSubmesh_SetMorphTargetWeight(faceSubMesh, 0, 0.5)

        av2 = self.getAllVerts(model)
        self.assertFalse(allVertsEqual(av1, av2))

        self.cal3d.CalSubmesh_SetMorphTargetWeight(faceSubMesh, 0, 0.0)

        av3 = self.getAllVerts(model)
        self.assertTrue(allVertsEqual(av1, av3))

    def _savePickle(self, value, fname):
        f = self.testDataFs.file(fname, 'wb')
        pickle.dump(value, f)
        f.close()
    def _loadPickle(self, fname):
        f = self.testDataFs.file(fname, 'rb')
        value = pickle.load(f)
        f.close()
        return value

    def testExpectedVertexResult(self):
        model, meshIds, skelAnimIds, morphAnimIds = self.loadModelWithEverything('product80.cfl')
        self.cal3d.CalModel_Update(model, 0.0)

        headMesh = self.cal3d.CalModel_GetMesh(model, meshIds['Female04_Anime01_HeadMorph.xmf'])
        faceSubMesh = self.cal3d.CalMesh_GetSubmesh(headMesh, 0)
        self.cal3d.CalSubmesh_SetMorphTargetWeight(faceSubMesh, morphAnimIds['Female04_Anime01_Idle_HappyFace05.xpf'], 0.2)
        self.cal3d.CalSubmesh_SetMorphTargetWeight(faceSubMesh, morphAnimIds['blink.xpf'], 0.2)

        pickleName = "expectedVertices1.pickle"
        av1 = self.getAllVerts(model)
        if 0: self._savePickle(av1, pickleName)

        av2 = self._loadPickle(pickleName)
        self.assertTrue(allVertsEqual(av1, av2))

if __name__ == '__main__':
    imvu.test.main()
