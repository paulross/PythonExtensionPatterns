//
//  custom_pickle.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 08/04/2021.
//  Copyright (c) 2021 Paul Ross. All rights reserved.
//
// This adds pickling to a standard custom object.

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"

#define FPRINTF_DEBUG 0

typedef struct {
    PyObject_HEAD
    PyObject *first; /* first name */
    PyObject *last;  /* last name */
    int number;
} CustomObject;

static void
Custom_dealloc(CustomObject *self)
{
    Py_XDECREF(self->first);
    Py_XDECREF(self->last);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
Custom_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds))
{
    CustomObject *self;
    self = (CustomObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->first = PyUnicode_FromString("");
        if (self->first == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->last = PyUnicode_FromString("");
        if (self->last == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        self->number = 0;
    }
#if FPRINTF_DEBUG
    fprintf(stdout, "Custom_new() reference counts first %zu last %zu\n", Py_REFCNT(self->first), Py_REFCNT(self->last));
#endif
    return (PyObject *) self;
}

static int
Custom_init(CustomObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"first", "last", "number", NULL};
    PyObject *first = NULL, *last = NULL, *tmp;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOi", kwlist,
                                     &first, &last,
                                     &self->number))
        return -1;

    if (first) {
        tmp = self->first;
        Py_INCREF(first);
        self->first = first;
        Py_XDECREF(tmp);
    }
    if (last) {
        tmp = self->last;
        Py_INCREF(last);
        self->last = last;
        Py_XDECREF(tmp);
    }
    return 0;
}

static PyMemberDef Custom_members[] = {
        {"first", T_OBJECT_EX, offsetof(CustomObject, first), 0,
                "first name"},
        {"last", T_OBJECT_EX, offsetof(CustomObject, last), 0,
                "last name"},
        {"number", T_INT, offsetof(CustomObject, number), 0,
                "custom number"},
        {NULL, 0, 0, 0, NULL}  /* Sentinel */
};

static PyObject *
Custom_name(CustomObject *self, PyObject *Py_UNUSED(ignored))
{
    if (self->first == NULL) {
        PyErr_SetString(PyExc_AttributeError, "first");
        return NULL;
    }
    if (self->last == NULL) {
        PyErr_SetString(PyExc_AttributeError, "last");
        return NULL;
    }
    return PyUnicode_FromFormat("%S %S", self->first, self->last);
}

/* Pickle the object */
static const char* PICKLE_VERSION_KEY = "_pickle_version";
static int PICKLE_VERSION = 1;

static PyObject *
Custom___getstate__(CustomObject *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *ret = Py_BuildValue("{sOsOsisi}",
                                  "first", self->first,
                                  "last", self->last,
                                  "number", self->number,
                                  PICKLE_VERSION_KEY, PICKLE_VERSION);
#if FPRINTF_DEBUG
    fprintf(stdout, "Custom___getstate__ returning type %s\n", Py_TYPE(ret)->tp_name);
#endif
    return ret;
}


static PyObject *
Custom___setstate__(CustomObject *self, PyObject *state) {
#if FPRINTF_DEBUG
    fprintf(stdout, "Custom___getstate__ getting type %s\n", Py_TYPE(state)->tp_name);
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(state, &pos, &key, &value)) {
        /* do something interesting with the values... */
        fprintf(stdout, "Types Key: %s Value: %s\n", Py_TYPE(key)->tp_name, Py_TYPE(value)->tp_name);
        fprintf(stdout, "Key ");
        PyObject_Print(key, stdout, Py_PRINT_RAW);
        fprintf(stdout, " = ");
        PyObject_Print(value, stdout, Py_PRINT_RAW);
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "Initial reference counts first %zu last %zu\n", Py_REFCNT(self->first), Py_REFCNT(self->last));
#endif

//    static char *kwlist[] = {"first", "last", "number", NULL};
//
//    PyArg_ParseTupleAndKeywords(args, state, "OOi", kwlist, &self->first, &self->last, &self->number);

#if 0
    //    PyObject *key = NULL;
        Py_DECREF(self->first);
        key = Py_BuildValue("s", "first");
        self->first = PyDict_GetItem(state, key);
        Py_DECREF(key);
        Py_INCREF(self->first);

        Py_DECREF(self->last);
        key = Py_BuildValue("s", "last");
        self->last = PyDict_GetItem(state, key);
        Py_DECREF(key);
        Py_INCREF(self->last);

        key = Py_BuildValue("s", "number");
        self->number = PyLong_AsLong(PyDict_GetItem(state, key));
        Py_DECREF(key);
#endif
    if (!PyDict_CheckExact(state)) {
        PyErr_SetString(PyExc_ValueError, "Pickled object is not a dict.");
        return NULL;
    }
    /* Version check. */
    /* Borrowed reference but no need to increment as we create a C long from it. */
    PyObject *temp = PyDict_GetItemString(state, PICKLE_VERSION_KEY);
    if (temp == NULL) {
        /* PyDict_GetItemString does not set any error state so we have to. */
        PyErr_Format(PyExc_KeyError, "No \"%s\" in pickled dict.", PICKLE_VERSION_KEY);
        return NULL;
    }
    int pickle_version = (int) PyLong_AsLong(temp);
    if (pickle_version != PICKLE_VERSION) {
        PyErr_Format(PyExc_ValueError, "Pickle version mismatch. Got version %d but expected version %d.",
                     pickle_version, PICKLE_VERSION);
        return NULL;
    }

    Py_DECREF(self->first);
    self->first = PyDict_GetItemString(state, "first"); /* Borrowed reference. */
    if (self->first == NULL) {
        /* PyDict_GetItemString does not set any error state so we have to. */
        PyErr_SetString(PyExc_KeyError, "No \"first\" in pickled dict.");
        return NULL;
    }
    Py_INCREF(self->first);

    Py_DECREF(self->last);
    self->last = PyDict_GetItemString(state, "last"); /* Borrowed reference. */
    if (self->last == NULL) {
        /* PyDict_GetItemString does not set any error state so we have to. */
        PyErr_SetString(PyExc_KeyError, "No \"last\" in pickled dict.");
        return NULL;
    }
    Py_INCREF(self->last);

    /* Borrowed reference but no need to increment as we create a C long from it. */
    PyObject *number = PyDict_GetItemString(state, "number");
    if (number == NULL) {
        /* PyDict_GetItemString does not set any error state so we have to. */
        PyErr_SetString(PyExc_KeyError, "No \"number\" in pickled dict.");
        return NULL;
    }
    self->number = (int) PyLong_AsLong(number);
#if FPRINTF_DEBUG
    fprintf(stdout, "Final reference counts first %zu last %zu\n", Py_REFCNT(self->first),
            Py_REFCNT(self->last));
#endif
    Py_RETURN_NONE;
}

static PyMethodDef Custom_methods[] = {
    {"name", (PyCFunction) Custom_name, METH_NOARGS,
            "Return the name, combining the first and last name"
    },
    {"__getstate__", (PyCFunction) Custom___getstate__, METH_NOARGS,
            "Return the state for pickling"
    },
    {"__setstate__", (PyCFunction) Custom___setstate__, METH_O,
            "Set the state from a pickle"
    },
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyTypeObject CustomType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "cPyExtPatt.cPickle.Custom",
        .tp_doc = "Custom objects",
        .tp_basicsize = sizeof(CustomObject),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_new = Custom_new,
        .tp_init = (initproc) Custom_init,
        .tp_dealloc = (destructor) Custom_dealloc,
        .tp_members = Custom_members,
        .tp_methods = Custom_methods,
};

static PyModuleDef cPicklemodule = {
        PyModuleDef_HEAD_INIT,
        .m_name = "cPickle",
        .m_doc = "Example module that creates a pickleable extension type.",
        .m_size = -1,
};

PyMODINIT_FUNC
PyInit_cPickle(void)
{
    PyObject *m;
    if (PyType_Ready(&CustomType) < 0)
        return NULL;

    m = PyModule_Create(&cPicklemodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&CustomType);
    if (PyModule_AddObject(m, "Custom", (PyObject *) &CustomType) < 0) {
        Py_DECREF(&CustomType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
