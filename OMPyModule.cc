#include "OMPyModule.hh"
#include <OpenMesh/src/Python/Bindings.cc>

//fix for msvc 2015 update 3, delete in future
namespace boost
{
    template <>
    OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> const volatile * get_pointer<OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> const volatile >(
        OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> const volatile *c)
    {
        return c;
    }

}

#if (PY_MAJOR_VERSION == 2)
    PyObject* (*openmesh_pyinit_function)(void) = &OpenMesh::Python::initopenmesh;//untested
#elif (PY_MAJOR_VERSION == 3)
    PyObject* (*openmesh_pyinit_function)(void) = &OpenMesh::Python::PyInit_openmesh;
#endif
