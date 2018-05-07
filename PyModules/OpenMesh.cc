#include "OpenMesh.hh"
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <memory>
#include "OpenMesh-Python/src/MeshWrapperT.hh"

#define OPENMESH_PYTHON_MESHTYPES_HH
struct MeshTraits : public OpenMesh::DefaultTraits {
    /** Use double precision points */
    typedef OpenMesh::Vec3d Point;

    /** Use double precision normals */
    typedef OpenMesh::Vec3d Normal;

    /** Use RGBA colors */
    typedef OpenMesh::Vec4f Color;
};


using TriMesh = MeshWrapperT<OpenMesh::TriMesh_ArrayKernelT<MeshTraits> >;
using PolyMesh = MeshWrapperT<OpenMesh::PolyMesh_ArrayKernelT<MeshTraits> >;

template<typename T>
struct NoDeleter
{
    void operator()(T*) {}
};

template<typename T>
using HolderType = std::unique_ptr<T, NoDeleter<T>>;

#include "OpenMesh-Python/src/Bindings.cc"
#include "OpenMesh-Python/src/Miscellaneous.cc"
#include "OpenMesh-Python/src/InputOutput.cc"


#if (PY_MAJOR_VERSION == 2)
    PyObject* (*openmesh_pyinit_function)(void) = &initopenmesh;//untested
#elif (PY_MAJOR_VERSION == 3)
    PyObject* (*openmesh_pyinit_function)(void) = &PyInit_openmesh;
#endif
