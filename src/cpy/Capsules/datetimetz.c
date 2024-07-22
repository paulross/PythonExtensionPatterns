//
// Created by Paul Ross on 13/07/2024.
//
// Implements a datatimetz subclass of datetime that always has a timezone.
// This is an example of using Capsules and the datetime Capsule API.

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "datetime.h"
#include "py_call_super.h"

#define FPRINTF_DEBUG 0

/* From /Library/Frameworks/Python.framework/Versions/3.13/include/python3.13/object.h
 * These were introduced in Python 3.10: https://docs.python.org/3/c-api/structures.html#c.Py_IsNone
 * */
#if PY_MINOR_VERSION < 10
// Test if the 'x' object is the 'y' object, the same as "x is y" in Python.
PyAPI_FUNC(int) Py_Is(PyObject *x, PyObject *y);
#define Py_Is(x, y) ((x) == (y))

// Test if an object is the None singleton, the same as "x is None" in Python.
PyAPI_FUNC(int) Py_IsNone(PyObject *x);
#define Py_IsNone(x) Py_Is((x), Py_None)

//PyAPI_FUNC(int) _PyDateTime_HAS_TZINFO(PyObject *datetime) {
//    if (datetime->tzinfo == NULL) {
//        return -1;
//    } else if (Py_IsNone(datetime->tzinfo)) {
//        PyErr_SetString(PyExc_TypeError, "No time zone provided.");
//        return -2;
//    }
//    return 0;
//}
#endif

typedef struct {
    PyDateTime_DateTime datetime;
} DateTimeTZ;

/**
 * Does a version dependent check to see if a datatimetz has a tzinfo.
 * If not, sets an error and returns NULL.
 */
static DateTimeTZ *
raise_if_no_tzinfo(DateTimeTZ *self) {
    // Raise if no TZ.
#if PY_MINOR_VERSION >= 10
    if (!_PyDateTime_HAS_TZINFO(&self->datetime)) {
        PyErr_SetString(PyExc_TypeError, "No time zone provided.");
        Py_DECREF(self);
        self = NULL;
    }
#else // PY_MINOR_VERSION < 10
    if (self->datetime.tzinfo == NULL) {
        PyErr_SetString(PyExc_TypeError, "No time zone provided.");
        Py_DECREF(self);
        self = NULL;
    } else if (Py_IsNone(self->datetime.tzinfo)) {
        PyErr_SetString(PyExc_TypeError, "No time zone provided.");
        Py_DECREF(self);
        self = NULL;
    }
#endif // PY_MINOR_VERSION
    return self;
}

static PyObject *
DateTimeTZ_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
#if FPRINTF_DEBUG
    fprintf(stdout, "DateTimeTZ_new() type:\n");
    PyObject_Print((PyObject *)type, stdout, Py_PRINT_RAW);
    fprintf(stdout, "\n");
    fprintf(stdout, "DateTimeTZ_new() args:\n");
    PyObject_Print(args, stdout, Py_PRINT_RAW);
    fprintf(stdout, "\n");
    fprintf(stdout, "DateTimeTZ_new() kwds:\n");
    PyObject_Print(kwds, stdout, Py_PRINT_RAW);
    fprintf(stdout, "\n");
#endif
    DateTimeTZ *self = (DateTimeTZ *) PyDateTimeAPI->DateTimeType->tp_new(type, args, kwds);
    if (self) {
#if FPRINTF_DEBUG
        fprintf(stdout, "DateTimeTZ_new() self:\n");
        PyObject_Print((PyObject *)self, stdout, Py_PRINT_RAW);
        fprintf(stdout, "\n");
        fprintf(stdout, "DateTimeTZ_new() &self->datetime:\n");
        PyObject_Print((PyObject*)(&self->datetime), stdout, Py_PRINT_RAW);
        fprintf(stdout, "\n");
#if PY_MINOR_VERSION >= 10
        fprintf(stdout, "DateTimeTZ_new() _PyDateTime_HAS_TZINFO(&self->datetime): %d\n", _PyDateTime_HAS_TZINFO(&self->datetime));
#else // PY_MINOR_VERSION >= 10
        fprintf(stdout, "DateTimeTZ_new() self->datetime.tzinfo:\n");
        if ((void *)&self->datetime != NULL && (&self->datetime)->tzinfo) {
//            fprintf(stdout, "tzinfo %p %s\n", (void *)(&self->datetime)->tzinfo, Py_TYPE((&self->datetime)->tzinfo)->tp_name);
            PyObject_Print((PyObject *) ((&self->datetime)->tzinfo), stdout, Py_PRINT_RAW);
        } else {
            fprintf(stdout, "No tzinfo\n");
        }
        fprintf(stdout, "\n");
#endif // PY_MINOR_VERSION < 10
#endif
        self = raise_if_no_tzinfo(self);
    }
    return (PyObject *) self;
}

static PyObject *
DateTimeTZ_replace(PyObject *self, PyObject *args, PyObject *kwargs) {
//    PyObject_Print(self, stdout, 0);
    PyObject *result = call_super_name(self, "replace", args, kwargs);
//    PyObject_Print(self, stdout, 0);
    if (result) {
        result = (PyObject *) raise_if_no_tzinfo((DateTimeTZ *) result);
    }
    return result;
}

static PyMethodDef DateTimeTZ_methods[] = {
        {
                "replace",
                (PyCFunction) DateTimeTZ_replace,
                METH_VARARGS | METH_KEYWORDS,
                PyDoc_STR("Return datetime with new specified fields.")
        },
        {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyTypeObject DatetimeTZType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "datetimetz.datetimetz",
        .tp_doc = "A datetime that requires a time zone.",
        .tp_basicsize = sizeof(DateTimeTZ),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = DateTimeTZ_new,
        .tp_methods = DateTimeTZ_methods,
};

static PyModuleDef datetimetzmodule = {
        PyModuleDef_HEAD_INIT,
        .m_name = "datetimetz",
        .m_doc = (
                "Module that contains a datetimetz,"
                "a datetime.datetime with a mandatory time zone."
        ),
        .m_size = -1,
};

PyMODINIT_FUNC
PyInit_datetimetz(void) {
    PyObject *m = PyModule_Create(&datetimetzmodule);
    if (m == NULL) {
        return NULL;
    }
    // datetime.datetime_CAPI
    PyDateTime_IMPORT;
    if (!PyDateTimeAPI) {
        Py_DECREF(m);
        return NULL;
    }
    // Set inheritance.
    DatetimeTZType.tp_base = PyDateTimeAPI->DateTimeType;
    if (PyType_Ready(&DatetimeTZType) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(&DatetimeTZType);
    PyModule_AddObject(m, "datetimetz", (PyObject *) &DatetimeTZType);
    /* additional initialization can happen here */
    return m;
}
