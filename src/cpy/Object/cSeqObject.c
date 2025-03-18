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
//    fprintf(stdout, "%s(%p): returns=%zd\n", __FUNCTION__, (void *) self, ((SequenceLongObject *) self)->size);
    return ((SequenceLongObject *) self)->size;
}

// Forward references
static PyTypeObject SequenceLongObjectType;

static int is_sequence_of_long_type(PyObject *op);

/**
 * Returns a new SequenceLongObject composed of self + other.
 * @param self
 * @param other
 * @return
 */
static PyObject *
SequenceLongObject_sq_concat(PyObject *self, PyObject *other) {
//    fprintf(stdout, "%s(%p):\n", __FUNCTION__, (void *) self);
    if (!is_sequence_of_long_type(other)) {
        PyErr_Format(
                PyExc_TypeError,
                "%s(): argument 1 must have type \"SequenceLongObject\" not %s",
                Py_TYPE(other)->tp_name
        );
        return NULL;
    }
    PyObject *ret = SequenceLongObject_new(&SequenceLongObjectType, NULL, NULL);
    if (!ret) {
        assert(PyErr_Occurred());
        return NULL;
    }
    /* For convenience. */
    SequenceLongObject *ret_as_slo = (SequenceLongObject *) ret;
    ret_as_slo->size = ((SequenceLongObject *) self)->size + ((SequenceLongObject *) other)->size;
    ret_as_slo->array_long = malloc(ret_as_slo->size * sizeof(long));
    if (!ret_as_slo->array_long) {
        PyErr_Format(PyExc_MemoryError, "%s(): Can not create new object.", __FUNCTION__);
        Py_DECREF(ret);
        return NULL;
    }
//    fprintf(stdout, "%s(): New %p size=%zd\n", __FUNCTION__, (void *) ret_as_slo, ret_as_slo->size);

    ssize_t i = 0;
    ssize_t ub = ((SequenceLongObject *) self)->size;
    while (i < ub) {
//        fprintf(stdout, "%s(): Setting from %p [%zd] to [%zd]\n", __FUNCTION__, (void *) self, i, i);
        ret_as_slo->array_long[i] = ((SequenceLongObject *) self)->array_long[i];
        i++;
    }
    ssize_t j = 0;
    ub = ((SequenceLongObject *) other)->size;
    while (j < ub) {
//        fprintf(stdout, "%s(): Setting %p [%zd] to [%zd]\n", __FUNCTION__, (void *) other, j, i);
        ret_as_slo->array_long[i] = ((SequenceLongObject *) other)->array_long[j];
        i++;
        j++;
    }
    return ret;
}

/**
 * Return a new sequence which contains the old one repeated count times.
 * @param self
 * @param count
 * @return
 */
static PyObject *
SequenceLongObject_sq_repeat(PyObject *self, Py_ssize_t count) {
    PyObject *ret = SequenceLongObject_new(&SequenceLongObjectType, NULL, NULL);
    if (!ret) {
        assert(PyErr_Occurred());
        return NULL;
    }
    assert(ret != self);
    if (((SequenceLongObject *) self)->size > 0 && count > 0) {
        /* For convenience. */
        SequenceLongObject *self_as_slo = (SequenceLongObject *) self;
        SequenceLongObject *ret_as_slo = (SequenceLongObject *) ret;
        ret_as_slo->size = self_as_slo->size * count;
        assert(ret_as_slo->size > 0);
        ret_as_slo->array_long = malloc(ret_as_slo->size * sizeof(long));
        if (!ret_as_slo->array_long) {
            PyErr_Format(PyExc_MemoryError, "%s(): Can not create new object.", __FUNCTION__);
            Py_DECREF(ret);
            return NULL;
        }
        Py_ssize_t ret_index = 0;
        for (Py_ssize_t i = 0; i < count; ++i) {
//            fprintf(stdout, "%s(): Setting %p Count %zd\n", __FUNCTION__, (void *) ret, i);
            for (Py_ssize_t j = 0; j < self_as_slo->size; ++j) {
//                fprintf(
//                    stdout, "%s(): Setting %p [%zd] to %zd\n",
//                    __FUNCTION__, (void *) ret, ret_index, self_as_slo->array_long[j]
//                );
                ret_as_slo->array_long[ret_index] = self_as_slo->array_long[j];
                ++ret_index;
            }
        }
    } else {
        /* Empty sequence. */
    }
    return ret;
}

/**
 * Returns a new reference to an indexed item in a sequence.
 * @param self
 * @param index
 * @return
 */
static PyObject *
SequenceLongObject_sq_item(PyObject *self, Py_ssize_t index) {
//    fprintf(stdout, "%s(): index=%zd\n", __FUNCTION__, index);
    Py_ssize_t my_index = index;
    if (my_index < 0) {
        my_index += SequenceLongObject_sq_length(self);
    }
    // Corner case example: len(self) == 0 and index < 0
    if (my_index < 0 || my_index >= SequenceLongObject_sq_length(self)) {
//        fprintf(stdout, "%s(): index=%zd\n", __FUNCTION__, index);
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

static int
SequenceLongObject_sq_ass_item(PyObject *self, Py_ssize_t index, PyObject *value) {
    fprintf(
        stdout, "%s()#%d: self=%p index=%zd value=%p\n",
        __FUNCTION__, __LINE__, (void *) self, index, (void *) value
    );
    /* This is very weird.
     * When the given index is negative and out of range PyObject_SetItem()
     * and PyObject_DelItem() will have *already* added the sequence length
     * before calling this function.
     * So to get the original out of range negative index we have to *subtract*
     * the sequence length. */
    if (index < 0) {
        fprintf(
            stdout, "%s()#%d: Fixing index index=%zd to %zd\n", __FUNCTION__, __LINE__,
            index, index - SequenceLongObject_sq_length(self)
        );
        index -= SequenceLongObject_sq_length(self);
    }
    /* Isn't it? */
    Py_ssize_t my_index = index;
    if (my_index < 0) {
        my_index += SequenceLongObject_sq_length(self);
    }
    // Corner case example: len(self) == 0 and index < 0
    fprintf(
        stdout, "%s()#%d: len=%zd index=%zd my_index=%zd\n", __FUNCTION__, __LINE__,
        SequenceLongObject_sq_length(self), index, my_index
    );
    if (my_index < 0 || my_index >= SequenceLongObject_sq_length(self)) {
        PyErr_Format(
                PyExc_IndexError,
                "Index %ld is out of range for length %ld",
                index,
                SequenceLongObject_sq_length(self)
        );
        return -1;
    }
    if (value != NULL) {
        /* Just set the value. */
        if (!PyLong_Check(value)) {
            PyErr_Format(
                    PyExc_TypeError,
                    "sq_ass_item value needs to be an int, not type %s",
                    Py_TYPE(value)->tp_name
            );
            return -1;
        }
        ((SequenceLongObject *) self)->array_long[my_index] = PyLong_AsLong(value);
    } else {
        /* Delete the value. */
        /* For convenience. */
        SequenceLongObject *self_as_slo = (SequenceLongObject *) self;
        /* Special case: deleting the only item in the array. */
        if (self_as_slo->size == 1) {
            fprintf(stdout, "%s()#%d: deleting empty index\n", __FUNCTION__, __LINE__);
            free(self_as_slo->array_long);
            self_as_slo->array_long = NULL;
            self_as_slo->size = 0;
        } else {
            /* Delete the value and re-compose the array. */
            fprintf(stdout, "%s()#%d: deleting index=%zd\n", __FUNCTION__, __LINE__, index);
            long *new_array = malloc((self_as_slo->size - 1) * sizeof(long));
            if (!new_array) {
                PyErr_Format(
                        PyExc_MemoryError,
                        "sq_ass_item can not allocate new array. %s#%d",
                        __FILE__, __LINE__
                );
                return -1;
            }
            /* Copy up to the index. */
            Py_ssize_t index_new_array = 0;
            for (Py_ssize_t i = 0; i < my_index; ++i, ++index_new_array) {
                new_array[index_new_array] = self_as_slo->array_long[i];
            }
            /* Copy past the index. */
            for (Py_ssize_t i = my_index + 1; i < self_as_slo->size; ++i, ++index_new_array) {
                new_array[index_new_array] = self_as_slo->array_long[i];
            }

            free(self_as_slo->array_long);
            self_as_slo->array_long = new_array;
            --self_as_slo->size;
        }
    }
    return 0;
}

/**
 * If an item in self is equal to value, return 1, otherwise return 0. On error, return -1.
 * @param self
 * @param value
 * @return
 */
static int
SequenceLongObject_sq_contains(PyObject *self, PyObject *value) {
    fprintf(
            stdout, "%s()#%d: self=%p value=%p\n",
            __FUNCTION__, __LINE__, (void *) self, (void *) value
    );
    if (!PyLong_Check(value)) {
        /* Alternates: Could raise TypeError or return -1.
         * Here we act benignly! */
        return 0;
    }
    long c_value = PyLong_AsLong(value);
    /* For convenience. */
    SequenceLongObject *self_as_slo = (SequenceLongObject *) self;
    for (Py_ssize_t i = 0; i < SequenceLongObject_sq_length(self); ++i) {
        if (self_as_slo->array_long[i] == c_value) {
            return 1;
        }
    }
    return 0;
}

static PySequenceMethods SequenceLongObject_sequence_methods = {
        .sq_length = (lenfunc)SequenceLongObject_sq_length,
        .sq_concat = (binaryfunc)SequenceLongObject_sq_concat,
        .sq_repeat = (ssizeargfunc)SequenceLongObject_sq_repeat,
        .sq_item = (ssizeargfunc)SequenceLongObject_sq_item,
        .sq_ass_item = (ssizeobjargproc)SequenceLongObject_sq_ass_item,
        .sq_contains = (objobjproc)SequenceLongObject_sq_contains,
        .sq_inplace_concat = (binaryfunc)NULL,
        .sq_inplace_repeat = (ssizeargfunc)NULL,
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
//        .tp_iter = NULL,
//        .tp_iternext = NULL,
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
