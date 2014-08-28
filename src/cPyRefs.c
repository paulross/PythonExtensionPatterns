//
//  PyReferences.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 07/05/2014.
//  Copyright (c) 2014 Paul Ross. All rights reserved.
//
#include "Python.h"

//#include <stdio.h>

/*
 * 'New', 'stolen' and 'borrowed' references.
 * These terms are used throughout the Python documentation, they refer to
 * who is the real owner of the reference i.e. whose job it is to finally
 * decref it (free it).
 *
 * This is all about programming by contract and each of reference types
 * has a different contract.
 */

/* New reference.
 * This is object creation and it is your job to dispose of it.
 *
 * The analogy with 'C' is the reference has been malloc'd and must be free'd
 * by you.
 */
static PyObject *subtract_long(long a, long b) {
    PyObject *pA, *pB, *r;
    
    pA = PyLong_FromLong(a);        /* pA: New reference. */
    pB = PyLong_FromLong(b);        /* pB: New reference. */
    r = PyNumber_Subtract(pA, pB);  /* r: New reference. */
    Py_DECREF(pA);                  /* My responsibility to decref. */
    Py_DECREF(pB);                  /* My responsibility to decref. */
    return r;                       /* Callers responsibility to decref. */
}

static PyObject *subtract_two_longs(PyObject *pModule) {
    return subtract_long(421, 17);
}

/* Stolen reference.
 * This is object creation but where another object takes responsibility
 * for decref'ing (freeing) the object.
 * These are quite rare; typical examples are object insertion into tuples
 * lists, dicts etc.
 *
 * The analogy with C would be malloc'ing some memory, populating it and
 * inserting that pointer into a linked list where the linked list promises
 * to free the memory when that item in the list is removed.
 */
static PyObject *make_tuple(PyObject *pModule) {
    PyObject *r;
    PyObject *v;
    
    r = PyTuple_New(3);         /* New reference. */
    fprintf(stdout, "Ref count new: %zd\n", r->ob_refcnt);
    v = PyLong_FromLong(1L);    /* New reference. */
    /* PyTuple_SetItem steals the new reference v. */
    PyTuple_SetItem(r, 0, v);
    /* This is fine. */
    v = PyLong_FromLong(2L);
    PyTuple_SetItem(r, 1, v);
    /* More common pattern. */
    PyTuple_SetItem(r, 2, PyUnicode_FromString("three"));
    return r; /* Callers responsibility to decref. */
}

void handle_list(PyObject *pList) {
    while (PyList_Size(pList) > 0) {
        PySequence_DelItem(pList, PyList_Size(pList) - 1);
    }
}

/* 'Borrowed' reference this is when reading from an object, you get back a
 * reference to something that the object still owns _and_ the container
 * can dispose of at _any_ time.
 * The problem is that you might want that reference for longer.
 */
static PyObject *pop_and_print_BAD(PyObject *pModule, PyObject *pList) {
    PyObject *pLast;
    
    pLast = PyList_GetItem(pList, PyList_Size(pList) - 1);
    fprintf(stdout, "Ref count was: %zd\n", pLast->ob_refcnt);
    /* ... stuff here ... */
    handle_list(pList);
    /* ... more stuff here ... */
    fprintf(stdout, "Ref count now: %zd\n", pLast->ob_refcnt);
    PyObject_Print(pLast, stdout, 0); /* Boom. */
    fprintf(stdout, "\n");
    Py_RETURN_NONE;
}

static PyObject *pop_and_print_OK(PyObject *pModule, PyObject *pList) {
    PyObject *pLast;
    
    pLast = PyList_GetItem(pList, PyList_Size(pList) - 1);
    fprintf(stdout, "Ref count was: %zd\n", pLast->ob_refcnt);
    Py_INCREF(pLast);
    fprintf(stdout, "Ref count now: %zd\n", pLast->ob_refcnt);
    /* ... stuff here ... */
    handle_list(pList);
    /* ... more stuff here ... */
    PyObject_Print(pLast, stdout, 0);
    fprintf(stdout, "\n");
    Py_DECREF(pLast);
    fprintf(stdout, "Ref count fin: %zd\n", pLast->ob_refcnt);

    Py_RETURN_NONE;
}

static PyObject *bad_incref(PyObject *pModule, PyObject *pObj) {
    fprintf(stdout, "Ref count was: %zd\n", pObj->ob_refcnt);
    Py_INCREF(pObj);
    fprintf(stdout, "Ref count now: %zd\n", pObj->ob_refcnt);
    if (1) {
        Py_RETURN_NONE;
    }
    Py_RETURN_NONE;
}

static PyMethodDef cPyRefs_methods[] = {
    {"newRef", (PyCFunction)subtract_two_longs, METH_NOARGS,
		"Returns a new long by subtracting two longs in Python."
    },
    {"stealRef", (PyCFunction)make_tuple, METH_NOARGS,
		"Creates a tuple by stealing new references."
    },
    {"popBAD", (PyCFunction)pop_and_print_BAD, METH_O,
		"Borrowed refs, might segfault."
    },
    {"popOK", (PyCFunction)pop_and_print_OK, METH_O,
		"Borrowed refs, should not segfault."
    },
    {"incref", (PyCFunction)bad_incref, METH_O,
		"Naughty incref."
    },
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyModuleDef cPyRefs_module = {
    PyModuleDef_HEAD_INIT,
    "cPyRefs",
    "Examples of reference types in a 'C' extension.",
    -1,
	cPyRefs_methods,
	NULL, /* inquiry m_reload */
	NULL, /* traverseproc m_traverse */
	NULL, /* inquiry m_clear */
	NULL, /* freefunc m_free */
};

PyMODINIT_FUNC
PyInit_cPyRefs(void)
{
    return PyModule_Create(&cPyRefs_module);
}
