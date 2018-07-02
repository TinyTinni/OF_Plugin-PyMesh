#include "OpenMesh.hh"

#include "PyMeshType.hh"

using TriMesh = PyTriMesh;
using PolyMesh = PyPolyMesh;

#include "openmesh-python/src/Bindings.cc"
#include "openmesh-python/src/Miscellaneous.cc"
#include "openmesh-python/src/InputOutput.cc"


#if (PY_MAJOR_VERSION == 2)
    PyObject* (*openmesh_pyinit_function)(void) = &initopenmesh;//untested
#elif (PY_MAJOR_VERSION == 3)
    PyObject* (*openmesh_pyinit_function)(void) = &PyInit_openmesh;
#endif
