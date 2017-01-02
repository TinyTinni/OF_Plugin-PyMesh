#include "PyLogHook.h"

#include <boost/python.hpp>

#include <cassert>

#include <iostream>

// todo: remove dependency
#include "../PyMeshPlugin.hh"

void* data = 0;

void pyLog(const char* log)
{
    assert(data); // todo: remove when depedency is removed
    reinterpret_cast<PyMeshPlugin*>(data)->pyOutput(log);
}

PyObject* stdOutPutCPP(PyObject* self, PyObject* args)
{
    // parse parameters
    const char* str;
    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    // callback
    pyLog(str);

    // return nothing
    Py_INCREF(Py_None);
    return Py_None;
}

void stdOutPutCPP2(const char* w)
{
    pyLog(w);
}

void initPyLogger(PyObject* module, void* customData)
{
    assert(Py_IsInitialized());
    data = customData;

    const char* register_stdOutErrClass = "\
import sys\n\
class CatchOutErr:\n\
    def write(self, txt):\n\
        stdOutPutCPP(txt)\n\
    def flush(self):\n\
        pass\n\
sys.stdout = CatchOutErr()\n\
sys.stderr = CatchOutErr()\n\
";

   
   
    boost::python::object main_module(boost::python::handle<>(boost::python::borrowed(module)));
    boost::python::object main_namespace = main_module.attr("__dict__");
    main_namespace["stdOutPutCPP"] = stdOutPutCPP2;

    PyMethodDef pmd[] = { {"stdOutPutCPP",stdOutPutCPP,METH_VARARGS, ""},{NULL,NULL,0,NULL} };
    //PyModule_AddFunctions(module, pmd);
    //boost::python::exec("stdOutPutCPP(\"Hello non Hook\")", main_namespace);

    PyRun_SimpleString(register_stdOutErrClass);

    PyErr_Print();
}


