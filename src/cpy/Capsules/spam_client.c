//
// Created by Paul Ross on 13/07/2024.
//
// Implements: https://docs.python.org/3/extending/extending.html#extending-simpleexample
// but using a capsule exported by spam_capsule.h/.c
// Excludes specific exception.
// Lightly edited.

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "spam_capsule.h"

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

static struct PyModuleDef spam_clientmodule = {
        PyModuleDef_HEAD_INIT,
        "spam_client",   /* name of module */
        PyDoc_STR("Documentation for the spam module"), /* module documentation, may be NULL */
        -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
        SpamMethods,
        NULL, NULL, NULL, NULL,
};

PyMODINIT_FUNC
PyInit_spam_client(void) {
    PyObject *m;

    m = PyModule_Create(&spam_clientmodule);
    if (m == NULL)
        return NULL;
    if (import_spam_capsule() < 0)
        return NULL;
    /* additional initialization can happen here */
    return m;
}
