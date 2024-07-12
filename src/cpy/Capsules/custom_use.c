//
// Created by Paul Ross on 19/06/2021.
//
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "custom_capsule.h"


//PyMODINIT_FUNC
//PyInit_customuse(void)
//{
//    PyObject *m;
//
//    m = PyModule_Create(&clientmodule);
//    if (m == NULL)
//        return NULL;
//    if (import_custom() < 0)
//        return NULL;
//    /* additional initialization can happen here */
//    return m;
//}
