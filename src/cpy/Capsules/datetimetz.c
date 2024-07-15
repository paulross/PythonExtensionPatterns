//
// Created by Paul Ross on 13/07/2024.
//
// Implements a datatimetz subclass of datetime that always has a timezone.
// This is an example of using Capsules and the datetime Capsule API.

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "datetime.h"

typedef struct {
    PyDateTime_DateTime datetime;
} DateTimeTZ;

// Forward reference
//typedef struct DatetimeTZType DatetimeTZType;

static PyObject *
DateTimeTZ_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
//    DateTimeTZ* self;
//    self = (DateTimeTZ*)type->tp_alloc(type, 0);
//    if (self != NULL) {
//        self->number = 0;
//    }
//    return (PyObject*)self;

    fprintf(stdout, "DateTimeTZ_new() type:\n");
    PyObject_Print((PyObject *)type, stdout, Py_PRINT_RAW);
    fprintf(stdout, "\n");
    fprintf(stdout, "DateTimeTZ_new() args:\n");
    PyObject_Print(args, stdout, Py_PRINT_RAW);
    fprintf(stdout, "\n");
    fprintf(stdout, "DateTimeTZ_new() kwds:\n");
    PyObject_Print(kwds, stdout, Py_PRINT_RAW);
    fprintf(stdout, "\n");

    DateTimeTZ *self = (DateTimeTZ *)PyDateTimeAPI->DateTimeType->tp_new(type, args, kwds);
    if (self) {
        // Raise if no TZ.
        if (self->datetime.tzinfo == NULL || Py_IsNone(self->datetime.tzinfo)) {
            PyErr_SetString(PyExc_ValueError, "No time zone provided.");
            Py_DECREF(self);
            self = NULL;
        }
        fprintf(stdout, "DateTimeTZ_new() self:\n");
        PyObject_Print((PyObject *)self, stdout, Py_PRINT_RAW);
        fprintf(stdout, "\n");
        fprintf(stdout, "DateTimeTZ_new() self->datetime:\n");
        PyObject_Print((PyObject*)(&self->datetime), stdout, 0);
        fprintf(stdout, "\n");
        fprintf(stdout, "DateTimeTZ_new() self->datetime.tzinfo:\n");
        PyObject_Print((PyObject*)(&self->datetime.tzinfo), stdout, Py_PRINT_RAW);
        fprintf(stdout, "\n");
    }
    return (PyObject *)self;
//    if (self == NULL) {
//        return -1;
//    }
//    // Raise if no TZ.
//    if (self->datetime.tzinfo == NULL || Py_IsNone(self->datetime.tzinfo)) {
//        PyErr_SetString(PyExc_ValueError, "No time zone provided.");
//    }
//    return 0;
}

#if 0
static int
DateTimeTZ_init(DateTimeTZ *self, PyObject *args, PyObject *kwds)
{
    if (PyDateTimeAPI->DateTimeType->tp_init((PyObject *)self, args, kwds) < 0) {
        return -1;
    }
    // Raise if no TZ.
    if (self->datetime.tzinfo == NULL || Py_IsNone(self->datetime.tzinfo)) {
        PyErr_SetString(PyExc_ValueError, "No time zone provided.");
    }
    return 0;
}
#endif

static PyTypeObject DatetimeTZType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "datetimetz.datetimetz",
        .tp_doc = "A datetime that requires a time zone.",
        .tp_basicsize = sizeof(DateTimeTZ),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = DateTimeTZ_new,
//        .tp_init = (initproc) DateTimeTZ_init,
//        .tp_dealloc = (destructor) DateTimeTZ_dealloc,
//        .tp_members = DateTimeTZ_members,
//        .tp_methods = DateTimeTZ_methods,
//        .tp_getset = DateTimeTZ_getsetters,
};

static PyModuleDef datetimetzmodule = {
        PyModuleDef_HEAD_INIT,
        .m_name = "datetimetz",
        .m_doc = "Module that contains a datetimetz, a datetime.datetime with a mandatory time zone.",
        .m_size = -1,
};

PyMODINIT_FUNC
PyInit_datetimetz(void)
{
    PyObject *m;

    m = PyModule_Create(&datetimetzmodule);
    if (m == NULL)
        return NULL;
//    if (import_spam_capsule() < 0)
//        return NULL;
    // datetime.datetime_CAPI
    PyDateTime_IMPORT;
    if (!PyDateTimeAPI) {
        Py_DECREF(m);
        return NULL;
    }

    DatetimeTZType.tp_base = PyDateTimeAPI->DateTimeType;
    if (PyType_Ready(&DatetimeTZType) < 0) {
        return NULL;
    }

    Py_INCREF(&DatetimeTZType);
    PyModule_AddObject(m, "datetimetz", (PyObject *) &DatetimeTZType);
    /* additional initialization can happen here */
    return m;
}

