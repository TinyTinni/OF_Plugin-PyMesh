/** @file */

#ifndef OPENMESH_PYTHON_MESHTYPES_HH
#define OPENMESH_PYTHON_MESHTYPES_HH

#define OM_STATIC_BUILD

#include "MeshWrapperT.hh"
#include <memory>

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


typedef MeshWrapperT<OpenMesh::TriMesh_ArrayKernelT<MeshTraits> > TriMesh;
typedef MeshWrapperT<OpenMesh::PolyMesh_ArrayKernelT<MeshTraits> > PolyMesh;

template<typename T>
using HolderType = std::unique_ptr<T>;

#endif
