#include "OpenFlipper.hh"
#include "OpenMesh-Python/src/MeshTypes.hh"
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include <ACG/Utils/SmartPointer.hh>

#include <OpenFlipper/BasePlugin/RPCWrappers.hh>

namespace py = pybind11;

py::dict create_dict_from_ids(const std::vector<int>& ids)
{
    py::dict d;
    for (int i : ids)
    {
        TriMeshObject* triobj;
        PolyMeshObject* polyobj;
        if (PluginFunctions::getObject(i, triobj))
        {
            typedef TriMesh PyMesh;
            PyMesh* m = reinterpret_cast<PyMesh*>(triobj->mesh());
            d[triobj->name().toStdString()] = m;//ptr::shared_ptr<PyMesh>(m, [](PyMesh*){});
        }
        else if (PluginFunctions::getObject(i, polyobj))
        {
            typedef PolyMesh PyMesh;
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

PyObject* rpc_call(const char* plugin_name, const char* function_name, std::vector<QScriptValue> params)
{
    QScriptValue ret = RPC::callFunction(plugin_name, function_name, std::move(params));
    if (ret.isNumber())
        return PyLong_FromLong(ret.toInt32());

    // in case of no return type could be evaluated
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject* rpc_call(const char* plugin_name, const char* function_name, const py::list& py_params/*{ QString:"value",... }*/)
{
    std::vector<QScriptValue> q_params;
    // convert parameters to scriptvalues

    QScriptEngine* engine = RPC::getScriptEngine();
    for (py::ssize_t i = 0; i < py::len(py_params); i+=2)
    {
        std::string type_name = py::cast<std::string>(py_params[i])();
        //todo: error checking

        QScriptValue script_value;
        if (type_name == "QString")
            script_value = engine->toScriptValue(QString(py::cast<std::string>(py_params[i + 1])));
        else if (type_name == "int")
            script_value = engine->toScriptValue(py::cast<int>(py_params[i + 1]));
        else if (type_name == "uint")
            script_value = engine->toScriptValue(py::cast<unsigned int>(py_params[i + 1]));
        else if (type_name == "bool")
            script_value = engine->toScriptValue(py::cast<bool>(py_params[i + 1]));
        else if (type_name == "IdList")
            script_value = engine->toScriptValue(py::cast<IdList>(py_params[i + 1]));
        else if (type_name == "Vector")
            script_value = engine->toScriptValue(py::cast<Vector>(py_params[i + 1]));
        else if (type_name == "Vector4")
            script_value = engine->toScriptValue(py::cast<Vector4>(py_params[i + 1]));

        q_params.push_back(std::move(script_value));
    }
    return rpc_call(plugin_name, function_name, std::move(q_params));
}

PyObject* rpc_call(const char* plugin_name, const char* function_name)
{
    std::vector<QScriptValue> params;
    return rpc_call(plugin_name, function_name, std::move(params));
}

template<typename T>
struct NoDeleter
{
    void operator()(T*) {}
};

py::object getMesh(int id)
{
    using PythonMesh = PolyMesh;
    using Mesh = PolyMesh;
    Mesh* polymesh = nullptr;
    if (PluginFunctions::getMesh(id, polymesh))
        return py::object(std::shared_ptr<PythonMesh>(reinterpret_cast<PythonMesh*>(polymesh), NoDeleter<PythonMesh>()));

    using PythonTriMesh = TriMesh;
    using TriMesh = TriMesh;
    TriMesh* trimesh = nullptr;
    if (PluginFunctions::getMesh(id, trimesh))
        return py::object(std::shared_ptr<PythonTriMesh>(reinterpret_cast<PythonTriMesh*>(trimesh), NoDeleter<PythonTriMesh>()));

    // error case
    PyErr_SetString(PyExc_ValueError, "Passed Id is not a PolyMesh");
    return py::none();
}


PYBIND11_MODULE(openflipper, m)
{
    m.def("targets", &targets);
    m.def("sources", &sources);
    m.def("meshes", &meshes);

    m.def("get_mesh", &getMesh);

    PyObject* (*rpc_callArgs)(const char*, const char*, const py::list&) = rpc_call;
    PyObject* (*rpc_callNoArgs)(const char* , const char* ) = rpc_call;
    m.def("rpc_call", rpc_callArgs);
    m.def("rpc_call", rpc_callNoArgs);

    //py::register_ptr_to_python< ptr::shared_ptr<OpenMesh::Python::TriMesh> >();
    //py::register_ptr_to_python< ptr::shared_ptr<OpenMesh::Python::PolyMesh> >();

}


#if (PY_MAJOR_VERSION == 2)
    PyObject* (*openflipper_pyinit_function)(void) = &initopenflipper;//untested
#elif
    PyObject* (*openflipper_pyinit_function)(void) = &PyInit_openflipper;
#endif
