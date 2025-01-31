//
// Created by Paul Ross on 30/01/2025.
//

#ifndef PYTHONEXTENSIONPATTERNS_PYEXTPATT_UTIL_H
#define PYTHONEXTENSIONPATTERNS_PYEXTPATT_UTIL_H

#define PPY_SSIZE_T_CLEAN

#include "Python.h"

PyObject *new_unique_string(const char *function_name, const char *suffix);

#endif //PYTHONEXTENSIONPATTERNS_PYEXTPATT_UTIL_H
