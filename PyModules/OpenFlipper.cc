#include "OpenFlipper.hh"
#include "PyModules/PyMeshType.hh"
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include <ACG/Utils/SmartPointer.hh>

#include <pybind11/pybind11.h>

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
            PyTriMesh* m = reinterpret_cast<PyTriMesh*>(triobj->mesh());
            d[triobj->name().toLatin1()] = py::cast(m);
        }
        else if (PluginFunctions::getObject(i, polyobj))
        {
            PyPolyMesh* m = reinterpret_cast<PyPolyMesh*>(polyobj->mesh());
            d[polyobj->name().toLatin1()] = py::cast(m);
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

py::object rpc_call(const char* plugin_name, const char* function_name, std::vector<QScriptValue> params)
{
    QScriptValue ret = RPC::callFunction(plugin_name, function_name, std::move(params));
    if (ret.isNumber())
        return py::cast(ret.toInt32());

    return py::none();
}

py::object rpc_call(const char* plugin_name, const char* function_name, const py::list& py_params/*{ QString:"value",... }*/)
{
    std::vector<QScriptValue> q_params;
    // convert parameters to scriptvalues

    QScriptEngine* engine = RPC::getScriptEngine();
    for (py::ssize_t i = 0; i < py::len(py_params); i+=2)
    {
        std::string type_name = py::cast<std::string>(py_params[i]);
        //todo: error checking

        QScriptValue script_value;
        if (type_name == "QString")
            script_value = engine->toScriptValue(QString(py::cast<std::string>(py_params[i + 1]).c_str()));
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

py::object rpc_call(const char* plugin_name, const char* function_name)
{
    std::vector<QScriptValue> params;
    return rpc_call(plugin_name, function_name, std::move(params));
}

py::object getMesh(int id)
{
    PolyMesh* polymesh = nullptr;
    if (PluginFunctions::getMesh(id, polymesh))
        return py::cast(reinterpret_cast<PyPolyMesh*>(polymesh));

    TriMesh* trimesh = nullptr;
    if (PluginFunctions::getMesh(id, trimesh))
        return py::cast(reinterpret_cast<PyTriMesh*>(trimesh));

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

    py::object (*rpc_callArgs)(const char*, const char*, const py::list&) = rpc_call;
    py::object (*rpc_callNoArgs)(const char* , const char* ) = rpc_call;
    m.def("rpc_call", rpc_callArgs);
    m.def("rpc_call", rpc_callNoArgs);
}


#if (PY_MAJOR_VERSION == 2)
    PyObject* (*openflipper_pyinit_function)(void) = &initopenflipper;//untested
#else
    PyObject* (*openflipper_pyinit_function)(void) = &PyInit_openflipper;
#endif
