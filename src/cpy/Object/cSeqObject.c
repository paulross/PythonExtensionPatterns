//
// Created by Paul Ross on 08/07/2021.
//
// Example of an object implementing the sequence methods with PySequenceMethods.
//
// See also src/cpy/Iterators/cIterator.c
#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "structmember.h"

typedef struct {
    PyObject_HEAD
    long *array_long;
    ssize_t size;
} SequenceLongObject;

// Forward references
static PyTypeObject SequenceLongObjectType;

static int is_sequence_of_long_type(PyObject *op);

static PyObject *
SequenceLongObject_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    SequenceLongObject *self;
    self = (SequenceLongObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
        assert(!PyErr_Occurred());
        self->size = 0;
        self->array_long = NULL;
    }
    return (PyObject *) self;
}

static int
SequenceLongObject_init(SequenceLongObject *self, PyObject *args, PyObject *kwds) {
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
    if (!self->array_long) {
        return -3;
    }
    for (Py_ssize_t i = 0; i < self->size; ++i) {
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
SequenceLongObject_dealloc(SequenceLongObject *self) {
    free(self->array_long);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyMethodDef SequenceLongObject_methods[] = {
//        {
//                "size",
//                (PyCFunction) SequenceLongObject_size,
//                METH_NOARGS,
//                "Return the size of the sequence."
//        },
        {NULL, NULL, 0, NULL}  /* Sentinel */
};

/* Sequence methods. */
static Py_ssize_t
SequenceLongObject_sq_length(PyObject *self) {
    return ((SequenceLongObject *) self)->size;
}

/**
 * Returns a new SequenceLongObject composed of self + other.
 * @param self
 * @param other
 * @return
 */
static PyObject *
SequenceLongObject_sq_concat(PyObject *self, PyObject *other) {
    if (!is_sequence_of_long_type(other)) {
        PyErr_Format(
                PyExc_TypeError,
                "%s(): argument 1 must have type \"SequenceLongObject\" not %s",
                Py_TYPE(other)->tp_name
        );
        return NULL;
    }
    PyObject *ret = SequenceLongObject_new(&SequenceLongObjectType, NULL, NULL);
    /* For convenience. */
    SequenceLongObject *sol = (SequenceLongObject *) ret;
    sol->size = ((SequenceLongObject *) self)->size + ((SequenceLongObject *) other)->size;
    sol->array_long = malloc(sol->size * sizeof(long));
    if (!sol->array_long) {
        PyErr_Format(PyExc_MemoryError, "%s(): Can not create new object.", __FUNCTION__);
    }

    ssize_t i = 0;
    ssize_t ub = ((SequenceLongObject *) self)->size;
    while (i < ub) {
        sol->array_long[i] = ((SequenceLongObject *) self)->array_long[i];
        i++;
    }
    ub += ((SequenceLongObject *) other)->size;
    while (i < ub) {
        sol->array_long[i] = ((SequenceLongObject *) other)->array_long[i];
        i++;
    }
    return ret;
}


static PyObject *
SequenceLongObject_sq_item(PyObject *self, Py_ssize_t index) {
    Py_ssize_t my_index = index;
    if (my_index < 0) {
        my_index += SequenceLongObject_sq_length(self);
    }
    if (my_index > SequenceLongObject_sq_length(self)) {
        PyErr_Format(
                PyExc_IndexError,
                "Index %ld is out of range for length %ld",
                index,
                SequenceLongObject_sq_length(self)
        );
        return NULL;
    }
    return PyLong_FromLong(((SequenceLongObject *) self)->array_long[my_index]);
}

PySequenceMethods SequenceLongObject_sequence_methods = {
        .sq_length = &SequenceLongObject_sq_length,
        .sq_concat = &SequenceLongObject_sq_concat,
        .sq_repeat = NULL,
        .sq_item = &SequenceLongObject_sq_item,
        .sq_ass_item = NULL,
        .sq_contains = NULL,
        .sq_inplace_concat = NULL,
        .sq_inplace_repeat = NULL,
};

static PyObject *
SequenceLongObject___str__(SequenceLongObject *self, PyObject *Py_UNUSED(ignored)) {
    assert(!PyErr_Occurred());
    return PyUnicode_FromFormat("<SequenceLongObject sequence size: %ld>", self->size);
}

static PyTypeObject SequenceLongObjectType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "SequenceLongObject",
        .tp_basicsize = sizeof(SequenceLongObject),
        .tp_itemsize = 0,
        .tp_dealloc = (destructor) SequenceLongObject_dealloc,
        .tp_as_sequence = &SequenceLongObject_sequence_methods,
        .tp_str = (reprfunc) SequenceLongObject___str__,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = "Sequence of long integers.",
        .tp_methods = SequenceLongObject_methods,
        .tp_init = (initproc) SequenceLongObject_init,
        .tp_new = SequenceLongObject_new,
};

static int
is_sequence_of_long_type(PyObject *op) {
    return Py_TYPE(op) == &SequenceLongObjectType;
}

static PyMethodDef cIterator_methods[] = {
//        {"iterate_and_print", (PyCFunction) iterate_and_print, METH_VARARGS,
//                "Iteratee through the argument printing the values."},
        {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef sequence_object_cmodule = {
        PyModuleDef_HEAD_INIT,
        .m_name = "cSeqObject",
        .m_doc = (
                "Example module that creates an extension type with sequence methods"
        ),
        .m_size = -1,
        .m_methods = cIterator_methods,
};

PyMODINIT_FUNC
PyInit_cSeqObject(void) {
    PyObject *m;
    m = PyModule_Create(&sequence_object_cmodule);
    if (m == NULL) {
        return NULL;
    }

    if (PyType_Ready(&SequenceLongObjectType) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(&SequenceLongObjectType);
    if (PyModule_AddObject(
            m,
            "SequenceLongObject",
            (PyObject *) &SequenceLongObjectType) < 0
            ) {
        Py_DECREF(&SequenceLongObjectType);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
