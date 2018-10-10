/*
 * Copyright (c) 2016-2018 Matthias Möller <m_moeller@live.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of mosquitto nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
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
