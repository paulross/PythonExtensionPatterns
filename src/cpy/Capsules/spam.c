//
// Created by Paul Ross on 13/07/2024.
//
// Implements: https://docs.python.org/3/extending/extending.html#extending-simpleexample
// Excludes specific exception.
// Lightly edited.

#define PY_SSIZE_T_CLEAN
#include <Python.h>

static PyObject *
spam_system(PyObject *Py_UNUSED(self), PyObject *args) {
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = system(command);
    return PyLong_FromLong(sts);
}

static PyMethodDef SpamMethods[] = {
        /* ... */
        {"system",  spam_system, METH_VARARGS,
                    "Execute a shell command."},
        /* ... */
        {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef spammodule = {
        PyModuleDef_HEAD_INIT,
        "spam",   /* name of module */
        PyDoc_STR("Documentation for the spam module"), /* module documentation, may be NULL */
        -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
        SpamMethods,
        NULL,
        NULL,
        NULL,
        NULL,
};

PyMODINIT_FUNC
PyInit_spam(void) {
    return PyModule_Create(&spammodule);
}
