/** @file */

#ifndef OPENMESH_PYTHON_MESHTYPES_HH
#define OPENMESH_PYTHON_MESHTYPES_HH

#define OM_STATIC_BUILD

#include "MeshWrapperT.hh"

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <memory>

namespace py = pybind11;


struct MeshTraits : public OpenMesh::DefaultTraits {
	/** Use double precision points */
	typedef OpenMesh::Vec3d Point;

	/** Use double precision normals */
	typedef OpenMesh::Vec3d Normal;

	/** Use RGBA colors */
	typedef OpenMesh::Vec4f Color;

	/** Use double precision texcoords */
	typedef double TexCoord1D;
	typedef OpenMesh::Vec2d TexCoord2D;
	typedef OpenMesh::Vec3d TexCoord3D;
};


using TriMesh = MeshWrapperT<OpenMesh::TriMesh_ArrayKernelT<MeshTraits> >;
using PolyMesh = MeshWrapperT<OpenMesh::PolyMesh_ArrayKernelT<MeshTraits> >;

template<typename T>
using HolderType = std::unique_ptr<T>;

#endif
