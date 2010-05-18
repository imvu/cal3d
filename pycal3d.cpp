#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <cal3d/coreanimatedmorph.h>
#include <cal3d/corematerial.h>
#include <cal3d/coremesh.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/coresubmesh.h>
#include <cal3d/loader.h>
#include <cal3d/saver.h>

using namespace boost::python;

boost::shared_ptr<CalCoreSkeleton> loadCoreSkeletonFromBuffer(const std::string& buffer) {
    return boost::shared_ptr<CalCoreSkeleton>(
        CalLoader::loadCoreSkeletonFromBuffer(buffer.data(), buffer.size())
    );
}

bool saveCoreSkeleton(const boost::shared_ptr<CalCoreSkeleton>& skeleton, const std::string& path) {
    return CalSaver::saveCoreSkeleton(path, skeleton.get());
}

boost::shared_ptr<CalCoreMaterial> loadCoreMaterialFromBuffer(const std::string& buffer) {
    return boost::shared_ptr<CalCoreMaterial>(
        CalLoader::loadCoreMaterialFromBuffer(buffer.data(), buffer.size())
    );
}

bool saveCoreMaterial(const boost::shared_ptr<CalCoreMaterial>& material, const std::string& path) {
    return CalSaver::saveCoreMaterial(path, material.get());
}

boost::shared_ptr<CalCoreMesh> loadCoreMeshFromBuffer(const std::string& buffer) {
    return boost::shared_ptr<CalCoreMesh>(
        CalLoader::loadCoreMeshFromBuffer(buffer.data(), buffer.size())
    );
}

bool saveCoreMesh(const boost::shared_ptr<CalCoreMesh>& mesh, const std::string& path) {
    return CalSaver::saveCoreMesh(path, mesh.get());
}

boost::shared_ptr<CalCoreAnimatedMorph> loadCoreAnimatedMorphFromBuffer(const std::string& buffer) {
    return boost::shared_ptr<CalCoreAnimatedMorph>(
        CalLoader::loadCoreAnimatedMorphFromBuffer(buffer.data(), buffer.size())
    );
}

bool saveCoreAnimatedMorph(const boost::shared_ptr<CalCoreAnimatedMorph>& animatedMorph, const std::string& path) {
    return CalSaver::saveCoreAnimatedMorph(path, animatedMorph.get());
}

#ifndef NDEBUG
BOOST_PYTHON_MODULE(_cal3d_debug)
#else
BOOST_PYTHON_MODULE(_cal3d)
#endif
{
    class_<CalCoreSkeleton, boost::shared_ptr<CalCoreSkeleton> >("CoreSkeleton")
        ;

    {
        scope CalCoreMaterial_class(
            class_<CalCoreMaterial, boost::shared_ptr<CalCoreMaterial> >("CoreMaterial")
                .def_readwrite("maps", &CalCoreMaterial::maps)
        );

        class_< std::vector<CalCoreMaterial::Map> >("MapVector")
            .def(vector_indexing_suite<std::vector<CalCoreMaterial::Map> >())
            ;

        class_<CalCoreMaterial::Map>("Map")
            .def_readonly("filename", &CalCoreMaterial::Map::filename)
            .def_readonly("type", &CalCoreMaterial::Map::type)
            ;
    }

    class_<CalCoreSubmesh, boost::shared_ptr<CalCoreSubmesh>, boost::noncopyable>("CoreSubmesh", no_init)
        .add_property("coreMaterialThreadId", &CalCoreSubmesh::getCoreMaterialThreadId)
        .add_property("faceCount", &CalCoreSubmesh::getFaceCount)
        .add_property("lodCount", &CalCoreSubmesh::getLodCount)
        .add_property("vertexCount", &CalCoreSubmesh::getVertexCount)
        ;

    class_< CalCoreMesh::CalCoreSubmeshVector >("CalCoreSubmeshVector")
        .def(vector_indexing_suite<CalCoreMesh::CalCoreSubmeshVector, true>())
        ;

    class_<CalCoreMesh, boost::shared_ptr<CalCoreMesh> >("CoreMesh")
        .def_readwrite("submeshes", &CalCoreMesh::m_vectorCoreSubmesh)
        ;

    class_<CalCoreAnimatedMorph, boost::shared_ptr<CalCoreAnimatedMorph> >("CoreAnimatedMorph")
        ;

    def("loadCoreSkeletonFromBuffer", &loadCoreSkeletonFromBuffer);
    def("saveCoreSkeleton", &saveCoreSkeleton);

    def("loadCoreMaterialFromBuffer", &loadCoreMaterialFromBuffer);
    def("saveCoreMaterial", &saveCoreMaterial);

    def("loadCoreMeshFromBuffer", &loadCoreMeshFromBuffer);
    def("saveCoreMesh", &saveCoreMesh);

    def("loadCoreAnimatedMorphFromBuffer", &loadCoreAnimatedMorphFromBuffer);
    def("saveCoreAnimatedMorph", &saveCoreAnimatedMorph);
}
