#include "OpenFlipper.hh"
#include "PyModules/PyMeshType.hh"
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include <ACG/Utils/SmartPointer.hh>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

#include <OpenFlipper/BasePlugin/RPCWrappers.hh>
#include <OpenFlipper/common/UpdateType.hh>

#include <algorithm>
#include <functional>
#include <unordered_map>

namespace py = pybind11;

const std::unordered_map < std::string, std::function<QScriptValue(QScriptEngine* e, const py::object&) >> type_conversion_map =
{
    { "QString",[](QScriptEngine*const e, const py::object& obj) {return e->toScriptValue(QString(py::cast<std::string>(obj).c_str())); } },
        { "int",[](QScriptEngine*const e, const py::object& obj) {return e->toScriptValue(py::cast<int>(obj)); } },
        { "uint" ,[](QScriptEngine*const e, const py::object& obj) {return e->toScriptValue(py::cast<uint>(obj)); } },
        { "bool" ,[](QScriptEngine*const e, const py::object& obj) {return e->toScriptValue(py::cast<bool>(obj)); } },
        { "IdList" ,[](QScriptEngine*const e, const py::object& obj) {return e->toScriptValue(py::cast<IdList>(obj)); } },
        { "Vector" ,[](QScriptEngine*const e, const py::object& obj) {return e->toScriptValue(py::cast<Vector>(obj)); } },
        { "Vector4" ,[](QScriptEngine*const e, const py::object& obj) {return e->toScriptValue(py::cast<Vector4>(obj)); } },
        { "UpdateType" ,[](QScriptEngine*const e, const py::object& obj) {return e->toScriptValue(py::cast<UpdateType>(obj)); } },
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
            PyTriMesh*const m = reinterpret_cast<PyTriMesh*>(triobj->mesh());
            d[triobj->name().toLatin1()] = py::cast(m);
        }
        else if (PluginFunctions::getObject(i, polyobj))
        {
            PyPolyMesh*const m = reinterpret_cast<PyPolyMesh*>(polyobj->mesh());
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
    if (ret.isString())
        return py::cast(ret.toString().toStdString());
    if (ret.isBool())
        return py::cast(ret.toBool());

    //cannot cast to pytype -> return none (no error)
    return py::none();
}

py::object rpc_call(const char* plugin_name, const char* function_name, const py::list& py_params/*{ QString:"value",... }*/)
{
    std::vector<QScriptValue> q_params;
    QScriptEngine* const engine = RPC::getScriptEngine();
    // convert parameters to scriptvalues
    for (py::ssize_t i = 0; i < py::len(py_params); i += 2)
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

        q_params.push_back(cast_f(engine, py_params[i + 1]));
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
    {
        PyErr_SetString(PyExc_RuntimeError, "Cannot find given mesh. Arguments correct?");
        throw py::error_already_set();
    }
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

    py::class_<UpdateType> update_type(m, "Update");
    update_type
        .def(py::init<>())
        .def(py::init<UpdateType>())
        .def(py::self | py::self)
        .def(py::self |= py::self)
        .def("__contains__", &UpdateType::contains, py::is_operator())
        .def_property_readonly_static("NONE", [](py::object) {return UPDATE_NONE;})
        .def_property_readonly_static("ALL", [](py::object) {return UPDATE_ALL;})
        .def_property_readonly_static("VISIBILITY", [](py::object) {return UPDATE_VISIBILITY;})
        .def_property_readonly_static("GEOMETRY", [](py::object) {return UPDATE_GEOMETRY;})
        .def_property_readonly_static("TOPOLOGY", [](py::object) {return UPDATE_TOPOLOGY;})
        .def_property_readonly_static("SELECTION", [](py::object) {return UPDATE_SELECTION;})
        .def_property_readonly_static("SELECTION_VERTICES", [](py::object) {return UPDATE_SELECTION_VERTICES;})
        .def_property_readonly_static("SELECTION_EDGES", [](py::object) {return UPDATE_SELECTION_EDGES;})
        .def_property_readonly_static("SELECTION_HALFEDGES", [](py::object) {return UPDATE_SELECTION_HALFEDGES;})
        .def_property_readonly_static("SELECTION_FACES", [](py::object) {return UPDATE_SELECTION_FACES;})
        .def_property_readonly_static("SELECTION_KNOTS", [](py::object) {return UPDATE_SELECTION_KNOTS;})
        .def_property_readonly_static("COLOR", [](py::object) {return UPDATE_COLOR;})
        .def_property_readonly_static("TEXTURE", [](py::object) {return UPDATE_TEXTURE;})
        .def_property_readonly_static("STATE", [](py::object) {return UPDATE_STATE;})
        .def_property_readonly_static("UNUSED", [](py::object) {return UPDATE_UNUSED;;});
}

#if (PY_MAJOR_VERSION == 2)
    PyObject* (*openflipper_pyinit_function)(void) = &initopenflipper;//untested
#else
    PyObject* (*openflipper_pyinit_function)(void) = &PyInit_openflipper;
#endif
