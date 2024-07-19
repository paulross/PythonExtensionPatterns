//
// Created by Paul Ross on 08/07/2021.
//
// Example of a class ('Generator') that has generator member functions.
//
#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "structmember.h"

typedef struct {
    PyObject_HEAD
    long *array_long;
    ssize_t size;
} SequenceOfLong;

// Forward reference
int is_generator_type(PyObject *op);

typedef struct {
    PyObject_HEAD
    PyObject *generator;
    size_t index;
    int forward;
} GeneratorIterator;

static void
GeneratorIterator_dealloc(GeneratorIterator *self) {
    Py_XDECREF(self->generator);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
GeneratorIterator_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    GeneratorIterator *self;
    self = (GeneratorIterator *) type->tp_alloc(type, 0);
    if (self != NULL) {
    }
    return (PyObject *) self;
}

static int
GeneratorIterator_init(GeneratorIterator *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"generator", "forward", NULL};
    PyObject *generator = NULL;
    int forward = 1; // Default is forward.

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|p", kwlist, &generator, &forward)) {
        return -1;
    }
    if (!is_generator_type(generator)) {
        PyErr_Format(PyExc_ValueError, "Argument must be a GeneratorType, not a %s", Py_TYPE(generator)->tp_name);
    }
    Py_INCREF(generator);
    self->generator = generator;
    self->index = 0;
    self->forward = forward;
    return 0;
}

static PyObject *
GeneratorIterator_next(GeneratorIterator *self) {
    size_t size = ((SequenceOfLong *) self->generator)->size;
    if (self->index < size) {
        size_t index;
        if (self->forward) {
            index = self->index;
        } else {
            index = size - self->index - 1;
        }
        self->index += 1;
        PyObject *ret = PyLong_FromLong(((SequenceOfLong *) self->generator)->array_long[index]);
        return ret;
    }
    // End iteration.
    Py_CLEAR(self->generator);
    return NULL;
}

static PyMemberDef GeneratorIterator_members[] = {
        {NULL, 0, 0, 0, NULL}  /* Sentinel */
};

static PyMethodDef GeneratorIterator_methods[] = {
        {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyObject *
GeneratorIterator___str__(GeneratorIterator *self, PyObject *Py_UNUSED(ignored)) {
    assert(!PyErr_Occurred());
    if (self->generator) {
        return PyUnicode_FromFormat(
                "<SequenceOfLong generator @: %p of size %ld index %ld",
                self->generator, ((SequenceOfLong *) self->generator)->size, self->index
        );
    } else {
        return PyUnicode_FromFormat(
                "<SequenceOfLong generator @: %p of size NULL generator (exhausted) index %ld",
                self->generator, self->index
        );
    }
}

static PyTypeObject GeneratorIteratorType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "GeneratorIterator",
        .tp_basicsize = sizeof(GeneratorIterator),
        .tp_itemsize = 0,
        .tp_dealloc = (destructor) GeneratorIterator_dealloc,
        .tp_str = (reprfunc) GeneratorIterator___str__,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = "GeneratorIterator object.",
        .tp_iter = PyObject_SelfIter,
        .tp_iternext = (iternextfunc) GeneratorIterator_next,
        .tp_methods = GeneratorIterator_methods,
        .tp_members = GeneratorIterator_members,
        .tp_init = (initproc) GeneratorIterator_init,
        .tp_new = GeneratorIterator_new,
};

static void
Generator_dealloc(SequenceOfLong *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
Generator_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    SequenceOfLong *self;
    self = (SequenceOfLong *) type->tp_alloc(type, 0);
    if (self != NULL) {
    }
    return (PyObject *) self;
}

static int
Generator_init(SequenceOfLong *self, PyObject *args, PyObject *kwds) {
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
            Py_DECREF(py_value);
            return -4;
        }
    }
    return 0;
}

static PyMemberDef Generator_members[] = {
        {NULL, 0, 0, 0, NULL}  /* Sentinel */
};

static PyObject *
Generator_size(SequenceOfLong *self, PyObject *Py_UNUSED(ignored)) {
    return Py_BuildValue("n", self->size);
}

static PyObject *
Generator_iter_forward(SequenceOfLong *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *ret = GeneratorIterator_new(&GeneratorIteratorType, NULL, NULL);
    if (ret) {
        PyObject *args = Py_BuildValue("OO", self, Py_True);
        if (!args || GeneratorIterator_init((GeneratorIterator *) ret, args, NULL)) {
            Py_DECREF(ret);
            ret = NULL;
        }
        Py_DECREF(args);
    }
    return ret;
}

static PyObject *
Generator_iter_reverse(SequenceOfLong *self, PyObject *Py_UNUSED(ignored)) {
    PyObject *ret = GeneratorIterator_new(&GeneratorIteratorType, NULL, NULL);
    if (ret) {
        PyObject *args = Py_BuildValue("OO", self, Py_False);
        if (!args || GeneratorIterator_init((GeneratorIterator *) ret, args, NULL)) {
            Py_DECREF(ret);
            ret = NULL;
        }
        Py_DECREF(args);
    }
    return ret;
}

static PyMethodDef Generator_methods[] = {
        {
            "size",
            (PyCFunction) Generator_size,
            METH_NOARGS,
            "Return the size of the sequence."
        },
        {
            "iter_forward",
            (PyCFunction) Generator_iter_forward,
            METH_NOARGS,
            "Forward iterator across the sequence."
        },
        {
            "iter_reverse",
            (PyCFunction) Generator_iter_reverse,
            METH_NOARGS,"Reverse iterator across the sequence."
        },
        {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyObject *
Generator___str__(SequenceOfLong *self, PyObject *Py_UNUSED(ignored)) {
    assert(!PyErr_Occurred());

    return PyUnicode_FromFormat("<SequenceOfLong sequence size: %ld>", self->size);
}

static PyTypeObject GeneratorType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "Generator",
        .tp_basicsize = sizeof(SequenceOfLong),
        .tp_itemsize = 0,
        .tp_dealloc = (destructor) Generator_dealloc,
        .tp_str = (reprfunc) Generator___str__,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = "Generator objects",
        .tp_methods = Generator_methods,
        .tp_members = Generator_members,
        .tp_init = (initproc) Generator_init,
        .tp_new = Generator_new,
};

int is_generator_type(PyObject *op) {
    return Py_TYPE(op) == &GeneratorType;
}

static PyModuleDef gen_cmodule = {
        PyModuleDef_HEAD_INIT,
        .m_name = "gen_c",
        .m_doc = "Example module that creates an extension type that has generators.",
        .m_size = -1,
};


PyMODINIT_FUNC
PyInit_gen_c(void) {
    PyObject *m;
    m = PyModule_Create(&gen_cmodule);
    if (m == NULL) {
        return NULL;
    }

    if (PyType_Ready(&GeneratorType) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(&GeneratorType);
    if (PyModule_AddObject(m, "Generator", (PyObject *) &GeneratorType) < 0) {
        Py_DECREF(&GeneratorType);
        Py_DECREF(m);
        return NULL;
    }

    if (PyType_Ready(&GeneratorIteratorType) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(&GeneratorIteratorType);
    if (PyModule_AddObject(m, "GeneratorIterator", (PyObject *) &GeneratorIteratorType) < 0) {
        Py_DECREF(&GeneratorType);
        Py_DECREF(&GeneratorIteratorType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
