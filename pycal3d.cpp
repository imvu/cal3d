#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <cal3d/coreanimation.h>
#include <cal3d/coreanimatedmorph.h>
#include <cal3d/corebone.h>
#include <cal3d/corematerial.h>
#include <cal3d/coremesh.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/coresubmesh.h>
#include <cal3d/loader.h>
#include <cal3d/saver.h>
#include <cal3d/error.h>

using namespace boost::python;

boost::shared_ptr<CalCoreAnimation> loadCoreAnimationFromBuffer(const std::string& buffer) {
    return boost::shared_ptr<CalCoreAnimation>(
        CalLoader::loadCoreAnimationFromBuffer(buffer.data(), buffer.size(), NULL)
    );
}

bool saveCoreAnimation(const boost::shared_ptr<CalCoreAnimation>& animation, const std::string& path) {
    return CalSaver::saveCoreAnimation(path, animation.get());
}

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
std::string saveCoreMaterialToString(const boost::shared_ptr<CalCoreMaterial>& material) {
    return CalSaver::saveCoreMaterialToString(material.get());
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

std::string saveCoreAnimatedMorphToBuffer(const boost::shared_ptr<CalCoreAnimatedMorph>& animatedMorph) {
    std::ostringstream os;
    CalSaver::saveCoreAnimatedMorph(os, animatedMorph.get());
    return os.str();
}

tuple getCoreSkeletonSceneAmbientColor(boost::shared_ptr<CalCoreSkeleton> skel) {
    CalVector sceneAmbient = skel->sceneAmbientColor;
    return make_tuple(
        sceneAmbient.x,
        sceneAmbient.y,
        sceneAmbient.z
    );
}

list getKeyframes(const CalCoreMorphTrack* t) {
    list rv;
    const std::vector<CalCoreMorphKeyframe>& keyframes = t->keyframes;
    for (
        std::vector<CalCoreMorphKeyframe>::const_iterator i = keyframes.begin();
        i != keyframes.end();
        ++i
    ) {
        rv.append(*i);
    }
    return rv;
}

list getTracks(const CalCoreAnimatedMorph* m) {
    list rv;
    const std::vector<CalCoreMorphTrack>& tracks = m->tracks;
    for (
        std::vector<CalCoreMorphTrack>::const_iterator i = tracks.begin();
        i != tracks.end();
        ++i
    ) {
        rv.append(*i);
    }
    return rv;
}

#ifndef NDEBUG
BOOST_PYTHON_MODULE(_cal3d_debug)
#else
BOOST_PYTHON_MODULE(_cal3d)
#endif
{
    class_<CalCoreBone, boost::shared_ptr<CalCoreBone> >("CoreBone", no_init)
        .def(init<std::string>())
        .add_property("name", &CalCoreBone::name)
        ;
    class_< std::vector<boost::shared_ptr<CalCoreBone> > >("BoneVector")
        .def(vector_indexing_suite< std::vector<boost::shared_ptr<CalCoreBone> >, true >())
        ;

    class_<CalCoreSkeleton, boost::shared_ptr<CalCoreSkeleton> >("CoreSkeleton")
        .def("addCoreBone", &CalCoreSkeleton::addCoreBone)
        .add_property("sceneAmbientColor", &getCoreSkeletonSceneAmbientColor)
        .add_property("bones", &CalCoreSkeleton::coreBones)
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
            .def_readwrite("filename", &CalCoreMaterial::Map::filename)
            .def_readwrite("type", &CalCoreMaterial::Map::type)
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

    class_<CalCoreAnimation, boost::shared_ptr<CalCoreAnimation> >("CoreAnimation")
        ;

    class_<CalCoreMorphKeyframe>("CoreMorphKeyframe")
        .add_property("time", &CalCoreMorphKeyframe::time)
        .add_property("weight", &CalCoreMorphKeyframe::weight)
        ;

    class_<CalCoreMorphTrack, boost::shared_ptr<CalCoreMorphTrack> >("CoreMorphTrack")
        .def_readonly("name", &CalCoreMorphTrack::morphName)
        .add_property("keyframes", &getKeyframes)
        ;

    class_<CalCoreAnimatedMorph, boost::shared_ptr<CalCoreAnimatedMorph> >("CoreAnimatedMorph")
        .def("removeZeroScaleTracks", &CalCoreAnimatedMorph::removeZeroScaleTracks)
        .def_readonly("duration", &CalCoreAnimatedMorph::duration)
        .add_property("tracks", &getTracks)
        ;

    def("loadCoreAnimationFromBuffer", &loadCoreAnimationFromBuffer);
    def("saveCoreAnimation", &saveCoreAnimation);

    def("loadCoreSkeletonFromBuffer", &loadCoreSkeletonFromBuffer);
    def("saveCoreSkeleton", &saveCoreSkeleton);

    def("loadCoreMaterialFromBuffer", &loadCoreMaterialFromBuffer);
    def("saveCoreMaterial", &saveCoreMaterial);
    def("saveCoreMaterialToString", &saveCoreMaterialToString);

    def("loadCoreMeshFromBuffer", &loadCoreMeshFromBuffer);
    def("saveCoreMesh", &saveCoreMesh);

    def("loadCoreAnimatedMorphFromBuffer", &loadCoreAnimatedMorphFromBuffer);
    def("saveCoreAnimatedMorphToBuffer", &saveCoreAnimatedMorphToBuffer);
    def("saveCoreAnimatedMorph", &saveCoreAnimatedMorph);

    def("getLastErrorText", &CalError::getLastErrorText);
}
