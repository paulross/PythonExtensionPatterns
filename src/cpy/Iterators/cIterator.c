//
// Created by Paul Ross on 08/07/2021.
//
// Example of a iterator.
//
#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "structmember.h"

typedef struct {
    PyObject_HEAD
    long *array_long;
    ssize_t size;
} SequenceOfLong;

typedef struct {
    PyObject_HEAD
    PyObject *sequence;
    size_t index;
} SequenceOfLongIterator;

static PyObject *
SequenceOfLongIterator_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    SequenceOfLongIterator *self;
    self = (SequenceOfLongIterator *) type->tp_alloc(type, 0);
    if (self != NULL) {
        assert(!PyErr_Occurred());
    }
    return (PyObject *) self;
}

// Forward reference
static int is_sequence_of_long_type(PyObject *op);

static int
SequenceOfLongIterator_init(SequenceOfLongIterator *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"sequence", NULL};
    PyObject *sequence = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &sequence)) {
        return -1;
    }
    if (!is_sequence_of_long_type(sequence)) {
        PyErr_Format(
                PyExc_ValueError,
                "Argument must be a SequenceOfLongType, not type %s",
                Py_TYPE(sequence)->tp_name
                );
        return -2;
    }
    // Borrowed reference
    // Keep the sequence alive as long as the iterator is alive.
    // Decrement on iterator de-allocation.
    Py_INCREF(sequence);
    self->sequence = sequence;
    self->index = 0;
    return 0;
}

static void
SequenceOfLongIterator_dealloc(SequenceOfLongIterator *self) {
    // Decrement borrowed reference.
    Py_XDECREF(self->sequence);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
SequenceOfLongIterator_next(SequenceOfLongIterator *self) {
    size_t size = ((SequenceOfLong *) self->sequence)->size;
    if (self->index < size) {
        PyObject *ret = PyLong_FromLong(((SequenceOfLong *) self->sequence)->array_long[self->index]);
        self->index += 1;
        return ret;
    }
    // End iteration.
    return NULL;
}

static PyObject *
SequenceOfLongIterator___str__(SequenceOfLongIterator *self, PyObject *Py_UNUSED(ignored)) {
    assert(!PyErr_Occurred());
    if (self->sequence) {
        return PyUnicode_FromFormat(
                "<SequenceOfLong iterator @: %p of size %ld index %ld",
                self->sequence, ((SequenceOfLong *) self->sequence)->size, self->index
        );
    } else {
        return PyUnicode_FromFormat(
                "<SequenceOfLong iterator @: %p of size NULL sequence (exhausted) index %ld",
                self->sequence, self->index
        );
    }
}

static PyTypeObject SequenceOfLongIteratorType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "SequenceOfLongIterator",
        .tp_basicsize = sizeof(SequenceOfLongIterator),
        .tp_itemsize = 0,
        .tp_dealloc = (destructor) SequenceOfLongIterator_dealloc,
        .tp_str = (reprfunc) SequenceOfLongIterator___str__,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = "SequenceOfLongIterator object.",
        .tp_iter = PyObject_SelfIter,
        .tp_iternext = (iternextfunc) SequenceOfLongIterator_next,
        .tp_init = (initproc) SequenceOfLongIterator_init,
        .tp_new = SequenceOfLongIterator_new,
};

static PyObject *
SequenceOfLong_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    SequenceOfLong *self;
    self = (SequenceOfLong *) type->tp_alloc(type, 0);
    if (self != NULL) {
        assert(!PyErr_Occurred());
        self->size = 0;
        self->array_long = NULL;
    }
    return (PyObject *) self;
}

static int
SequenceOfLong_init(SequenceOfLong *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"sequence", NULL};
    PyObject *sequence = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &sequence)) {
        return -1;
    }
    if (!PySequence_Check(sequence)) {
        return -2;
    }
    self->size = PySequence_Length(sequence);
    self->array_long = malloc(self->size * sizeof(long));
    if (! self->array_long) {
        return -3;
    }
    for (Py_ssize_t i = 0; i < PySequence_Length(sequence); ++i) {
        // New reference.
        PyObject *py_value = PySequence_GetItem(sequence, i);
        if (PyLong_Check(py_value)) {
            self->array_long[i] = PyLong_AsLong(py_value);
            Py_DECREF(py_value);
        } else {
            PyErr_Format(
                    PyExc_TypeError,
                    "Argument [%zd] must be a int, not type %s",
                    i,
                    Py_TYPE(sequence)->tp_name
            );
            // Clean up on error.
            free(self->array_long);
            self->array_long = NULL;
            Py_DECREF(py_value);
            return -4;
        }
    }
    return 0;
}

static void
SequenceOfLong_dealloc(SequenceOfLong *self) {
    free(self->array_long);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
SequenceOfLong_size(SequenceOfLong *self, PyObject *Py_UNUSED(ignored)) {
    return Py_BuildValue("n", self->size);
}

static PyObject *
SequenceOfLong_iter(SequenceOfLong *self) {
    PyObject *ret = SequenceOfLongIterator_new(&SequenceOfLongIteratorType, NULL, NULL);
    if (ret) {
        PyObject *args = Py_BuildValue("(O)", self);
        if (!args || SequenceOfLongIterator_init((SequenceOfLongIterator *) ret, args, NULL)) {
            Py_DECREF(ret);
            ret = NULL;
        }
        Py_DECREF(args);
    }
    return ret;
}

static PyMethodDef SequenceOfLong_methods[] = {
        {
            "size",
                (PyCFunction) SequenceOfLong_size,
            METH_NOARGS,
            "Return the size of the sequence."
        },
        {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyObject *
SequenceOfLong___str__(SequenceOfLong *self, PyObject *Py_UNUSED(ignored)) {
    assert(!PyErr_Occurred());
    return PyUnicode_FromFormat("<SequenceOfLong sequence size: %ld>", self->size);
}

static PyTypeObject SequenceOfLongType= {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "SequenceOfLong",
        .tp_basicsize = sizeof(SequenceOfLong),
        .tp_itemsize = 0,
        .tp_dealloc = (destructor) SequenceOfLong_dealloc,
        .tp_str = (reprfunc) SequenceOfLong___str__,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = "Sequence of long integers.",
        .tp_iter = (getiterfunc) SequenceOfLong_iter,
//        .tp_iternext = (iternextfunc) SequenceOfLongIterator_next,
        .tp_methods = SequenceOfLong_methods,
        .tp_init = (initproc) SequenceOfLong_init,
        .tp_new = SequenceOfLong_new,
};

static int
is_sequence_of_long_type(PyObject *op) {
    return Py_TYPE(op) == &SequenceOfLongType;
}

static PyModuleDef iterator_cmodule = {
        PyModuleDef_HEAD_INIT,
        .m_name = "cIterator",
        .m_doc = (
            "Example module that creates an extension type"
            "that has forward and reverse iterators."
        ),
        .m_size = -1,
};

PyMODINIT_FUNC
PyInit_cIterator(void) {
    PyObject *m;
    m = PyModule_Create(&iterator_cmodule);
    if (m == NULL) {
        return NULL;
    }

    if (PyType_Ready(&SequenceOfLongType) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(&SequenceOfLongType);
    if (PyModule_AddObject(
            m,
            "SequenceOfLong",
            (PyObject *) &SequenceOfLongType) < 0
            ) {
        Py_DECREF(&SequenceOfLongType);
        Py_DECREF(m);
        return NULL;
    }
    if (PyType_Ready(&SequenceOfLongIteratorType) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(&SequenceOfLongIteratorType);
    // Not strictly necessary unless you need to expose this type.
    // For type checking for example.
    if (PyModule_AddObject(
            m,
            "SequenceOfLongIterator",
            (PyObject *) &SequenceOfLongIteratorType) < 0
            ) {
        Py_DECREF(&SequenceOfLongType);
        Py_DECREF(&SequenceOfLongIteratorType);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
