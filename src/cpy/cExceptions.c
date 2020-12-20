//
//  cExcep.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 08/05/2014.
//  Copyright (c) 2014 Paul Ross. All rights reserved.
//

#include "Python.h"

static PyObject *_raise_error(PyObject *module) {
    
    PyErr_SetString(PyExc_ValueError, "Ooops.");
    assert(PyErr_Occurred());
	return NULL;
}

static PyObject *_raise_error_formatted(PyObject *module) {
    PyErr_Format(PyExc_ValueError,
                 "Can not read %d bytes when offset %d in byte length %d.", \
                 12, 25, 32
                 );
    assert(PyErr_Occurred());
	return NULL;
}

/* Illustrate returning NULL but not setting an exception. */
static PyObject *_raise_error_bad(PyObject *module) {
    PyErr_Clear();
    assert(! PyErr_Occurred());
	return NULL;
}

/* Set and exception but fail to signal by returning non-NULL. */
static PyObject *_raise_error_mixup(PyObject *module) {
    PyErr_SetString(PyExc_ValueError, "ERROR: _raise_error_mixup()");
    assert(PyErr_Occurred());
	Py_RETURN_NONE;
}

/* Test and exception, possibly set by another function. */
static PyObject *_raise_error_mixup_test(PyObject *module) {
    if (PyErr_Occurred()) {
        return NULL;
    }
	Py_RETURN_NONE;
}

/* Shows that second PyErr_SetString() is ignored. */
static PyObject *_raise_error_overwrite(PyObject *module) {
    PyErr_SetString(PyExc_RuntimeError, "FORGOTTEN.");
    PyErr_SetString(PyExc_ValueError, "ERROR: _raise_error_overwrite()");
    assert(PyErr_Occurred());
	return NULL;
}

/* Specialise exceptions. */
static PyObject *ExceptionBase;
static PyObject *SpecialisedError;


static PyMethodDef cExceptions_methods[] = {
    {"raiseErr", (PyCFunction)_raise_error, METH_NOARGS,
		"Raise a simple exception."
    },
    {"raiseErrFmt", (PyCFunction)_raise_error_formatted, METH_NOARGS,
		"Raise a formatted exception."
    },
    {"raiseErrBad", (PyCFunction)_raise_error_bad, METH_NOARGS,
		"Signal an exception by returning NULL but fail to set an exception."
    },
    {"raiseErrMix", (PyCFunction)_raise_error_mixup, METH_NOARGS,
		"Set an exception but fail to signal it but returning non-NULL."
    },
    {"raiseErrTst", (PyCFunction)_raise_error_mixup_test, METH_NOARGS,
		"Raise if an exception is set otherwise returns None."
    },
    {"raiseErrOver", (PyCFunction)_raise_error_overwrite, METH_NOARGS,
		"Example of overwriting exceptions, a RuntimeError is set then a ValueError. Only the latter is seen."
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
PyInit_cExceptions(void)
{
    PyObject* m = PyModule_Create(&cExceptions_module);
    if (m == NULL) {
        return NULL;
    }
    /* Initialise exceptions here.
     *
     * Firstly a base class exception that inherits from the builtin Exception.
     * This is acheieved by passing NULL as the PyObject* as the third argument.
     *
     * PyErr_NewExceptionWithDoc returns a new reference.
     */
    ExceptionBase = PyErr_NewExceptionWithDoc(
                                              "cExceptions.ExceptionBase", /* char *name */
                                              "Base exception class for the noddy module.", /* char *doc */
                                              NULL, /* PyObject *base */
                                              NULL /* PyObject *dict */);
    /* Error checking: this is oversimplified as it should decref
     * anything created above such as m.
     */
    if (! ExceptionBase) {
        return NULL;
    } else {
        PyModule_AddObject(m, "ExceptionBase", ExceptionBase);
    }
    /* Now a sub-class exception that inherits from the base exception above.
     * This is acheieved by passing non-NULL as the PyObject* as the third argument.
     *
     * PyErr_NewExceptionWithDoc returns a new reference.
     */
    SpecialisedError = PyErr_NewExceptionWithDoc(
                                                 "cExceptions.SpecialsiedError", /* char *name */
                                                 "Some specialised problem description here.", /* char *doc */
                                                 ExceptionBase, /* PyObject *base */
                                                 NULL /* PyObject *dict */);
    if (! SpecialisedError) {
        return NULL;
    } else {
        PyModule_AddObject(m, "SpecialisedError", SpecialisedError);
    }
    /* END: Initialise exceptions here. */
    return m;
}
