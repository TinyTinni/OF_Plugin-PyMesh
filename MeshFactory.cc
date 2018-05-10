#include "MeshFactory.hh"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include <memory>
#include <utility>

#include "PyModules/PyMeshType.hh"

static std::function<void* ()> requestTriMesh = nullptr;
static std::function<void* ()> requestPolyMesh = nullptr;


namespace py = pybind11;

PyTriMesh* createTriMesh()
{
    return reinterpret_cast<PyTriMesh*>(requestTriMesh());
}

PyPolyMesh* createPolyMesh()
{
	return reinterpret_cast<PyPolyMesh*>(requestPolyMesh());
}

//////////////////////////////////////////
// Helper function for __init__
template<typename Mesh, typename... Args>
std::function<void(py::detail::value_and_holder&, Args...)> create_init_function(Mesh*(*f)(Args...))
{
    return [f](py::detail::value_and_holder& v_h, Args... args)
    {
        py::detail::initimpl::construct<py::class_<Mesh,HolderType<Mesh>>>(v_h,
            f(std::forward<Args>(args)...), Py_TYPE(v_h.inst) != v_h.type->type);
    };
}

template<typename Mesh, typename... Args>
void set_constructor(py::object& obj, Mesh*(*f)(Args...))
{
    auto i = py::cpp_function(create_init_function<Mesh, Args...>(f), py::name("__init__"), py::is_method(obj),
        py::sibling(py::getattr(obj, "__init__", py::none())), 
        py::detail::is_new_style_constructor{}, py::return_value_policy::reference);
    py::setattr(obj, "__init__", std::move(i));

}

//////////////////////////////////////////

void registerFactoryMethods(pybind11::module& _om_module,
		std::function<void* ()> _create_trimesh,
		std::function<void* ()> _create_polymesh)
{
	requestTriMesh = _create_trimesh;
	requestPolyMesh = _create_polymesh;

	py::object om_namespace = _om_module.attr("__dict__");

	py::object trimesh_m = _om_module.attr("TriMesh");
	py::setattr(trimesh_m,"__init__",py::none());//erase all constructors (espacially the overloads overloaded)

	set_constructor<PyTriMesh>(trimesh_m, &createTriMesh);
	//set_constructor<TriMesh,int>(trimesh_m, &createTriMesh);

    py::object polymesh_m = _om_module.attr("PolyMesh");
    py::setattr(polymesh_m, "__init__", py::none());//erase all constructors (espacially the overloads overloaded)

    set_constructor<PyPolyMesh>(polymesh_m, &createPolyMesh);


}

