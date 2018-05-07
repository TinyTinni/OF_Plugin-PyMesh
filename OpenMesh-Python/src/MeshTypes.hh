/** @file */

#ifndef OPENMESH_PYTHON_MESHTYPES_HH
#define OPENMESH_PYTHON_MESHTYPES_HH

#define OM_STATIC_BUILD

#include "Utilities.hh"

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

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


template <class Mesh>
class MeshWrapperT : public Mesh {
public:

	typedef OpenMesh::VPropHandleT<py::none> VPropHandle;
	typedef OpenMesh::HPropHandleT<py::none> HPropHandle;
	typedef OpenMesh::EPropHandleT<py::none> EPropHandle;
	typedef OpenMesh::FPropHandleT<py::none> FPropHandle;

	template <class Handle, class PropHandle>
	py::none py_property(const std::string& _name, Handle _h) {
		const auto prop = py_prop_on_demand<Handle, PropHandle>(_name);
		return Mesh::property(prop, _h);
	}

	template <class Handle, class PropHandle>
	void py_set_property(const std::string& _name, Handle _h, py::object _val) {
		const auto prop = py_prop_on_demand<Handle, PropHandle>(_name);
		Mesh::property(prop, _h) = _val;
	}

	template <class Handle>
	bool py_has_property(const std::string& _name) {
        HandleToPropHandle<Handle> dummy;
        return Mesh::get_property_handle(dummy, _name);
	}

	template <class Handle>
	void py_remove_property(const std::string& _name) {
        HandleToPropHandle<Handle> h;
        Mesh::get_property_handle(h, _name);
        Mesh::remove_property(h);
	}

	template <class Handle, class PropHandle>
	py::list py_property_generic(const std::string& _name) {
		const size_t n = py_n_items(Handle());
		const auto prop = py_prop_on_demand<Handle, PropHandle>(_name);

		py::list res;
		for (size_t i = 0; i < n; ++i) {
			res.append(Mesh::property(prop, Handle(i)));
		}
		return res;
	}

	template <class Handle, class PropHandle>
	void py_set_property_generic(const std::string& _name, py::list _list) {
		const size_t n = py_n_items(Handle());
		const auto prop = py_prop_on_demand<Handle, PropHandle>(_name);

		if (_list.size() != n) {
			return;
		}
		for (size_t i = 0; i < n; ++i) {
			Mesh::property(prop, Handle(i)) = py::object(_list[i]);
		}
	}

	template <class Handle, class PropHandle>
	py::array_t<double> py_property_array(const std::string& _name) {
		const size_t n = py_n_items(Handle());
		const auto prop = py_prop_on_demand<Handle, PropHandle>(_name);

		// assume that all arrays have the same size and
		// retrieve the size of the first array
		const py::object tmp_obj = Mesh::property(prop, Handle(0));
		const auto first_arr = make_c_style(tmp_obj.cast<py::array_t<double> >());
		const size_t size = first_arr.size();

		// better check this now
		if (size == 0) {
			PyErr_SetString(PyExc_RuntimeError, "One of the arrays has size 0.");
			throw py::error_already_set();
		}

		// preserve array shape and strides
		std::vector<size_t> shape({n});
		std::vector<size_t> strides({size * sizeof(double)});
		shape.insert(shape.end(), first_arr.shape(), first_arr.shape() + first_arr.ndim());
		strides.insert(strides.end(), first_arr.strides(), first_arr.strides() + first_arr.ndim());

		// allocate memory
		double *data = new double[size * n];
		py::capsule base = free_when_done(data);

		// copy one array at a time
		for (size_t i = 0; i < n; ++i) {
			const Handle hnd(i);
			const py::object obj = Mesh::property(prop, hnd);
			const auto arr = make_c_style(obj.cast<py::array_t<double> >());
			if (arr.size() != first_arr.size()) {
				PyErr_SetString(PyExc_RuntimeError, "Array sizes do not match.");
				throw py::error_already_set();
			}
			if (arr.ndim() != first_arr.ndim()) {
				PyErr_SetString(PyExc_RuntimeError, "Array dimensions do not match.");
				throw py::error_already_set();
			}
			if (!std::equal(arr.shape(), arr.shape() + arr.ndim(), first_arr.shape())) {
				PyErr_SetString(PyExc_RuntimeError, "Array shapes do not match.");
				throw py::error_already_set();
			}
			std::copy(arr.data(0), arr.data(0) + size, &data[size * i]);
		}

		return py::array_t<double>(shape, strides, data, base);
	}

	template <class Handle, class PropHandle>
	void py_set_property_array(const std::string& _name, py::array_t<double, py::array::c_style | py::array::forcecast> _arr) {
		const size_t n = py_n_items(Handle());
		const auto prop = py_prop_on_demand<Handle, PropHandle>(_name);

		// array cannot be empty and its shape has to be (_n, m,...)
		if (_arr.size() == 0 || _arr.ndim() < 2 || _arr.shape(0) != n) {
			return;
		}

		// copy one array at a time
		const size_t size = _arr.strides(0) / sizeof(double);
		for (size_t i = 0; i < n; ++i) {
			double *data = new double[size];
			std::copy(_arr.data(i), _arr.data(i) + size, data);
			const std::vector<size_t> shape(_arr.shape() + 1, _arr.shape() + _arr.ndim());
			const std::vector<size_t> strides(_arr.strides() + 1, _arr.strides() + _arr.ndim());
			py::capsule base = free_when_done(data);
			py::array_t<double> tmp(shape, strides, data, base);
			Mesh::property(prop, Handle(i)) = tmp;
		}
	}

	template <class Handle, class PropHandle>
	void py_copy_property(const std::string& _name, Handle _from, Handle _to) {
		auto prop = py_prop_on_demand<Handle, PropHandle>(_name);
		Mesh::copy_property(prop, _from, _to);
	}

	py::object py_copy() {
		return py::cast(MeshWrapperT(*this));
	}

	py::object py_deepcopy(py::dict _memo) {
		#if PY_MAJOR_VERSION < 3
		py::object id = py::module::import("__builtin__").attr("id");
		#else
		py::object id = py::module::import("builtins").attr("id");
		#endif
		py::object deepcopy = py::module::import("copy").attr("deepcopy");

		MeshWrapperT *copy = new MeshWrapperT(*this);
		py::object copy_pyobj = py::cast(copy, py::return_value_policy::take_ownership);
		_memo[id(py::cast(this))] = copy_pyobj;

		py_deepcopy_prop<OpenMesh::VertexHandle>(copy, deepcopy, _memo);
		py_deepcopy_prop<OpenMesh::HalfedgeHandle>(copy, deepcopy, _memo);
		py_deepcopy_prop<OpenMesh::EdgeHandle>(copy, deepcopy, _memo);
		py_deepcopy_prop<OpenMesh::FaceHandle>(copy, deepcopy, _memo);

		return copy_pyobj;
	}

	size_t py_n_items(OpenMesh::VertexHandle) const { return Mesh::n_vertices(); }
	size_t py_n_items(OpenMesh::HalfedgeHandle) const { return Mesh::n_halfedges(); }
	size_t py_n_items(OpenMesh::EdgeHandle) const { return Mesh::n_edges(); }
	size_t py_n_items(OpenMesh::FaceHandle) const { return Mesh::n_faces(); }

	size_t py_has_status(OpenMesh::VertexHandle) const { return Mesh::has_vertex_status(); }
	size_t py_has_status(OpenMesh::HalfedgeHandle) const { return Mesh::has_halfedge_status(); }
	size_t py_has_status(OpenMesh::EdgeHandle) const { return Mesh::has_edge_status(); }
	size_t py_has_status(OpenMesh::FaceHandle) const { return Mesh::has_face_status(); }

private:

	template <class Handle>
	void py_deepcopy_prop(MeshWrapperT *_copy, py::object _copyfunc, py::dict _memo) {
        using PropHandle = HandleToPropHandle<Handle>;
        const auto enditer = this->end(Handle());
        for (auto iter = this->begin(Handle()); iter != enditer; ++iter){

            PropHandle prop;
            if (!this->get_property_handle(prop, (*iter)->name())) 
                continue; // no python prop, skip it
            PropHandle copyProp = _copy->py_prop_on_demand<Handle, PropHandle>((*iter)->name());

			for (size_t i = 0; i < py_n_items(Handle()); ++i) {
				const Handle h(i);
				_copy->property(copyProp, h) = _copyfunc(this->property(prop, h), _memo);
			}
		}
	}

	template <class Handle, class PropHandle>
	PropHandle py_prop_on_demand(const std::string& _name) {
        PropHandle result;
        if (!Mesh::get_property_handle(result, _name))
            Mesh::add_property(result, _name);
		return result;
	}

    typename Mesh::prop_iterator begin(OpenMesh::VertexHandle) { return this->vprops_begin(); }
    typename Mesh::prop_iterator end(OpenMesh::VertexHandle) { return this->vprops_end(); }
    typename Mesh::prop_iterator begin(OpenMesh::HalfedgeHandle) { return this->hprops_begin(); }
    typename Mesh::prop_iterator end(OpenMesh::HalfedgeHandle) { return this->hprops_end(); }
    typename Mesh::prop_iterator begin(OpenMesh::EdgeHandle) { return this->eprops_begin(); }
    typename Mesh::prop_iterator end(OpenMesh::EdgeHandle) { return this->eprops_end(); }
    typename Mesh::prop_iterator begin(OpenMesh::FaceHandle) { return this->fprops_begin(); }
    typename Mesh::prop_iterator end(OpenMesh::FaceHandle) { return this->fprops_end(); }

    template<typename Handle, typename Dummy>
    struct HandleToPropHandle_D
    {};

    template<typename Dummy>
    struct HandleToPropHandle_D<OpenMesh::VertexHandle, Dummy>
    {
        using type = VPropHandle;
    };
    template<typename Dummy>
    struct HandleToPropHandle_D<OpenMesh::HalfedgeHandle, Dummy>
    {
        using type = HPropHandle;
    };
    template<typename Dummy>
    struct HandleToPropHandle_D<OpenMesh::EdgeHandle, Dummy>
    {
        using type = EPropHandle;
    };
    template<typename Dummy>
    struct HandleToPropHandle_D<OpenMesh::FaceHandle, Dummy>
    {
        using type = FPropHandle;
    };
    template<typename Handle>
    using HandleToPropHandle = typename HandleToPropHandle_D<Handle,Handle>::type;

};


typedef MeshWrapperT<OpenMesh::TriMesh_ArrayKernelT<MeshTraits> > TriMesh;
typedef MeshWrapperT<OpenMesh::PolyMesh_ArrayKernelT<MeshTraits> > PolyMesh;

#endif
