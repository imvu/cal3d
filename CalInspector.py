import imvu
import hashlib
import logging
import mmap

import cal3d

logger = logging.getLogger("imvu." + __name__)

class Cal3dException(Exception):
    pass

def __mmap_to_string(b):
    if isinstance(b, mmap.mmap):
        b = b.read(b.size())
    return b

def getMaterialInfo(materialBuffer):
    materialBuffer = __mmap_to_string(materialBuffer)
    cachekey = hashlib.md5(materialBuffer).hexdigest()
    material = cal3d.loadCoreMaterialFromBuffer(materialBuffer)
    if not material:
        raise Cal3dException, cal3d_dll.CalError_GetLastErrorText()
    materialInfo = {}
    materialInfo['maps'] = {}
    for m in material.maps:
        materialInfo['maps'][m.type] = m.filename
    return materialInfo

def getMeshInfo(meshBuffer):
    meshBuffer = __mmap_to_string(meshBuffer)
    mesh = cal3d.loadCoreMeshFromBuffer(meshBuffer)
    if not mesh:
        raise Cal3dException, cal3d_dll.CalError_GetLastErrorText()
    result = {}
    for i, submesh in enumerate(mesh.submeshes):
        result[i] = {}
        result[i]['materialId'] = submesh.coreMaterialThreadId
        result[i]['faceCount'] = submesh.faceCount
        result[i]['lodCount'] = submesh.lodCount
        result[i]['vertexCount'] = submesh.vertexCount
    return result
