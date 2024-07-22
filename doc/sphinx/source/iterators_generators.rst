.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _chapter_iterators_generators:

***************************
Iterators and Generators
***************************

This chapter describes how to write iterators on your C objects.
These iterators allow your objects to be used with
`Generators <https://docs.python.org/3/glossary.html#term-generator>`_.

===========================
Iterators
===========================

The iterator concept is actually fairly straight forward:

- You have some object that contains some data.
- You have some iterator that traverses the data.

That iterator:

- Has a strong reference to the originating object, thus its data.
  This strong reference keeps the originating object alive as long as the iterator is alive.
- It has a notion of *state*, in other words 'where I was before and where I go next'.

.. warning::

    The strong reference to the underlying data structure keeps it alive but what happens if the underlying structure
    is altered *during* iteration?

    Here is an example:

    .. code-block:: python

        lst = list(range(8))
        for i, value in enumerate(lst):
            print(f'i={i} value={value}')
            del lst[i]

    This gives the sequence:

    .. code-block:: bash

        i=0 value=0
        i=1 value=2
        i=2 value=4
        i=3 value=6

    Which may not be what you want.
    It is hard to make a 'good' design to cope with this (defer the ``del``? raise?) so the general advice is: do not
    alter the underlying structure whilst iterating.

--------------------------------------
Example of a Sequence
--------------------------------------

In this example we create a sequence of C ``long`` s.
The complete code is in ``src/cpy/Iterators/cIterator.c`` here are the essential parts.
The test code is in ``tests/unit/test_c_iterators.py``:

.. code-block:: c

    #define PY_SSIZE_T_CLEAN

    #include <Python.h>
    #include "structmember.h"

    typedef struct {
        PyObject_HEAD
        long *array_long;
        ssize_t size;
    } SequenceOfLong;

This will be initialised with a Python sequence of integers:

.. code-block:: c

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

And the de-allocation function frees the dynamically allocated memory:

.. code-block:: c

    static void
    SequenceOfLong_dealloc(SequenceOfLong *self) {
        free(self->array_long);
        Py_TYPE(self)->tp_free((PyObject *) self);
    }

We provide a single method ``size()`` for the length of the sequence and a ``__str__`` method:

.. code-block:: c

    SequenceOfLong_size(SequenceOfLong *self, PyObject *Py_UNUSED(ignored)) {
        return Py_BuildValue("n", self->size);
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

The type declaration then becomes:

.. code-block:: c

    static PyTypeObject SequenceOfLongType= {
            PyVarObject_HEAD_INIT(NULL, 0)
            .tp_name = "SequenceOfLong",
            .tp_basicsize = sizeof(SequenceOfLong),
            .tp_itemsize = 0,
            .tp_dealloc = (destructor) SequenceOfLong_dealloc,
            .tp_str = (reprfunc) SequenceOfLong___str__,
            .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
            .tp_doc = "Sequence of long integers.",
            .tp_methods = SequenceOfLong_methods,
            .tp_init = (initproc) SequenceOfLong_init,
            .tp_new = SequenceOfLong_new,
    };

--------------------------------------
Adding an Iterator
--------------------------------------




===========================
Generators
===========================

Iterators are a very powerful requirement for `Generators <https://docs.python.org/3/glossary.html#term-generator>`_,
the secret weapon in Pythons toolbox.
If you don't believe me then ask David Beazley who has done some very fine and informative presentations on
`Generators <https://www.dabeaz.com/generators/>`_

