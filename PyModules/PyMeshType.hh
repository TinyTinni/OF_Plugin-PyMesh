#ifndef OPENMESH_PYTHON_MESHTYPES_HH
#define OPENMESH_PYTHON_MESHTYPES_HH

#define OM_FORCE_STATIC_CAST

#include <OpenMesh/Core/Mesh/Traits.hh>
#include <memory>
#include "../openmesh-python/src/MeshWrapperT.hh"
#include <pybind11/pybind11.h>


struct MeshTraits : public OpenMesh::DefaultTraits {
    /** Use double precision points */
    typedef OpenMesh::Vec3d Point;

    /** Use double precision normals */
    typedef OpenMesh::Vec3d Normal;

    /** Use RGBA colors */
    typedef OpenMesh::Vec4f Color;
};


using PyTriMesh = MeshWrapperT<OpenMesh::TriMesh_ArrayKernelT<MeshTraits> >;
using PyPolyMesh = MeshWrapperT<OpenMesh::PolyMesh_ArrayKernelT<MeshTraits> >;

template<typename T>
using HolderType = std::unique_ptr<T, pybind11::nodelete >;
#endif