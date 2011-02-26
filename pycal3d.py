import os
import tempfile

from imvu import imvuctypes
import cal3d

class Cal3d:
    def __init__(self):
        self.typeToBinaryExtensionMap_ = { "CoreAnimation" : "caf",
                                           "CoreMaterial"  : "crf",
                                           "CoreAnimatedMorph"  : "cpf",
                                           "CoreMesh"      : "cmf",
                                           "CoreSkeleton"  : "csf" }
        self.typeToXmlExtensionMap_ = { "CoreAnimation" : "xaf",
                                        "CoreMaterial"  : "xrf",
                                        "CoreAnimatedMorph"  : "xpf",
                                        "CoreMesh"      : "xmf",
                                        "CoreSkeleton"  : "xsf" }
        self.xmlExtensionToTypeMap_ = {}
        for key, value in self.typeToXmlExtensionMap_.items() + self.typeToBinaryExtensionMap_.items():
            self.xmlExtensionToTypeMap_[".%s" % value] = key

    def cal3dTypeForPath(self, path):
        name, ext = os.path.splitext(path)
        try:
            return self.xmlExtensionToTypeMap_[ext]
        except KeyError:
            return None
        
    def convertToBinary(self, calCoreType, data):
        extension = self.typeToBinaryExtensionMap_[calCoreType]
        return self.__convert(calCoreType=calCoreType, data=data, extension=extension)

    def convertToXml(self, calCoreType, data):
        extension = self.typeToXmlExtensionMap_[calCoreType]
        return self.__convert(calCoreType=calCoreType, data=data, extension=extension)

    def getFormat(self, data):
        for calCoreType in self.typeToBinaryExtensionMap_.keys():
            loaderFunc = getattr(cal3d, "load"+calCoreType+"FromBuffer")
            if not loaderFunc:
                raise "could not find Loader for calCoreType %s" % calCoreType

            obj = loaderFunc(data)
            if obj:
                if data.startswith('<HEADER'):
                    binary_flag = 'XML' # not really, two root document elements, but that's another story ;_;
                else:
                    binary_flag = 'Binary'

                return '%s,%s' % (calCoreType, binary_flag)

        return None

    def __convert(self, calCoreType, data, extension):
        loaderFunc = getattr(cal3d, "load"+calCoreType+"FromBuffer")
        saverFunc = getattr(cal3d, "save"+calCoreType)
        if not loaderFunc:
            raise Exception("could not find Loader for calCoreType %s" % calCoreType)
        if not saverFunc:
            raise Exception("could not find Saver for calCoreType %s" % calCoreType)
        object = loaderFunc(data)
        if not object:
            cal3d_dll.CalError_PrintLastError()
            raise Exception("could not load data (len %s) for calCoreType %s" % (len(data), calCoreType))
        path = "%s.%s" % (tempfile.mktemp(), extension)
        try:
            r = saverFunc(object, path)
            if not r:
                cal3d_dll.CalError_PrintLastError();
                raise Exception("could not save object 0x%x (calCoreType=%s) to path %s" % (object, calCoreType, path))
            f = open(path, "rb")
            ret = f.read()
            f.close()
        finally:
            if os.path.exists(path):
                os.remove(path)

        return ret

