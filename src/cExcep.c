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

/* Set and exception but fail to signal by returning NULL. */
static PyObject *_raise_error_mixup(PyObject *module) {
    
    PyErr_SetString(PyExc_ValueError, "ERROR: _raise_error_mixup()");
    assert(PyErr_Occurred());
	Py_RETURN_NONE;
}

/* Test and exception, possibly set by another function. */
static PyObject *_raise_error_mixup_test(PyObject *module) {
    
//    PyErr_SetString(PyExc_ValueError, "ERROR: _raise_error_mixup_test()");
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

static PyMethodDef cExcep_methods[] = {
    {"raiseErr", (PyCFunction)_raise_error, METH_NOARGS,
		"Raise a simple exception."
    },
    {"raiseErrFmt", (PyCFunction)_raise_error_formatted, METH_NOARGS,
		"Raise a formatted exception."
    },
    {"raiseErrBad", (PyCFunction)_raise_error_bad, METH_NOARGS,
		"Signal an exception but fail to set an exception."
    },
    {"raiseErrMix", (PyCFunction)_raise_error_mixup, METH_NOARGS,
		"Set an exception but fail to signal it."
    },
    {"raiseErrTst", (PyCFunction)_raise_error_mixup_test, METH_NOARGS,
		"Test for an exception."
    },
    {"raiseErrOver", (PyCFunction)_raise_error_overwrite, METH_NOARGS,
		"Test for an exception."
    },
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyModuleDef cExcep_module = {
    PyModuleDef_HEAD_INIT,
    "cExcep",
    "Examples of raising exceptions.",
    -1,
	cExcep_methods,
	NULL, /* inquiry m_reload */
	NULL, /* traverseproc m_traverse */
	NULL, /* inquiry m_clear */
	NULL, /* freefunc m_free */
};

PyMODINIT_FUNC
PyInit_cExcep(void)
{
    return PyModule_Create(&cExcep_module);
}
/* END: Setting error conditions. */
