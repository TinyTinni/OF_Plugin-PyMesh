#include "OpenFlipper.hh"
#include "PyModules/PyMeshType.hh"
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include <ACG/Utils/SmartPointer.hh>

#include <pybind11/pybind11.h>

#include <OpenFlipper/BasePlugin/RPCWrappers.hh>

#include <algorithm>
#include <functional>
#include <unordered_map>

namespace py = pybind11;

const std::unordered_map < std::string, std::function<QScriptValue(QScriptEngine* e, const py::object&) >> type_conversion_map =
{
    { "QString",[](QScriptEngine* e, const py::object& obj) {return e->toScriptValue(QString(py::cast<std::string>(obj).c_str())); } },
        { "int",[](QScriptEngine* e, const py::object& obj) {return e->toScriptValue(py::cast<int>(obj)); } },
        { "uint" ,[](QScriptEngine* e, const py::object& obj) {return e->toScriptValue(py::cast<uint>(obj)); } },
        { "bool" ,[](QScriptEngine* e, const py::object& obj) {return e->toScriptValue(py::cast<bool>(obj)); } },
        { "IdList" ,[](QScriptEngine* e, const py::object& obj) {return e->toScriptValue(py::cast<IdList>(obj)); } },
        { "Vector" ,[](QScriptEngine* e, const py::object& obj) {return e->toScriptValue(py::cast<Vector>(obj)); } },
        { "Vector4" ,[](QScriptEngine* e, const py::object& obj) {return e->toScriptValue(py::cast<Vector4>(obj)); } }
};


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

        auto it = type_conversion_map.find(type_name);
        if (it == std::end(type_conversion_map)) //todo: raise exception
        {
            const QString err = QString("In function %1, parameter #%2 is can not be converted to %3.").arg(QString(function_name), QString::number(i / 2), QString(type_name.c_str()));
            PyErr_SetString(PyExc_ValueError, err.toLatin1());
            return py::none();
        }
        auto cast_f = it->second;

        q_params.push_back(cast_f(engine, py_params[i+1]));
    }
    if (PyErr_Occurred())
    	return py::none();
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

template<typename Mesh>
py::object getID(void* addr)
{
    IdList idlist;
    PluginFunctions::getAllMeshes(idlist);
    auto iter = std::find_if(std::begin(idlist), std::end(idlist), [&addr](int id)
    {
        Mesh* mesh;
        if (!PluginFunctions::getMesh(id, mesh))
            return false;
        return ((void*)mesh == (void*)addr);
    });
    if (iter == std::end(idlist))
        return py::cast(-1); //todo raise exception
    return py::cast(int(*iter));
}


PYBIND11_MODULE(openflipper, m)
{
    m.def("targets", &targets);
    m.def("sources", &sources);
    m.def("meshes", &meshes);

    m.def("get_mesh", &getMesh);
    m.def("get_id", [](PyTriMesh* m) {return getID<TriMesh>(m); });
    m.def("get_id", [](PyPolyMesh* m) {return getID<PolyMesh>(m); });

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
