#include <vector>

#include <boost/math/constants/constants.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <cal3d/buffersource.h>
#include <cal3d/coreanimation.h>
#include <cal3d/coremorphanimation.h>
#include <cal3d/corebone.h>
#include <cal3d/corematerial.h>
#include <cal3d/coremesh.h>
#include <cal3d/coreskeleton.h>
#include <cal3d/coresubmesh.h>
#include <cal3d/loader.h>
#include <cal3d/saver.h>
#include <cal3d/error.h>

using namespace boost::python;

CalCoreAnimationPtr loadCoreAnimationFromBuffer(const cal3d::Buffer& buffer) {
    CalBufferSource cbs(buffer.data(), buffer.size());
    return CalLoader::loadCoreAnimation(cbs);
}

bool saveCoreAnimation(const boost::shared_ptr<CalCoreAnimation>& animation, const std::string& path) {
    return CalSaver::saveCoreAnimation(path, animation.get());
}

CalCoreSkeletonPtr loadCoreSkeletonFromBuffer(const cal3d::Buffer& buffer) {
    CalBufferSource cbs(buffer.data(), buffer.size());
    return CalLoader::loadCoreSkeleton(cbs);
}

bool saveCoreSkeleton(const boost::shared_ptr<CalCoreSkeleton>& skeleton, const std::string& path) {
    return CalSaver::saveCoreSkeleton(path, skeleton.get());
}

CalCoreMaterialPtr loadCoreMaterialFromBuffer(const cal3d::Buffer& buffer) {
    CalBufferSource cbs(buffer.data(), buffer.size());
    return CalLoader::loadCoreMaterial(cbs);
}

bool saveCoreMaterial(const boost::shared_ptr<CalCoreMaterial>& material, const std::string& path) {
    return CalSaver::saveCoreMaterial(path, material.get());
}

CalCoreMeshPtr loadCoreMeshFromBuffer(const cal3d::Buffer& buffer) {
    CalBufferSource cbs(buffer.data(), buffer.size());
    return CalLoader::loadCoreMesh(cbs);
}

bool saveCoreMesh(const boost::shared_ptr<CalCoreMesh>& mesh, const std::string& path) {
    return CalSaver::saveCoreMesh(path, mesh.get());
}

CalCoreMorphAnimationPtr loadCoreMorphAnimationFromBuffer(const cal3d::Buffer& buffer) {
    CalBufferSource cbs(buffer.data(), buffer.size());
    return CalLoader::loadCoreMorphAnimation(cbs);
}

bool saveCoreMorphAnimation(const boost::shared_ptr<CalCoreMorphAnimation>& animatedMorph, const std::string& path) {
    return CalSaver::saveCoreMorphAnimation(path, animatedMorph.get());
}

tuple getCoreSkeletonSceneAmbientColor(const CalCoreSkeletonPtr& skel) {
    CalVector sceneAmbient = skel->sceneAmbientColor;
    return make_tuple(
        sceneAmbient.x,
        sceneAmbient.y,
        sceneAmbient.z
    );
}

void setCoreSkeletonSceneAmbientColor(const CalCoreSkeletonPtr& skel, tuple c) {
    if (len(c) != 3) {
        PyErr_SetString(PyExc_ValueError, "sceneAmbientColor must be a triple");
        throw_error_already_set();
    }
    skel->sceneAmbientColor.x = extract<float>(c[0]);
    skel->sceneAmbientColor.y = extract<float>(c[1]);
    skel->sceneAmbientColor.z = extract<float>(c[2]);
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

list getTracks(const CalCoreMorphAnimation* m) {
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

namespace cal3d {
    struct PythonBuffer : public Buffer {
    public:
        PythonBuffer(PyObject* p) {
            _ = p;
            incref(get());
        }

        ~PythonBuffer() {
            boost::python::PyGILState_AssertIsCurrent();
            decref(get());
        }

        size_t size() const {
            void* data;
            return get()->ob_type->tp_as_buffer->bf_getreadbuffer(get(), 0, &data);
        }

        const void* data() const {
            void* data;
            get()->ob_type->tp_as_buffer->bf_getreadbuffer(get(), 0, &data);
            return data;
        }

        boost::shared_ptr<Buffer> clone() const {
            return boost::shared_ptr<Buffer>(new PythonBuffer(get()));
        }
    private:
        PyObject* get() const {
            return static_cast<PyObject*>(_);
        }
    };

    struct BufferFromPythonObject {
        BufferFromPythonObject() {
            converter::registry::push_back(
                &convertible,
                &construct,
                boost::python::type_id<Buffer>()
            );
        }

        static void* convertible(PyObject* obj_ptr) {
            return (obj_ptr->ob_type->tp_as_buffer &&
                    obj_ptr->ob_type->tp_as_buffer->bf_getreadbuffer)
                   ? obj_ptr
                   : 0;
        }

        static void construct(
            PyObject* obj_ptr,
            boost::python::converter::rvalue_from_python_stage1_data* data
        ) {
            // Note that we're registered as a converter to the Buffer interface,
            // so Boost has already allocated sizeof(Buffer) for us.  Since we're
            // constructing PythonStringBuffer into that memory, assert that
            // PythonStringBuffer and Buffer have the same size.
            BOOST_STATIC_ASSERT(sizeof(Buffer) == sizeof(PythonBuffer));

            void* storage = ((boost::python::converter::rvalue_from_python_storage<PythonBuffer>*)data)->storage.bytes;

            new(storage) PythonBuffer(obj_ptr);

            data->convertible = storage;
        }
    };
}

template<unsigned i>
CalIndex getFaceIndex(const CalCoreSubmesh::Face& f) {
    return f.vertexId[i];
}

struct PythonVertex {
    PythonVertex() {}
    PythonVertex(const CalCoreSubmesh::Vertex& v) {
        position.x = v.position.x;
        position.y = v.position.y;
        position.z = v.position.z;

        normal.x = v.normal.x;
        normal.y = v.normal.y;
        normal.z = v.normal.z;
    }
    CalVector position;
    CalVector normal;

    bool operator==(const PythonVertex& rhs) const {
        return position == rhs.position && normal == rhs.normal;
    }
};

std::vector<PythonVertex> getVertices(const CalCoreSubmesh& submesh) {
    return std::vector<PythonVertex>(
        submesh.getVectorVertex().begin(),
        submesh.getVectorVertex().end());
}

template<typename T>
void exportVector(const char* name) {
    class_<std::vector<T> >(name)
        .def(vector_indexing_suite< std::vector<T>, true>())
        ;
}

std::string VectorRepr(const CalVector& v) {
    std::ostringstream os;
    os << "cal3d.Vector(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os.str();
}

std::string QuaternionRepr(const CalQuaternion& q) {
    float h2 = acos(-q.w);
    float angle = h2 * 360 / boost::math::constants::pi<float>();
    float s = sin(h2);

    std::ostringstream os;
    os << "cal3d.Quaternion(angle=" << angle << ", axis=(" << (q.x / s) << ", " << (q.y / s) << ", " << (q.z / s) << "))";
    return os.str();
}

std::string TransformRepr(const cal3d::Transform& t) {
    return "cal3d.Transform(" + QuaternionRepr(t.rotation) + ", " + VectorRepr(t.translation) + ")";
}

#ifndef NDEBUG
BOOST_PYTHON_MODULE(_cal3d_debug)
#else
BOOST_PYTHON_MODULE(_cal3d)
#endif
{
    cal3d::BufferFromPythonObject();

    class_<CalVector>("Vector")
        .def("__repr__", &VectorRepr)
        .def_readwrite("x", &CalVector::x)
        .def_readwrite("y", &CalVector::y)
        .def_readwrite("z", &CalVector::z)
        ;

    class_<CalQuaternion>("Quaternion")
        .def("__repr__", &QuaternionRepr)
        .def_readwrite("x", &CalQuaternion::x)
        .def_readwrite("y", &CalQuaternion::y)
        .def_readwrite("z", &CalQuaternion::z)
        .def_readwrite("w", &CalQuaternion::w)
        ;

    class_<cal3d::Transform>("Transform")
        .def("__repr__", &TransformRepr)
        .def_readwrite("translation", &cal3d::Transform::translation)
        .def_readwrite("rotation", &cal3d::Transform::rotation)
        ;

    class_<CalCoreBone, boost::shared_ptr<CalCoreBone> >("CoreBone", no_init)
        .def(init<std::string>())
        .def_readwrite("parentIndex", &CalCoreBone::parentId)
        .def_readonly("name", &CalCoreBone::name)
        .def_readwrite("relativeTransform", &CalCoreBone::relativeTransform)
        .def_readwrite("boneSpaceTransform", &CalCoreBone::boneSpaceTransform)
        ;

    exportVector<CalCoreBonePtr>("BoneVector");

    class_<CalCoreSkeleton, boost::shared_ptr<CalCoreSkeleton>, boost::noncopyable>("CoreSkeleton")
        .def("addCoreBone", &CalCoreSkeleton::addCoreBone)
        .def("scale", &CalCoreSkeleton::scale)
        .add_property("sceneAmbientColor", &getCoreSkeletonSceneAmbientColor, &setCoreSkeletonSceneAmbientColor)
        .add_property("bones", make_function(&CalCoreSkeleton::getCoreBones, return_value_policy<return_by_value>()))
        ;

    {
        scope CalCoreMaterial_class(
            class_<CalCoreMaterial, boost::shared_ptr<CalCoreMaterial> >("CoreMaterial")
                .def_readwrite("maps", &CalCoreMaterial::maps)
        );

        exportVector<CalCoreMaterial::Map>("MapVector");

        class_<CalCoreMaterial::Map>("Map")
            .def_readwrite("filename", &CalCoreMaterial::Map::filename)
            .def_readwrite("type", &CalCoreMaterial::Map::type)
            ;
    }

    class_<CalCoreSubmesh::Face>("Triangle")
        .add_property("v1", &getFaceIndex<0>)
        .add_property("v2", &getFaceIndex<1>)
        .add_property("v3", &getFaceIndex<2>)
        ;

    exportVector<CalCoreSubmesh::Face>("TriangleVector");

    class_<PythonVertex>("Vertex")
        .def_readwrite("position", &PythonVertex::position)
        .def_readwrite("normal", &PythonVertex::normal)
        ;

    exportVector<PythonVertex>("VertexVector");

    class_<CalCoreSubmesh::TextureCoordinate>("TextureCoordinate")
        .def_readwrite("u", &CalCoreSubmesh::TextureCoordinate::u)
        .def_readwrite("v", &CalCoreSubmesh::TextureCoordinate::v)
        ;

    exportVector<CalCoreSubmesh::TextureCoordinate>("TextureCoordinateVector");
    exportVector<std::vector<CalCoreSubmesh::TextureCoordinate> >("TextureCoordinateVectorVector");

    class_<CalCoreSubmesh::Influence>("Influence")
        .def_readwrite("boneId", &CalCoreSubmesh::Influence::boneId)
        .def_readwrite("weight", &CalCoreSubmesh::Influence::weight)
        .def_readwrite("isLast", &CalCoreSubmesh::Influence::lastInfluenceForThisVertex)
        ;

    exportVector<CalCoreSubmesh::Influence>("InfluenceVector");

    class_<CalCoreSubmesh, boost::shared_ptr<CalCoreSubmesh>, boost::noncopyable>("CoreSubmesh", no_init)
        .def_readwrite("coreMaterialThreadId", &CalCoreSubmesh::coreMaterialThreadId)
        .def_readwrite("triangles", &CalCoreSubmesh::faces)
        .add_property("vertices", &getVertices)
        .add_property("hasVertexColors", &CalCoreSubmesh::hasVertexColors)
        .add_property("colors", make_function(&CalCoreSubmesh::getVertexColors, return_value_policy<return_by_value>()))
        .add_property("texcoords", make_function(&CalCoreSubmesh::getVectorVectorTextureCoordinate, return_value_policy<return_by_value>()))
        .add_property("influences", make_function(&CalCoreSubmesh::getInfluences, return_value_policy<return_by_value>()))
        ;

    exportVector<CalCoreSubmeshPtr>("CoreSubmeshVector");

    class_<CalCoreMesh, CalCoreMeshPtr>("CoreMesh")
        .def_readwrite("submeshes", &CalCoreMesh::submeshes)
        .def("scale", &CalCoreMesh::scale)
        .def("fixup", &CalCoreMesh::fixup)
        ;

    class_<CalCoreKeyframe>("CoreKeyframe")
        .def_readwrite("time", &CalCoreKeyframe::time)
        .def_readwrite("transform", &CalCoreKeyframe::transform)
        ;

    exportVector<CalCoreKeyframe>("CoreKeyframeVector");

    class_<CalCoreTrack>("CoreTrack", init<int, const CalCoreTrack::KeyframeList&>())
        .def_readwrite("coreBoneId", &CalCoreTrack::coreBoneId)
        .def_readwrite("keyframes", &CalCoreTrack::keyframes)
        ;

    exportVector<CalCoreTrack>("CoreTrackVector");

    class_<CalCoreAnimation, CalCoreAnimationPtr>("CoreAnimation")
        .def("scale", &CalCoreAnimation::scale)
        .def("fixup", &CalCoreAnimation::fixup)
        .def_readwrite("duration", &CalCoreAnimation::duration)
        .def_readwrite("tracks", &CalCoreAnimation::tracks)
        ;

    class_<CalCoreMorphKeyframe>("CoreMorphKeyframe")
        .add_property("time", &CalCoreMorphKeyframe::time)
        .add_property("weight", &CalCoreMorphKeyframe::weight)
        ;

    class_<CalCoreMorphTrack, CalCoreMorphTrackPtr>("CoreMorphTrack")
        .def_readonly("name", &CalCoreMorphTrack::morphName)
        .add_property("keyframes", &getKeyframes)
        ;

    class_<CalCoreMorphAnimation, CalCoreMorphAnimationPtr>("CoreMorphAnimation")
        .def("removeZeroScaleTracks", &CalCoreMorphAnimation::removeZeroScaleTracks)
        .def("scale", &CalCoreMorphAnimation::scale)
        .def_readonly("duration", &CalCoreMorphAnimation::duration)
        .add_property("tracks", &getTracks)
        ;

    def("loadCoreAnimationFromBuffer", &loadCoreAnimationFromBuffer);
    def("saveCoreAnimation", &saveCoreAnimation);
    def("saveCoreAnimationToBuffer", &CalSaver::saveCoreAnimationToBuffer);

    def("loadCoreSkeletonFromBuffer", &loadCoreSkeletonFromBuffer);
    def("saveCoreSkeleton", &saveCoreSkeleton);
    def("saveCoreSkeletonToBuffer", &CalSaver::saveCoreSkeletonToBuffer);

    def("loadCoreMaterialFromBuffer", &loadCoreMaterialFromBuffer);
    def("saveCoreMaterial", &saveCoreMaterial);
    def("saveCoreMaterialToBuffer", &CalSaver::saveCoreMaterialToBuffer);

    def("loadCoreMeshFromBuffer", &loadCoreMeshFromBuffer);
    def("saveCoreMesh", &saveCoreMesh);
    def("saveCoreMeshToBuffer", &CalSaver::saveCoreMeshToBuffer);

    def("loadCoreMorphAnimationFromBuffer", &loadCoreMorphAnimationFromBuffer);
    def("saveCoreMorphAnimation", &saveCoreMorphAnimation);
    def("saveCoreMorphAnimationToBuffer", &CalSaver::saveCoreMorphAnimationToBuffer);

    def("getLastErrorText", &CalError::getLastErrorText);
}
