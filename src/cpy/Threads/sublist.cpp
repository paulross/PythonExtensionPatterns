//
//  sublist.c
//  Subclassing a Python list.
//
//  Created by Paul Ross on 22/07/2024.
//  Copyright (c) 2024 Paul Ross. All rights reserved.
//
// Based on: https://docs.python.org/3/extending/newtypes_tutorial.html#subclassing-other-types
// That describes sub-classing a list.
// However as well as the increment function this counts how many times
// append() is called and uses the super() class to call the base class append.
//
// This is very like src/cpy/SubClass/sublist.c but it includes a slow max() method
// to illustrate thread contention.

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#include "py_call_super.h"
#include "cThreadLock.h"

typedef struct {
    PyListObject list;
    int state;
    int appends;
#ifdef WITH_THREAD
    PyThread_type_lock lock;
#endif
} SubListObject;

static int
SubList_init(SubListObject *self, PyObject *args, PyObject *kwds) {
    if (PyList_Type.tp_init((PyObject *) self, args, kwds) < 0) {
        return -1;
    }
    self->state = 0;
    self->appends = 0;
    return 0;
}

static PyObject *
SubList_increment(SubListObject *self, PyObject *Py_UNUSED(unused)) {
    self->state++;
    return PyLong_FromLong(self->state);
}

/** append with a thread lock. */
static PyObject *
SubList_append(SubListObject *self, PyObject *args) {
    AcquireLock<SubListObject> local_lock((SubListObject *)self);
    PyObject *result = call_super_name((PyObject *)self, "append",
                                       args, NULL);
    if (result) {
        self->appends++;
    }
    return result;
}

/** This is a deliberately laborious find of the maximum value to
 * demonstrate protection against thread contention.
 */
static PyObject *
SubList_max(PyObject *self, PyObject *Py_UNUSED(unused)) {
    AcquireLock<SubListObject> local_lock((SubListObject *)self);
    // SubListObject
    size_t length = PyList_Size(self);
    if (length == 0) {
        // Raise
    } else if (length == 1) {
        // Return first
    } else {
        // laborious compare
    }
    return NULL;
}

static PyMethodDef SubList_methods[] = {
        {"increment", (PyCFunction) SubList_increment, METH_NOARGS,
                PyDoc_STR("increment state counter")},
        {"append", (PyCFunction) SubList_append, METH_VARARGS,
                PyDoc_STR("append an item")},
        {"max", (PyCFunction) SubList_max, METH_NOARGS,
                PyDoc_STR("Return the maximum value.")},
        {NULL, NULL, 0, NULL},
};

static PyMemberDef SubList_members[] = {
        {"state", T_INT, offsetof(SubListObject, state), 0,
                "Value of the state."},
        {"appends", T_INT, offsetof(SubListObject, appends), 0,
                "Number of append operations."},
        {NULL, 0, 0, 0, NULL}  /* Sentinel */
};

static PyTypeObject SubListType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "sublist.SubList",
        .tp_doc = PyDoc_STR("SubList objects"),
        .tp_basicsize = sizeof(SubListObject),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_init = (initproc) SubList_init,
        .tp_methods = SubList_methods,
        .tp_members = SubList_members,
};

static PyModuleDef sublistmodule = {
        PyModuleDef_HEAD_INIT,
        .m_name = "sublist",
        .m_doc = "Example module that creates an extension type.",
        .m_size = -1,
};

PyMODINIT_FUNC
PyInit_sublist(void) {
    PyObject *m;
    SubListType.tp_base = &PyList_Type;
    if (PyType_Ready(&SubListType) < 0) {
        return NULL;
    }
    m = PyModule_Create(&sublistmodule);
    if (m == NULL) {
        return NULL;
    }
    Py_INCREF(&SubListType);
    if (PyModule_AddObject(m, "SubList", (PyObject *) &SubListType) < 0) {
        Py_DECREF(&SubListType);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
