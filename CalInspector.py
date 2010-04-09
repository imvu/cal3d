import imvu
import hashlib
import logging
logger = logging.getLogger("imvu." + __name__)

from imvu import imvuctypes as ctypes

cal3d_dll = ctypes.imvudll('cal3d')

cal3d_dll.CalError_GetLastErrorText.restype = ctypes.c_char_p
cal3d_dll.CalCoreMaterial_GetMapFilename.restype = ctypes.c_char_p
cal3d_dll.CalCoreMaterial_GetMapType.restype = ctypes.c_char_p

gMaterialInfoCache = {}
gMeshInfoCache = {}

class Cal3dException(Exception): pass

def getMaterialInfo(materialBuffer):
    global gMaterialInfoCache
    cachekey = hashlib.md5(materialBuffer).hexdigest()
    cached = gMaterialInfoCache.get(cachekey, None)
    if cached: return cached

    material = cal3d_dll.CalLoader_LoadCoreMaterialFromBuffer(None, materialBuffer, len(materialBuffer))
    if not material:
        raise Cal3dException, cal3d_dll.CalError_GetLastErrorText()
    mapCount = cal3d_dll.CalCoreMaterial_GetMapCount(material)
    materialInfo = {}
    materialInfo['maps'] = {}
    for i in range(0, mapCount):
        mapType = cal3d_dll.CalCoreMaterial_GetMapType(material, i)
        mapName = cal3d_dll.CalCoreMaterial_GetMapFilename(material, i)
        materialInfo['maps'][mapType] = mapName
    gMaterialInfoCache[cachekey] = materialInfo
    return materialInfo

def getMeshInfo(meshBuffer):
    global gMeshInfoCache
    cachekey = hashlib.md5(meshBuffer).hexdigest()
    cached = gMeshInfoCache.get(cachekey, None)
    if cached:
        return cached

    mesh = cal3d_dll.CalLoader_LoadCoreMeshFromBuffer(None, meshBuffer, len(meshBuffer))
    if not mesh:
        raise Cal3dException, cal3d_dll.CalError_GetLastErrorText()
    submeshCount = cal3d_dll.CalCoreMesh_GetCoreSubmeshCount(mesh)
    result = {}
    for i in range(0, submeshCount):
        result[i] = {}
        submesh = cal3d_dll.CalCoreMesh_GetCoreSubmesh(mesh, i)
        result[i]['materialId'] = cal3d_dll.CalCoreSubmesh_GetCoreMaterialThreadId(submesh)
        result[i]['faceCount'] = cal3d_dll.CalCoreSubmesh_GetFaceCount(submesh)
        result[i]['lodCount'] = cal3d_dll.CalCoreSubmesh_GetLodCount(submesh)
        result[i]['vertexCount'] = cal3d_dll.CalCoreSubmesh_GetVertexCount(submesh)
    gMeshInfoCache[cachekey] = result
    return result
