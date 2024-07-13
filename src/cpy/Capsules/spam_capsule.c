//
// Created by Paul Ross on 13/07/2024.
//
// Implements: https://docs.python.org/3/extending/extending.html#extending-simpleexample
// as a capsule: https://docs.python.org/3/extending/extending.html#providing-a-c-api-for-an-extension-module
// Includes specific exception.
// Lightly edited.

#define PY_SSIZE_T_CLEAN

#include <Python.h>

#define SPAM_CAPSULE

#include "spam_capsule.h"

static int
PySpam_System(const char *command) {
    return system(command);
}

static PyObject *
spam_system(PyObject *Py_UNUSED(self), PyObject *args) {
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = PySpam_System(command);
    return PyLong_FromLong(sts);
}

static PyMethodDef SpamMethods[] = {
        /* ... */
        {"system", spam_system, METH_VARARGS,
                "Execute a shell command."},
        /* ... */
        {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef spammodule = {
        PyModuleDef_HEAD_INIT,
        "spam_capsule",   /* name of module */
        PyDoc_STR("Documentation for the spam module"), /* module documentation, may be NULL */
        -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
        SpamMethods,
        NULL, NULL, NULL, NULL,
};

PyMODINIT_FUNC
PyInit_spam_capsule(void) {
    PyObject *m;
    static void *PySpam_API[PySpam_API_pointers];
    PyObject *c_api_object;

    m = PyModule_Create(&spammodule);
    if (m == NULL)
        return NULL;

    /* Initialize the C API pointer array */
    PySpam_API[PySpam_System_NUM] = (void *) PySpam_System;

    /* Create a Capsule containing the API pointer array's address */
    c_api_object = PyCapsule_New((void *) PySpam_API, "cPyExtPatt.Capsules.spam_capsule._C_API", NULL);

    if (PyModule_AddObject(m, "_C_API", c_api_object) < 0) {
        Py_XDECREF(c_api_object);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
