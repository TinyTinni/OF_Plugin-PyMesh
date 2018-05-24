#include "OpenMesh.hh"

#include "PyMeshType.hh"

using TriMesh = PyTriMesh;
using PolyMesh = PyPolyMesh;

#include "OpenMesh-Python/src/Bindings.cc"
#include "OpenMesh-Python/src/Miscellaneous.cc"
#include "OpenMesh-Python/src/InputOutput.cc"


#if (PY_MAJOR_VERSION == 2)
    PyObject* (*openmesh_pyinit_function)(void) = &initopenmesh;//untested
#elif (PY_MAJOR_VERSION == 3)
    PyObject* (*openmesh_pyinit_function)(void) = &PyInit_openmesh;
#endif
