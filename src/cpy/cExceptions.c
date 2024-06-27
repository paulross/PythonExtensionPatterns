//
//  cExceptions.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 08/05/2014.
//  Copyright (c) 2014-2024 Paul Ross. All rights reserved.
//

#include "Python.h"

/** Raise a simple exception. */
static PyObject *raise_error(PyObject *Py_UNUSED(module)) {
    PyErr_SetString(PyExc_ValueError, "Ooops.");
    assert(PyErr_Occurred());
    return NULL;
}

/** Raise an exception with a formatted message. */
static PyObject *raise_error_formatted(PyObject *Py_UNUSED(module)) {
    PyErr_Format(PyExc_ValueError,
                 "Can not read %d bytes when offset %d in byte length %d.", \
                 12, 25, 32
    );
    assert(PyErr_Occurred());
    return NULL;
}

/** This illustrates the consequences of returning NULL but not setting an exception. */
static PyObject *raise_error_bad(PyObject *Py_UNUSED(module)) {
    PyErr_Clear();
    assert(!PyErr_Occurred());
    return NULL;
}

/** Set an exception but fail to signal by returning non-NULL. */
static PyObject *raise_error_silent(PyObject *Py_UNUSED(module)) {
    PyErr_SetString(PyExc_ValueError, "ERROR: raise_error_silent()");
    assert(PyErr_Occurred());
    Py_RETURN_NONE;
}

/** Test for an exception, possibly set by another function. */
static PyObject *raise_error_silent_test(PyObject *Py_UNUSED(module)) {
    if (PyErr_Occurred()) {
        return NULL;
    }
    Py_RETURN_NONE;
}

/** Shows that second PyErr_SetString() is ignored. */
static PyObject *raise_error_overwrite(PyObject *Py_UNUSED(module)) {
    PyErr_SetString(PyExc_RuntimeError, "FORGOTTEN.");
    PyErr_SetString(PyExc_ValueError, "ERROR: raise_error_overwrite()");
    assert(PyErr_Occurred());
    return NULL;
}

/** Specialise exceptions base exception. */
static PyObject *ExceptionBase = 0;
/** Specialise exceptions derived from base exception. */
static PyObject *SpecialisedError = 0;


/** Raises a ExceptionBase. */
static PyObject *raise_exception_base(PyObject *Py_UNUSED(module)) {
    if (ExceptionBase) {
        PyErr_Format(ExceptionBase, "One %d two %d three %d.", 1, 2, 3);
    } else {
        PyErr_SetString(PyExc_RuntimeError, "Can not raise exception, module not initialised correctly");
    }
    return NULL;
}

/** Raises a SpecialisedError. */
static PyObject *raise_specialised_error(PyObject *Py_UNUSED(module)) {
    if (SpecialisedError) {
        PyErr_Format(SpecialisedError, "One %d two %d three %d.", 1, 2, 3);
    } else {
        PyErr_SetString(PyExc_RuntimeError, "Can not raise exception, module not initialised correctly");
    }
    return NULL;
}

static PyMethodDef cExceptions_methods[] = {
        {"raise_error",             (PyCFunction) raise_error,             METH_NOARGS,
                                                                                        "Raise a simple exception."
        },
        {"raise_error_fmt",         (PyCFunction) raise_error_formatted,   METH_NOARGS,
                                                                                        "Raise a formatted exception."
        },
        {"raise_error_bad",         (PyCFunction) raise_error_bad,         METH_NOARGS,
                                                                                        "Signal an exception by returning NULL but fail to set an exception."
        },
        {"raise_error_silent",      (PyCFunction) raise_error_silent,      METH_NOARGS,
                                                                                        "Set an exception but fail to signal it but returning non-NULL."
        },
        {"raise_error_silent_test", (PyCFunction) raise_error_silent_test, METH_NOARGS,
                                                                                        "Raise if an exception is set otherwise returns None."
        },
        {
            "raise_error_overwrite",   (PyCFunction) raise_error_overwrite,   METH_NOARGS,
                                                                                        "Example of overwriting exceptions, a RuntimeError is set, then a ValueError. Only the latter is seen."
        },
        {
         "raise_exception_base",    (PyCFunction) raise_exception_base,    METH_NOARGS, "Raises a ExceptionBase."
        },
        {
         "raise_specialised_error", (PyCFunction) raise_specialised_error, METH_NOARGS, "Raises a SpecialisedError."
        },
        {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyModuleDef cExceptions_module = {
        PyModuleDef_HEAD_INIT,
        "cExceptions",
        "Examples of raising exceptions.",
        -1,
        cExceptions_methods,
        NULL, /* inquiry m_reload */
        NULL, /* traverseproc m_traverse */
        NULL, /* inquiry m_clear */
        NULL, /* freefunc m_free */
};

PyMODINIT_FUNC
PyInit_cExceptions(void) {
    PyObject *m = PyModule_Create(&cExceptions_module);
    if (m == NULL) {
        return NULL;
    }
    /* Initialise exceptions here.
     *
     * Firstly a base class exception that inherits from the builtin Exception.
     * This is achieved by passing NULL as the PyObject* as the third argument.
     *
     * PyErr_NewExceptionWithDoc returns a new reference.
     */
    ExceptionBase = PyErr_NewExceptionWithDoc(
            "cExceptions.ExceptionBase", /* char *name */
            "Base exception class for the noddy module.", /* char *doc */
            NULL, /* PyObject *base, resolves to PyExc_Exception. */
            NULL /* PyObject *dict */);
    /* Error checking: this is oversimplified as it should decref
     * anything created above such as m.
     */
    if (!ExceptionBase) {
        return NULL;
    } else {
        PyModule_AddObject(m, "ExceptionBase", ExceptionBase);
    }
    /* Now a subclass exception that inherits from the base exception above.
     * This is achieved by passing non-NULL as the PyObject* as the third argument.
     *
     * PyErr_NewExceptionWithDoc returns a new reference.
     */
    SpecialisedError = PyErr_NewExceptionWithDoc(
            "cExceptions.SpecialsiedError", /* char *name */
            "Some specialised problem description here.", /* char *doc */
            ExceptionBase, /* PyObject *base, declared above. */
            NULL /* PyObject *dict */);
    if (!SpecialisedError) {
        return NULL;
    } else {
        PyModule_AddObject(m, "SpecialisedError", SpecialisedError);
    }
    /* END: Initialise exceptions here. */
    return m;
}
