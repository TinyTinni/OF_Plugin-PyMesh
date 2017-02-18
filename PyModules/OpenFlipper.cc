#include "OpenFlipper.hh"
#include <OpenMesh/src/Python/Bindings.hh>
#include <boost/python.hpp>
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include <ACG/Utils/SmartPointer.hh>

namespace py = boost::python;

py::dict create_dict_from_ids(const std::vector<int>& ids)
{
    py::dict d;
    for (int i : ids)
    {
        TriMeshObject* triobj;
        PolyMeshObject* polyobj;
        if (PluginFunctions::getObject(i, triobj))
        {
            typedef OpenMesh::Python::TriMesh PyMesh;
            PyMesh* m = reinterpret_cast<PyMesh*>(triobj->mesh());
            d[triobj->name().toStdString()] = m;//ptr::shared_ptr<PyMesh>(m, [](PyMesh*){});
        }
        else if (PluginFunctions::getObject(i, polyobj))
        {
            typedef OpenMesh::Python::PolyMesh PyMesh;
            PyMesh* m = reinterpret_cast<PyMesh*>(polyobj->mesh());
            d[polyobj->name().toStdString()] = m;//ptr::shared_ptr<PyMesh>(m, [](PyMesh*){});
        }

    }
    return d;
}

py::dict targets()
{
    std::vector<int> ids;
    PluginFunctions::getTargetIdentifiers(ids);
    return create_dict_from_ids(ids);
}

py::dict sources()
{
    std::vector<int> ids;
    PluginFunctions::getSourceIdentifiers(ids);
    return create_dict_from_ids(ids);
}

py::dict meshes()
{
    std::vector<int> ids;
    PluginFunctions::getAllMeshes(ids);
    return create_dict_from_ids(ids);
}


BOOST_PYTHON_MODULE(openflipper)
{
    py::def("targets", &targets);
    py::def("sources", &sources);
    py::def("meshes", &meshes);

    py::register_ptr_to_python< ptr::shared_ptr<OpenMesh::Python::TriMesh> >();
    py::register_ptr_to_python< ptr::shared_ptr<OpenMesh::Python::PolyMesh> >();

}

namespace boost
{
    template <>
    OpenMesh::Python::PolyMesh const volatile * get_pointer<OpenMesh::Python::PolyMesh const volatile >(
        OpenMesh::Python::PolyMesh const volatile *c)
    {
        return c;
    }
    //template <>
    //OpenMesh::Python::TriMesh const volatile *  get_pointer<OpenMesh::Python::TriMesh const volatile >(
    //    OpenMesh::Python::TriMesh const volatile *c)
    //{
    //    return c;
    //}

}

#if (PY_MAJOR_VERSION == 2)
    PyObject* (*openflipper_pyinit_function)(void) = &initopenflipper;//untested
#elif (PY_MAJOR_VERSION == 3)
    PyObject* (*openflipper_pyinit_function)(void) = &PyInit_openflipper;
#endif