#pragma once

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#endif

extern PyObject* (*openmesh_pyinit_function)(void);
