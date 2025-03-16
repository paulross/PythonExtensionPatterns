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
} SequenceOfLong;

// Forward reference
static int is_sequence_of_long_type(PyObject *op);

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
    if (!self->array_long) {
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

//static PyObject *
//SequenceOfLong_size(SequenceOfLong *self, PyObject *Py_UNUSED(ignored)) {
//    return Py_BuildValue("n", self->size);
//}

static PyMethodDef SequenceOfLong_methods[] = {
//        {
//                "size",
//                (PyCFunction) SequenceOfLong_size,
//                METH_NOARGS,
//                "Return the size of the sequence."
//        },
        {NULL, NULL, 0, NULL}  /* Sentinel */
};

/* Sequence methods. */
static Py_ssize_t
SequenceOfLong_len(PyObject *self) {
    return ((SequenceOfLong *)self)->size;
}

static PyObject *
SequenceOfLong_getitem(PyObject *self, Py_ssize_t index) {
    Py_ssize_t my_index = index;
    if (my_index < 0) {
        my_index += SequenceOfLong_len(self);
    }
    if (my_index > SequenceOfLong_len(self)) {
        PyErr_Format(
                PyExc_IndexError,
                "Index %ld is out of range for length %ld",
                index,
                SequenceOfLong_len(self)
        );
        return NULL;
    }
    return PyLong_FromLong(((SequenceOfLong *)self)->array_long[my_index]);
}

PySequenceMethods SequenceOfLong_sequence_methods = {
        .sq_length = &SequenceOfLong_len,
        .sq_concat = NULL,
        .sq_repeat = NULL,
        .sq_item = &SequenceOfLong_getitem,
        .sq_ass_item = NULL,
        .sq_contains = NULL,
        .sq_inplace_concat = NULL,
        .sq_inplace_repeat = NULL,
};

static PyObject *
SequenceOfLong___str__(SequenceOfLong *self, PyObject *Py_UNUSED(ignored)) {
    assert(!PyErr_Occurred());
    return PyUnicode_FromFormat("<SequenceOfLong sequence size: %ld>", self->size);
}

static PyTypeObject SequenceOfLongType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "SequenceOfLong",
        .tp_basicsize = sizeof(SequenceOfLong),
        .tp_itemsize = 0,
        .tp_dealloc = (destructor) SequenceOfLong_dealloc,
        .tp_as_sequence = &SequenceOfLong_sequence_methods,
        .tp_str = (reprfunc) SequenceOfLong___str__,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = "Sequence of long integers.",
        .tp_methods = SequenceOfLong_methods,
        .tp_init = (initproc) SequenceOfLong_init,
        .tp_new = SequenceOfLong_new,
};

static int
is_sequence_of_long_type(PyObject *op) {
    return Py_TYPE(op) == &SequenceOfLongType;
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
    return m;
}
