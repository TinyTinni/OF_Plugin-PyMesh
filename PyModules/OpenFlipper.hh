#pragma once

#include <pybind11/pytypes.h>
#include <QStringList>
#include <QRegularExpression>

extern PyObject* (*openflipper_pyinit_function)(void);

PyObject*(*openflipper_get_init_function(const QStringList& ofs_functions))(void);

// regex for scripting functions. "plugin.function(param1,param2...)"
// no match, if param type is not supported by python scripting
QRegularExpression get_supported_function_regex();
