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
 * dec_ref it (free it).
 *
 * This is all about programming by contract and each of reference types
 * has a different contract.
 */

/** ==== Manipulating reference counts directly ==== */

/** Returns the reference count of a PyObject. */
static PyObject *ref_count(PyObject *Py_UNUSED(module), PyObject *pObj) {
    assert(pObj);
    return Py_BuildValue("n", pObj->ob_refcnt);
}

/** Increments the reference count of a PyObject.
 * Returns the original reference count. */
static PyObject *inc_ref(PyObject *Py_UNUSED(module), PyObject *pObj) {
    assert(pObj);
    Py_ssize_t ret = pObj->ob_refcnt;
    Py_INCREF(pObj);
    return Py_BuildValue("n", ret);
}

/** Decrements the reference count of a PyObject.
 * Returns the original reference count.
 * CAUTION: This may deallocate the object.
 * */
static PyObject *dec_ref(PyObject *Py_UNUSED(module), PyObject *pObj) {
    assert(pObj);
    Py_ssize_t ret = pObj->ob_refcnt;
    Py_DECREF(pObj);
    return Py_BuildValue("n", ret);
}

/** ==== END: Manipulating reference counts directly ==== */


/** ==== Example code for documentation. Not all of these are tested as they may segfault. ==== */

/** New reference.
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
    Py_DECREF(pA);                  /* My responsibility to dec_ref. */
    Py_DECREF(pB);                  /* My responsibility to dec_ref. */
    return r;                       /* Callers responsibility to dec_ref. */
}

static PyObject *subtract_two_longs(PyObject *Py_UNUSED(module)) {
    return subtract_long(421, 17);
}

/** Create an object, dec-ref it and then try and print it.
 * This may or may not work.
 * Not pytest tested.
 * */
static PyObject *access_after_free(PyObject *Py_UNUSED(module)) {
    PyObject *pA = PyLong_FromLong(1024L);
    Py_DECREF(pA);
    PyObject_Print(pA, stdout, 0);
    Py_RETURN_NONE;
}


/** Stolen reference.
 * This is object creation but where another object takes responsibility
 * for dec_ref'ing (freeing) the object.
 * These are quite rare; typical examples are object insertion into tuples
 * lists, dicts etc.
 *
 * The analogy with C would be malloc'ing some memory, populating it and
 * inserting that pointer into a linked list where the linked list promises
 * to free the memory when that item in the list is removed.
 */
static PyObject *make_tuple(PyObject *Py_UNUSED(module)) {
    PyObject *r;
    PyObject *v;

    r = PyTuple_New(3);         /* New reference. */
//    fprintf(stdout, "Ref count new: %zd\n", r->ob_refcnt);
    v = PyLong_FromLong(1L);    /* New reference. */
    /* PyTuple_SetItem steals the new reference v. */
    PyTuple_SetItem(r, 0, v);
    /* This is fine. */
    v = PyLong_FromLong(2L);
    PyTuple_SetItem(r, 1, v);
    /* More common pattern. */
    PyTuple_SetItem(r, 2, PyUnicode_FromString("three"));
    return r; /* Callers responsibility to dec_ref. */
}

/** Calls PySequence_DelItem() on each item.
 * This decrements the reference count by one for each item.
 */
void delete_all_list_items(PyObject *pList) {
    while (PyList_Size(pList) > 0) {
        PySequence_DelItem(pList, PyList_Size(pList) - 1);
    }
}

/** 'Borrowed' reference this is when reading from an object, you get back a
 * reference to something that the object still owns _and_ the container
 * can dispose of at _any_ time.
 * The problem is that you might want that reference for longer.
 *
 * Not pytest tested.
 */
static PyObject *pop_and_print_BAD(PyObject *Py_UNUSED(module), PyObject *pList) {
    PyObject *pLast;

    pLast = PyList_GetItem(pList, PyList_Size(pList) - 1);
    fprintf(stdout, "Ref count was: %zd\n", pLast->ob_refcnt);
    /* ... stuff here ... */
    delete_all_list_items(pList);
    /* ... more stuff here ... */
    fprintf(stdout, "Ref count now: %zd\n", pLast->ob_refcnt);
    PyObject_Print(pLast, stdout, 0); /* Boom. */
    fprintf(stdout, "\n");
    Py_RETURN_NONE;
}

/** The safer way, increment a borrowed reference.
 *
 * Not pytest tested.
 * */
static PyObject *pop_and_print_OK(PyObject *Py_UNUSED(module), PyObject *pList) {
    PyObject *pLast;

    pLast = PyList_GetItem(pList, PyList_Size(pList) - 1);
    fprintf(stdout, "Ref count was: %zd\n", pLast->ob_refcnt);
    Py_INCREF(pLast); /* This is the crucial change: increment a borrowed reference. */
    fprintf(stdout, "Ref count now: %zd\n", pLast->ob_refcnt);
    /* ... stuff here ... */
    delete_all_list_items(pList);
    /* ... more stuff here ... */
    PyObject_Print(pLast, stdout, 0);
    fprintf(stdout, "\n");
    Py_DECREF(pLast);
    fprintf(stdout, "Ref count fin: %zd\n", pLast->ob_refcnt);

    Py_RETURN_NONE;
}

/**
 * This leaks new references by creating count number of longs of given value but
 * never dec-refing them.
 *
 * Not pytest tested.
 */
static PyObject *leak_new_reference(PyObject *Py_UNUSED(module),
                                    PyObject *args, PyObject *kwargs) {
    PyObject *ret = NULL;
    int value, count;
    static char *kwlist[] = {"value", "count", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii", kwlist, &value,
                                     &count)) {
        goto except;
    }
    fprintf(stdout, "loose_new_reference: value=%d count=%d\n", value, count);
    for (int i = 0; i < count; ++i) {
        PyLong_FromLong(value);    /* New reference, leaked. */
    }

    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
except:
    Py_XDECREF(ret);
    ret = NULL;
finally:
    fprintf(stdout, "loose_new_reference: DONE\n");
    return ret;
}

/** ==== END: Example code for documentation. Not all of these are tested as they may segfault. ==== */

static PyMethodDef cPyRefs_methods[] = {
        {
            "ref_count",   (PyCFunction) ref_count,          METH_O,
            "Return the reference count a PyObject."
        },
        {
            "inc_ref",     (PyCFunction) inc_ref,            METH_O,
            "Increment the reference count a PyObject. Returns the original reference count"
        },
        {
            "dec_ref",     (PyCFunction) dec_ref,            METH_O,
            "Increment the reference count a PyObject. Returns the original reference count"
        },
        {
            "subtract_two_longs",      (PyCFunction) subtract_two_longs, METH_NOARGS,
            "Returns a new long by subtracting two longs in Python."
        },
        {
            "access_after_free",   (PyCFunction) access_after_free, METH_NOARGS,
            "Example of access after decrement reference."
        },
        {
            "make_tuple",    (PyCFunction) make_tuple,         METH_NOARGS,
            "Creates a tuple by stealing new references."
        },
        {
            "pop_and_print_BAD",      (PyCFunction) pop_and_print_BAD,  METH_O,
            "Borrowed refs, might segfault."
        },
        {
            "pop_and_print_OK",       (PyCFunction) pop_and_print_OK,   METH_O,
            "Borrowed refs, should not segfault."
        },
        {
            "leak_new_reference", (PyCFunction) leak_new_reference, METH_VARARGS | METH_KEYWORDS,
            "Leaks new references to longs."
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
PyInit_cPyRefs(void) {
    return PyModule_Create(&cPyRefs_module);
}
