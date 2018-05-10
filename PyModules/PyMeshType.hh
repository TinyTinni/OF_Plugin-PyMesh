#include <OpenMesh/Core/Mesh/Traits.hh>
#include <memory>
#include "../OpenMesh-Python/src/MeshWrapperT.hh"

#ifndef OPENMESH_PYTHON_MESHTYPES_HH
#define OPENMESH_PYTHON_MESHTYPES_HH
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
struct NoDeleter
{
    void operator()(T*) {}
};

template<typename T>
using HolderType = std::unique_ptr<T, NoDeleter<T>>;
#endif