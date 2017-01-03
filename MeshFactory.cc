#include "MeshFactory.hh"

#include "PyMeshPlugin.hh"

#include <ACG/Utils/SmartPointer.hh>

PyMeshPlugin* g_plugin = 0;

template<typename T>
struct NoDeleter
{
    void operator()(T*){}
};


ptr::shared_ptr<OpenMesh::Python::TriMesh> createTriMesh()
{
    typedef OpenMesh::Python::TriMesh Mesh;
    Mesh* mesh = g_plugin->createTriMesh();
    return std::shared_ptr<Mesh>(mesh, NoDeleter<Mesh>());
}


ptr::shared_ptr<OpenMesh::Python::PolyMesh> createPolyMesh()
{
    typedef OpenMesh::Python::PolyMesh Mesh;
    Mesh* mesh = g_plugin->createPolyMesh();
    return std::shared_ptr<Mesh>(mesh, NoDeleter<Mesh>());
}

void registerFactoryMethods(PyMeshPlugin* plugin, boost::python::object& om_module)
{
    g_plugin = plugin;

    boost::python::object om_namespace = om_module.attr("__dict__");

    boost::python::object trimesh_m = om_module.attr("TriMesh");
    trimesh_m.attr("__init__") = boost::python::make_constructor(createTriMesh);

    boost::python::object polymesh_m = om_module.attr("PolyMesh");
    polymesh_m.attr("__init__") = boost::python::make_constructor(createPolyMesh);

    
}

