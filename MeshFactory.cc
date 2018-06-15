#include "MeshFactory.hh"
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include <memory>
#include <utility>

#include "PyModules/PyMeshType.hh"

static std::function<void* ()> requestTriMesh = nullptr;
static std::function<void* ()> requestPolyMesh = nullptr;


namespace py = pybind11;

template<typename T>
T* createMesh() {static_assert(!std::is_same<T,T>::value,"No overloaded Function defined.");return nullptr; }

template<>
PyTriMesh* createMesh<PyTriMesh>()
{
    return reinterpret_cast<PyTriMesh*>(requestTriMesh());
}
template<>
PyPolyMesh* createMesh<PyPolyMesh>()
{
    return reinterpret_cast<PyPolyMesh*>(requestPolyMesh());
}

template<typename Mesh>
Mesh* createMesh_numpy(py::array_t<typename Mesh::Point::value_type> _points, py::array_t<int> _faces)
{
    using Point = typename Mesh::Point;
    namespace OM = OpenMesh;
    Mesh* mesh = createMesh<Mesh>();

    Mesh::init_helper(*mesh, std::move(_points), std::move(_faces));

    return mesh;
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

template<typename Mesh, typename T, typename... Extra>
void set_constructor(py::object& obj, T f, Extra... extra)
{
    auto i = py::cpp_function(create_init_function<Mesh>(f), py::name("__init__"), py::is_method(obj),
        py::sibling(py::getattr(obj, "__init__", py::none())), 
        py::detail::is_new_style_constructor{}, py::return_value_policy::reference, std::forward<Extra>(extra)...);
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

    set_constructor<PyTriMesh>(trimesh_m, &createMesh<PyTriMesh>);
    set_constructor<PyTriMesh>(trimesh_m, &createMesh_numpy<PyTriMesh>, py::arg("points"), py::arg("face_vertex_indices") = py::array_t<int>());
    //set_constructor<TriMesh,int>(trimesh_m, &createTriMesh);

    py::object polymesh_m = _om_module.attr("PolyMesh");
    py::setattr(polymesh_m, "__init__", py::none());//erase all constructors (espacially the overloads overloaded)

    set_constructor<PyPolyMesh>(polymesh_m, &createMesh<PyPolyMesh>);
    set_constructor<PyPolyMesh>(polymesh_m, &createMesh_numpy<PyPolyMesh>, py::arg("points"), py::arg("face_vertex_indices") = py::array_t<int>());


}

