#include "OpenFlipper.hh"
#include "PyModules/PyMeshType.hh"
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
#include <OpenFlipper/common/DataTypes.hh>
#include <ACG/Utils/SmartPointer.hh>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <OpenFlipper/BasePlugin/RPCWrappers.hh>
#include <OpenFlipper/common/UpdateType.hh>

#include <algorithm>
#include <functional>
#include <unordered_map>

namespace py = pybind11;

py::object rpc_call(const char* plugin_name, const char* function_name, const py::list& py_params/*{ QString:"value",... }*/);

/// Helpers

// Script call helper, from python to rpc (parameter managment)
class OFScriptCaller
{
    QString module_name_;
    QString function_name_;
    QVector<QStringList> params_vec_;

public:
    OFScriptCaller() {}
    OFScriptCaller(QString mn, QString fn, QStringList p):
        module_name_{std::move(mn)},
        function_name_{std::move(fn)},
        params_vec_{1, std::move(p)}
    {}

    void add_overload(QStringList p)
    {
        params_vec_.push_back(std::move(p));
    }

    py::object operator()(py::args args) const
    {
        py::list py_params;
        const auto argsize = args.size();
        auto it = std::find_if(params_vec_.cbegin(), params_vec_.cend(), 
            [argsize](const QStringList& p)
        {
            return static_cast<size_t>(p.size()) == argsize;
        });
        if (it == params_vec_.cend())
        {
            QString error_msg = QString("%1.%2 does not accept %3 arguments.").arg(module_name_, function_name_, QString::number(args.size()));
            PyErr_SetString(PyExc_RuntimeError, (const char*)error_msg.toLatin1());
            throw py::error_already_set();
        }

        const QStringList& params = *it;
        for (int i = 0; i < params.size(); ++i)
        {
            py_params.append((const char*)params[i].toLatin1());
            py_params.append(args[i]);
        }
        return rpc_call((const char*)module_name_.toLatin1(), (const char*)function_name_.toLatin1(), std::move(py_params));
    }
};

// conversions
namespace pybind11 {
    namespace detail {

#define DEFINE_VECTOR_TYPE_CASTER(T, N)\
        template<>\
        struct type_caster<ACG::VectorT<T, N> >\
        {\
        public:\
            using VecT = ACG::VectorT<T, N>;\
\
            PYBIND11_TYPE_CASTER(VecT, _("ACG::VectorT<"#T", "#N">"));\
            /*py -> c++*/\
            bool load(py::handle src, bool convert)\
            {\
                if (!convert && !py::array_t<double>::check_(src))\
                    return false;\
                auto buf = py::array_t<double, py::array::c_style | py::array::forcecast>::ensure(src);\
                if (!buf)\
                    return false;\
                if (buf.ndim() != 1 || buf.size() != N)\
                    return false;\
                value = VecT(buf.data());\
                return true;\
            }\
           /* c++ -> py*/\
            static py::handle cast(const VecT& src, py::return_value_policy policy, py::handle parent)\
            {\
                py::array a(N, src.data());\
                return a.release();\
            }\
        };
    // END DEFINE_VECTOR_TYPE_CASTER

    DEFINE_VECTOR_TYPE_CASTER(double, 3)
    DEFINE_VECTOR_TYPE_CASTER(double, 4)
#undef DEFINE_VECTOR_TYPE_CASTER

    }
} // namespace py::detail

// parameter types defined for openflipper scrippting, which are supported by python scripting
// currently, supports 714 functions out of 802 defined in Free branch (~90%)
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

QHash<QString, QHash<QString, OFScriptCaller>> g_submodule_collector;


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
////////////////////////////////////////////////////////////////////////////////////////
// first clumsy try to convert from qscriptvalue to some custom type
// qscriptvalue_cast does not return, whether the type is correct or not (could cast Vector to IdList)
// + add some conversion functionality to python (i.e. numpy for matrix, use pybind11/stl for IdList)
// current solution: check the property names which is slow
template <typename T>
std::tuple<bool, py::object> extract_from_qscript(QScriptValue& v)
{
    //const int id  = qMetaTypeId< T >();
    //T r;

    //// todo: check if type is correct


    //if (qscriptvalue_cast_helper(v, id, &r))
    //    return std::make_tuple<bool, py::object>(true, py::array_t<double>(r.size(), r.data()) );
    //else
    //{
    //    bool b = v.isVariant();
    //    if (b)
    //    {
    //        b = v.toVariant().canConvert<T>();
    //        if (b)
    //        {
    //            r = qvariant_cast<T>(v.toVariant());
    //            return std::make_tuple(b, py::cast(r));
    //        }
    //    }
    //}

    return std::make_tuple(false, py::none());
}

template <>
std::tuple<bool, py::object> extract_from_qscript<Vector>(QScriptValue& v)
{
    const int id = qMetaTypeId< Vector >();

    // todo: check if type is correct
    // i.e. check propertytype
    QScriptValueIterator it(v);
    QVector<QString> prop_check = { "x","y","z" };
    QVectorIterator<QString> it_check(prop_check);

    bool prop_valid = true;
    while (it.hasNext() && it_check.hasNext() && prop_valid)
    {
        it.next();
        prop_valid = it.name() == it_check.next();
    }
    prop_valid = it.hasNext() == it_check.hasNext();

    Vector r;
    if (prop_valid && qscriptvalue_cast_helper(v, id, &r))
        return std::make_tuple<bool, py::object>(true, py::cast(r));

    return std::make_tuple(false, py::none());
}

template <>
std::tuple<bool, py::object> extract_from_qscript<Vector4>(QScriptValue& v)
{
    const int id = qMetaTypeId< Vector4 >();

    // todo: check if type is correct
    // i.e. check propertytype
    QScriptValueIterator it(v);
    QVector<QString> prop_check = { "x","y","z","w" }; //is w correct?
    QVectorIterator<QString> it_check(prop_check);

    bool prop_valid = true;
    while (it.hasNext() && it_check.hasNext() && prop_valid)
    {
        it.next();
        prop_valid = it.name() == it_check.next();
    }

    prop_valid = it.hasNext() == it_check.hasNext();

    Vector r;
    if (qscriptvalue_cast_helper(v, id, &r))
        return std::make_tuple<bool, py::object>(true, py::cast(r));

    return std::make_tuple(false, py::none());
}
////////////////////////////////////////////////////////////////////////////////////////

py::object rpc_call(const char* plugin_name, const char* function_name, std::vector<QScriptValue> params)
{
    QScriptValue ret = RPC::callFunction(plugin_name, function_name, std::move(params));
    if (ret.isNumber())
        return py::cast(ret.toInt32());
    if (ret.isString())
        return py::cast(ret.toString().toStdString());
    if (ret.isBool())
        return py::cast(ret.toBool());

    // insert here: qobjects (if supported by python engine)

    bool is_type;
    py::object result;

    std::tie(is_type, result) = extract_from_qscript<Vector>(ret);
    if (is_type) return result;
    //std::tie(is_type, result) = extract_from_qscript<Vector4>(ret);
    //if (is_type) return result; //todo: check the property names
    //std::tie(is_type, result) = extract_from_qscript<Matrix4x4>(ret);
    //if (is_type) return result; //todo: property names & how to create a matrix
    //std::tie(is_type, result) = extract_from_qscript<UpdateType>(ret);
    //if (is_type) return result; //todo: test

    if (!ret.isNull())
    {
        //warning?
    }

    //cannot cast to pytype -> return none (no error)
    return py::none();
}

//example: ofp.rpc_call("core", "deleteObject", ["int", cube_id])
py::object rpc_call(const char* plugin_name, const char* function_name, const py::list& py_params/*{ QString:"value",... }*/)
{
    std::vector<QScriptValue> q_params;
    QScriptEngine* const engine = RPC::getScriptEngine();
    // convert parameters to scriptvalues
    for (size_t i = 0; i < py::len(py_params); i += 2)
    {
        std::string type_name = py::cast<std::string>(py_params[i]);

        auto it = type_conversion_map.find(type_name);
        if (it == std::end(type_conversion_map)) //todo: raise exception
        {
            QString err = QString("In function %1, parameter #%2 is can not be converted to %3.").arg(QString(function_name), QString::number(i / 2), QString(type_name.c_str()));
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

    // add openflipper scripting functions
    for (auto sm_it = g_submodule_collector.cbegin(); sm_it != g_submodule_collector.cend(); ++sm_it)
    {
        auto sm = m.def_submodule((const char*)sm_it.key().toLatin1());

        const  auto& sm_fns = sm_it.value();
        for (auto it = sm_fns.cbegin(); it != sm_fns.cend(); ++it)
        {
            sm.def((const char*)it.key().toLatin1(), it.value());
        }
    }
    g_submodule_collector.clear();
    decltype(g_submodule_collector)().swap(g_submodule_collector);//shrink to fit, avaiable first in Qt 5.10

}

PyObject*(*openflipper_get_init_function(const QStringList& ofs_functions))(void)
{
    QRegularExpression re = get_supported_function_regex();

    for (const auto& function : qAsConst(ofs_functions))
    {
        auto match = re.match(function);
        if (!match.hasMatch())
            continue;
        // 1: pluginname
        // 2: functionname
        // 3+: params
        QString pn = match.captured(1);
        if (pn == "pymesh" || pn == "-")
            continue;
        QString fn = match.captured(2);
        QStringList params;
        auto captured_groups = match.lastCapturedIndex();
        auto captured_txt = match.capturedTexts();
        if (match.captured(3) != "")
            params.append(match.captured(3));
        if (match.captured(4) != "")
        {
            QStringList p = match.captured(4).split(",", QString::SkipEmptyParts);
            for (auto& str : p)
                str = str.trimmed();
            params.append(p);
        }

        auto it = g_submodule_collector[pn].find(fn);
        if (it == g_submodule_collector[pn].end())
            g_submodule_collector[pn][fn] = OFScriptCaller{ pn, fn, params };
        else
            g_submodule_collector[pn][fn].add_overload(params);
    }


#if (PY_MAJOR_VERSION == 2)
    return &initopenflipper;//untested
#else
    return &PyInit_openflipper;
#endif
}

QRegularExpression get_supported_function_regex()
{
    const QString supportedTypes = QString::fromStdString(
        std::accumulate(std::next(type_conversion_map.begin()), type_conversion_map.end(), type_conversion_map.begin()->first, []
        (const std::string& lhs, const decltype(*type_conversion_map.begin())& rhs)
    {return std::string(lhs) + std::string("|") + std::string(rhs.first); }));// results in "int|uint|double|..."
                                                                              
    QRegularExpression re{ QString("(\\w+).(\\w+)\\((%1)?((,(%1))*)\\)").arg(supportedTypes) };

    return re;
}
