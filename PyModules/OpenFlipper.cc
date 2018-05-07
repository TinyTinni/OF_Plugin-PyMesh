//#include "OpenFlipper.hh"
//#include <OpenMesh/src/Python/Bindings.hh>
//#include <boost/python.hpp>
//#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
//#include <ObjectTypes/PolyMesh/PolyMesh.hh>
//#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>
//#include <ACG/Utils/SmartPointer.hh>
//
//#include <OpenFlipper/BasePlugin/RPCWrappers.hh>
//
//namespace py = boost::python;
//
//py::dict create_dict_from_ids(const std::vector<int>& ids)
//{
//    py::dict d;
//    for (int i : ids)
//    {
//        TriMeshObject* triobj;
//        PolyMeshObject* polyobj;
//        if (PluginFunctions::getObject(i, triobj))
//        {
//            typedef OpenMesh::Python::TriMesh PyMesh;
//            PyMesh* m = reinterpret_cast<PyMesh*>(triobj->mesh());
//            d[triobj->name().toStdString()] = m;//ptr::shared_ptr<PyMesh>(m, [](PyMesh*){});
//        }
//        else if (PluginFunctions::getObject(i, polyobj))
//        {
//            typedef OpenMesh::Python::PolyMesh PyMesh;
//            PyMesh* m = reinterpret_cast<PyMesh*>(polyobj->mesh());
//            d[polyobj->name().toStdString()] = m;//ptr::shared_ptr<PyMesh>(m, [](PyMesh*){});
//        }
//
//    }
//    return d;
//}
//
//py::dict targets()
//{
//    std::vector<int> ids;
//    PluginFunctions::getTargetIdentifiers(ids);
//    return create_dict_from_ids(ids);
//}
//
//py::dict sources()
//{
//    std::vector<int> ids;
//    PluginFunctions::getSourceIdentifiers(ids);
//    return create_dict_from_ids(ids);
//}
//
//py::dict meshes()
//{
//    std::vector<int> ids;
//    PluginFunctions::getAllMeshes(ids);
//    return create_dict_from_ids(ids);
//}
//
//PyObject* rpc_call(const char* plugin_name, const char* function_name, std::vector<QScriptValue> params)
//{
//    QScriptValue ret = RPC::callFunction(plugin_name, function_name, std::move(params));
//    if (ret.isNumber())
//        return PyLong_FromLong(ret.toInt32());
//
//    // in case of no return type could be evaluated
//    Py_INCREF(Py_None);
//    return Py_None;
//}
//
//PyObject* rpc_call(const char* plugin_name, const char* function_name, const boost::python::list& py_params/*{ QString:"value",... }*/)
//{
//    std::vector<QScriptValue> q_params;
//    // convert parameters to scriptvalues
//
//    QScriptEngine* engine = RPC::getScriptEngine();
//    for (py::ssize_t i = 0; i < py::len(py_params); i+=2)
//    {
//        std::string type_name = py::extract<std::string>(py_params[i])();
//        //todo: error checking
//
//        QScriptValue script_value;
//        if (type_name == "QString")
//            script_value = engine->toScriptValue(QString(py::extract<std::string>(py_params[i + 1])().c_str()));
//        else if (type_name == "int")
//            script_value = engine->toScriptValue(py::extract<int>(py_params[i + 1])());
//        else if (type_name == "uint")
//            script_value = engine->toScriptValue(py::extract<unsigned int>(py_params[i + 1])());
//        else if (type_name == "bool")
//            script_value = engine->toScriptValue(py::extract<bool>(py_params[i + 1])());
//        else if (type_name == "IdList")
//            script_value = engine->toScriptValue(py::extract<IdList>(py_params[i + 1])());
//        else if (type_name == "Vector")
//            script_value = engine->toScriptValue(py::extract<Vector>(py_params[i + 1])());
//        else if (type_name == "Vector4")
//            script_value = engine->toScriptValue(py::extract<Vector4>(py_params[i + 1])());
//
//        q_params.push_back(std::move(script_value));
//    }
//    return rpc_call(plugin_name, function_name, std::move(q_params));
//}
//
//PyObject* rpc_call(const char* plugin_name, const char* function_name)
//{
//    std::vector<QScriptValue> params;
//    return rpc_call(plugin_name, function_name, std::move(params));
//}
//
//template<typename T>
//struct NoDeleter
//{
//    void operator()(T*) {}
//};
//
//py::object getMesh(int id)
//{
//    using PythonMesh = OpenMesh::Python::PolyMesh;
//    using Mesh = PolyMesh;
//    Mesh* polymesh = nullptr;
//    if (PluginFunctions::getMesh(id, polymesh))
//        return py::object(std::shared_ptr<PythonMesh>(reinterpret_cast<PythonMesh*>(polymesh), NoDeleter<PythonMesh>()));
//
//    using PythonTriMesh = OpenMesh::Python::TriMesh;
//    using TriMesh = TriMesh;
//    TriMesh* trimesh = nullptr;
//    if (PluginFunctions::getMesh(id, trimesh))
//        return py::object(std::shared_ptr<PythonTriMesh>(reinterpret_cast<PythonTriMesh*>(trimesh), NoDeleter<PythonTriMesh>()));
//
//    // error case
//    PyErr_SetString(PyExc_ValueError, "Passed Id is not a PolyMesh");
//    Py_INCREF(Py_None); // incref for the python interpreter, when the error is handled
//    return py::object(py::handle<>(py::borrowed(Py_None))); //borrowed for the decrement in py::object destructor
//}
//
//
//BOOST_PYTHON_MODULE(openflipper)
//{
//    py::def("targets", &targets);
//    py::def("sources", &sources);
//    py::def("meshes", &meshes);
//
//    py::def("get_mesh", &getMesh);
//
//    PyObject* (*rpc_callArgs)(const char*, const char*, const boost::python::list&) = rpc_call;
//    PyObject* (*rpc_callNoArgs)(const char* , const char* ) = rpc_call;
//    py::def("rpc_call", rpc_callArgs);
//    py::def("rpc_call", rpc_callNoArgs);
//
//    py::register_ptr_to_python< ptr::shared_ptr<OpenMesh::Python::TriMesh> >();
//    py::register_ptr_to_python< ptr::shared_ptr<OpenMesh::Python::PolyMesh> >();
//
//}
//
//namespace boost
//{
//    template <>
//    OpenMesh::Python::PolyMesh const volatile * get_pointer<OpenMesh::Python::PolyMesh const volatile >(
//        OpenMesh::Python::PolyMesh const volatile *c)
//    {
//        return c;
//    }
//    //template <>
//    //OpenMesh::Python::TriMesh const volatile *  get_pointer<OpenMesh::Python::TriMesh const volatile >(
//    //    OpenMesh::Python::TriMesh const volatile *c)
//    //{
//    //    return c;
//    //}
//
//}
//
//#if (PY_MAJOR_VERSION == 2)
//    PyObject* (*openflipper_pyinit_function)(void) = &initopenflipper;//untested
//#elif
//    PyObject* (*openflipper_pyinit_function)(void) = &PyInit_openflipper;
//#endif

#include "OpenFlipper.hh"

PyObject* (*openflipper_pyinit_function)(void) = 0;
