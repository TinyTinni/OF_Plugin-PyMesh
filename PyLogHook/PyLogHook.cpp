#include "PyLogHook.h"

#include <boost/python.hpp>

#include <cassert>

#include <iostream>

// todo: remove dependency
#include "../PyMeshPlugin.hh"

void* data = 0;

void pyLog(const char* log)
{
    assert(data);
    reinterpret_cast<PyMeshPlugin*>(data)->pyOutput(log);
}

void pyErr(const char* log)
{
    assert(data);
    reinterpret_cast<PyMeshPlugin*>(data)->pyError(log);
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

PyObject* stdErrCPP(PyObject* self, PyObject* args)
{
    // parse parameters
    const char* str;
    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    // callback
    pyErr(str);

    // return nothing
    Py_INCREF(Py_None);
    return Py_None;
}

void stdOutPutCPP2(const char* w)
{
    pyLog(w);
}

void stdErrCPP2(const char* w)
{
    pyErr(w);
}


void initPyLogger(PyObject* module, void* customData)
{
    assert(Py_IsInitialized());
    data = customData;

    const char* register_stdOutErrClass = "\
import sys\n\
class OF_CatchOutput:\n\
    def write(self, txt):\n\
        stdOutPutCPP(txt)\n\
    def flush(self):\n\
        pass\n\
class OF_CatchError:\n\
    def write(self, txt):\n\
        stdErrCPP(txt)\n\
    def flush(self):\n\
        pass\n\
sys.stdout = OF_CatchOutput()\n\
sys.stderr = OF_CatchError()\n\
";

   
   
    boost::python::object main_module(boost::python::handle<>(boost::python::borrowed(module)));
    boost::python::object main_namespace = main_module.attr("__dict__");
    main_namespace["stdOutPutCPP"] = stdOutPutCPP2;
    main_namespace["stdErrCPP"] = stdErrCPP2;

    //PyMethodDef pmd[] = { 
    //    {"stdOutPutCPP",stdOutPutCPP,METH_VARARGS, ""},
    //    {"stdErrCPP",stdErrCPP,METH_VARARGS, ""},
    //    {NULL,NULL,0,NULL} };

    PyRun_SimpleString(register_stdOutErrClass);

    PyErr_Print();
}


