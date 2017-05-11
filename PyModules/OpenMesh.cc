#include "OpenMesh.hh"
#include <OpenMesh/src/Python/Bindings.cc>

#if (_MSC_VER == 1900 && !defined(DECL_BOOST_MISSING_GET_POINTER))
//workaround for msvc 2015 update 3
#define DECL_BOOST_MISSING_GET_POINTER(X)\
namespace boost\
{\
    template <>\
    X const volatile * get_pointer<X const volatile>(X const volatile *c)\
    {\
        return c;\
    }\
}

DECL_BOOST_MISSING_GET_POINTER(OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits>)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::PolyConnectivity)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModAspectRatioT<OpenMesh::PolyMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModRoundnessT<class OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModQuadricT<class OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModProgMeshT<class OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModNormalFlippingT<class OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModNormalDeviationT<class OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModIndependentSetsT<class OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModHausdorffT<class OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModEdgeLengthT<class OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModAspectRatioT<class OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModRoundnessT<class OpenMesh::PolyMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModQuadricT<class OpenMesh::PolyMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModProgMeshT<class OpenMesh::PolyMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModNormalFlippingT<class OpenMesh::PolyMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModNormalDeviationT<class OpenMesh::PolyMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModIndependentSetsT<class OpenMesh::PolyMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModHausdorffT<class OpenMesh::PolyMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
DECL_BOOST_MISSING_GET_POINTER(OpenMesh::Decimater::ModEdgeLengthT<class OpenMesh::PolyMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> >)
#endif

#if (PY_MAJOR_VERSION == 2)
    PyObject* (*openmesh_pyinit_function)(void) = &OpenMesh::Python::initopenmesh;//untested
#elif (PY_MAJOR_VERSION == 3)
    PyObject* (*openmesh_pyinit_function)(void) = &OpenMesh::Python::PyInit_openmesh;
#endif
