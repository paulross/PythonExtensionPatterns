//
//  cppsublist.cpp
//  Subclassing a Python list.
//
//  Created by Paul Ross on 22/07/2024.
//  Copyright (c) 2024 Paul Ross. All rights reserved.
//
// Based on: https://docs.python.org/3/extending/newtypes_tutorial.html#subclassing-other-types
//
// This is very like src/cpy/SubClass/sublist.c but it includes a slow max() method
// to illustrate thread contention.
// So it needs a thread lock.

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "structmember.h"

#include "py_call_super.h"
#include "cThreadLock.h"
#include <time.h>


typedef struct {
    PyListObject list;
#ifdef WITH_THREAD
    PyThread_type_lock lock;
#endif
} SubListObject;

static int
SubList_init(SubListObject *self, PyObject *args, PyObject *kwds) {
    if (PyList_Type.tp_init((PyObject *) self, args, kwds) < 0) {
        return -1;
    }
#ifdef WITH_THREAD
    self->lock = PyThread_allocate_lock();
    if (self->lock == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Unable to allocate thread lock.");
        return -2;
    }
#endif
    return 0;
}

static void
SubList_dealloc(SubListObject *self) {
    /* Deallocate other fields here. */
#ifdef WITH_THREAD
    if (self->lock) {
        PyThread_free_lock(self->lock);
        self->lock = NULL;
    }
#endif
    Py_TYPE(self)->tp_free((PyObject *)self);
}

void sleep_milliseconds(long ms) {
    struct timespec tim_request, tim_remain;
    tim_request.tv_sec = 0;
    tim_request.tv_nsec = ms * 1000L * 1000L;
    nanosleep(&tim_request, &tim_remain);
}

/** append with a thread lock. */
static PyObject *
SubList_append(SubListObject *self, PyObject *args) {
    AcquireLock<SubListObject> local_lock((SubListObject *)self);
    PyObject *result = call_super_name(
            (PyObject *) self, "append", args, NULL
    );
    // 0.25s delay to demonstrate holding on to the thread.
    sleep_milliseconds(250L);
    return result;
}

/** This is a deliberately laborious find of the maximum value to
 * demonstrate protection against thread contention.
 */
static PyObject *
SubList_max(PyObject *self, PyObject *Py_UNUSED(unused)) {
    assert(!PyErr_Occurred());
    AcquireLock<SubListObject> local_lock((SubListObject *)self);
    PyObject *ret = NULL;
    // SubListObject
    size_t length = PyList_Size(self);
    if (length == 0) {
        // Raise
        PyErr_SetString(PyExc_ValueError, "max() on empty list.");
    } else {
        // Return first
        ret = PyList_GetItem(self, 0);
        if (length > 1) {
            // laborious compare
            PyObject *item = NULL;
            for(Py_ssize_t i = 1; i <PyList_Size(self); ++i) {
                item = PyList_GetItem(self, i);
                int result = PyObject_RichCompareBool(item, ret, Py_GT);
                if (result < 0) {
                    // Error, not comparable.
                    ret = NULL;
                } else if (result > 0) {
                    ret = item;
                }
                // 2ms delay to demonstrate holding on to the thread.
                sleep_milliseconds(2L);
            }
        }
        Py_INCREF(ret);
    }
//    // 0.25s delay to demonstrate holding on to the thread.
//    sleep_milliseconds(250L);
    return ret;
}

static PyMethodDef SubList_methods[] = {
        {"append",    (PyCFunction) SubList_append,    METH_VARARGS,
                        PyDoc_STR("append an item with sleep(1).")},
        {"max",       (PyCFunction) SubList_max,       METH_NOARGS,
                        PyDoc_STR("Return the maximum value with sleep(1).")},
        {NULL, NULL, 0, NULL},
};

static PyMemberDef SubList_members[] = {
        {NULL, 0, 0, 0, NULL}  /* Sentinel */
};

static PyTypeObject cppSubListType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "cppsublist.cppSubList",
        .tp_basicsize = sizeof(SubListObject),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR("C++ SubList object"),
        .tp_methods = SubList_methods,
        .tp_members = SubList_members,
        .tp_init = (initproc) SubList_init,
        .tp_dealloc = (destructor) SubList_dealloc,
};

static PyModuleDef cppsublistmodule = {
        PyModuleDef_HEAD_INIT,
        .m_name = "cppsublist",
        .m_doc = "Example module that creates an extension type.",
        .m_size = -1,
};

PyMODINIT_FUNC
PyInit_cppsublist(void) {
    PyObject * m;
    cppSubListType.tp_base = &PyList_Type;
    if (PyType_Ready(&cppSubListType) < 0) {
        return NULL;
    }
    m = PyModule_Create(&cppsublistmodule);
    if (m == NULL) {
        return NULL;
    }
    Py_INCREF(&cppSubListType);
    if (PyModule_AddObject(m, "cppSubList", (PyObject *) &cppSubListType) < 0) {
        Py_DECREF(&cppSubListType);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
