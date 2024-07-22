.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _chapter_iterators_generators:

***************************
Iterators and Generators
***************************

This chapter describes how to write iterators for your C objects.
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

In this example we create a module that has an object which holds a sequence of C ``long`` s.
The complete code is in ``src/cpy/Iterators/cIterator.c`` here are just the essential parts.
The test code is in ``tests/unit/test_c_iterators.py``.

Essentially in Python this means I will be able do this:

.. code-block:: python

    from cPyExtPatt.Iterators import cIterator

    sequence = cIterator.SequenceOfLong([1, 7, 4])
    result = [v for v in sequence]
    assert result == [1, 7, 4]

.. note::

    Because of the entwined nature of the sequence and the iterator the code in ``src/cpy/Iterators/cIterator.c``
    occasionally appears out of order.

Firstly, here is the C declaration of the ``SequenceOfLong`` struct:

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
    SequenceOfLong_new(PyTypeObject *type, PyObject *Py_UNUSED(args),
                       PyObject *Py_UNUSED(kwds)) {
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

This can be used thus:

.. code-block:: python

    from cPyExtPatt.Iterators import cIterator

    sequence = cIterator.SequenceOfLong([1, 7, 4])

But we can't (yet) iterate across the sequence.
To do that we need to add an iterator.

--------------------------------------
Adding an Iterator
--------------------------------------

Here is the iterator that takes a reference to the ``SequenceOfLong`` and maintains an index:

.. code-block:: c

    typedef struct {
        PyObject_HEAD
        PyObject *sequence;
        size_t index;
    } SequenceOfLongIterator;

Here are the ``__new__``, ``__init__`` and de-allocation methods:

.. code-block:: c

    static PyObject *
    SequenceOfLongIterator_new(PyTypeObject *type, PyObject *Py_UNUSED(args),
                               PyObject *Py_UNUSED(kwds)) {
        SequenceOfLongIterator *self;
        self = (SequenceOfLongIterator *) type->tp_alloc(type, 0);
        if (self != NULL) {
            assert(!PyErr_Occurred());
        }
        return (PyObject *) self;
    }

    // Forward reference
    static int is_sequence_of_long_type(PyObject *op);
    // Defined later to be:
    // static int
    // is_sequence_of_long_type(PyObject *op) {
    //      return Py_TYPE(op) == &SequenceOfLongType;
    // }


    static int
    SequenceOfLongIterator_init(SequenceOfLongIterator *self, PyObject *args,
                                PyObject *kwds) {
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

Here is the ``__next__`` method.
This returns the next object in the sequence, or NULL if the sequence is exhausted.
It updates the internal counter on each call:

.. code-block:: c

    static PyObject *
    SequenceOfLongIterator_next(SequenceOfLongIterator *self) {
        size_t size = ((SequenceOfLong *) self->sequence)->size;
        if (self->index < size) {
            PyObject *ret = PyLong_FromLong(
                ((SequenceOfLong *) self->sequence)->array_long[self->index]
            );
            self->index += 1;
            return ret;
        }
        // End iteration.
        return NULL;
    }

Here is the iterator type declaration, note the use of
`tp_iter <https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_iter>`_
and `tp_iternext <https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_iternext>`_.

- ``tp_iter`` signals that this object is an iterable and its result is the iterator.
  The use of ``PyObject_SelfIter`` merely says "I am the iterator".
- ``tp_iternext`` is the function to call with the iterator as its sole argument
  and this returns the next item in the sequence.

.. note::

    ``PyObject_SelfIter`` is implemented thus:

    .. code-block:: c

        PyObject *
        PyObject_SelfIter(PyObject *obj)
        {
            Py_INCREF(obj);
            return obj;
        }

Here is the type declaration for the iterator, the iterator is both iterable and has a ``__next__`` method
so both ``tp_iter`` and ``tp_iternext`` are defined:

.. code-block:: c

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

Now change the type declaration of the ``SequenceOfLongType`` to add iteration.
This defines ``tp_iter`` as it is iterable but does *not* define the ``tp_iternext`` method as it does *not* have
a ``__next__`` method, the iterator instance provides that:

.. code-block::

    static PyTypeObject SequenceOfLongType= {
            PyVarObject_HEAD_INIT(NULL, 0)
            .tp_name = "SequenceOfLong",
            .tp_basicsize = sizeof(SequenceOfLong),
            .tp_itemsize = 0,
            .tp_dealloc = (destructor) SequenceOfLong_dealloc,
            .tp_str = (reprfunc) SequenceOfLong___str__,
            .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
            .tp_doc = "Sequence of long integers.",
            // I am iterable, this function gives you an iterator on me.
            .tp_iter = (getiterfunc) SequenceOfLong_iter,
            .tp_methods = SequenceOfLong_methods,
            .tp_init = (initproc) SequenceOfLong_init,
            .tp_new = SequenceOfLong_new,
    };

-----------------------------------------
A Note on Module Initialisation
-----------------------------------------

The module initialisation looks like this:

.. code-block:: c

    PyMODINIT_FUNC
    PyInit_cIterator(void) {
        PyObject *m;
        m = PyModule_Create(&iterator_cmodule);
        if (m == NULL) {
            return NULL;
        }
        // ...
    }

Naturally enough we have to include the initialisation of ``SequenceOfLongType``:

.. code-block:: c

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

We *must* include the initialisation of ``SequenceOfLongIteratorType``:

.. code-block:: c

    if (PyType_Ready(&SequenceOfLongIteratorType) < 0) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(&SequenceOfLongIteratorType);

However the following is optional, as the comment suggests:

.. code-block:: c

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

If you omit that the code will work just fine, the iterator is instantiated dynamically, it is just that the type is
not exposed in the module.


------------------------------
Examples
------------------------------

Now we can import the module, and create a sequence:

.. code-block:: python

    from cPyExtPatt.Iterators import cIterator

    sequence = cIterator.SequenceOfLong([1, 7, 4])

And these calls now work:

.. code-block:: python

    result = [v for v in sequence]
    assert result == [1, 7, 4]

Delete the underlying object, the iteration still works:

.. code-block:: python

    iterator = iter(sequence)
    del sequence
    result = [v for v in iterator]
    assert result == [1, 7, 4]

Explicit builtin ``next()``:

.. code-block:: python

    iterator = iter(sequence)
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4
    # next() will raise a StopIteration.

Explicit builtin ``sorted()``:

.. code-block:: python

    sequence = cIterator.SequenceOfLong([1, 7, 4])
    result = sorted(sequence)
    assert result == [1, 4, 7, ]



===========================
Generators
===========================

Iterators are a very powerful requirement for `Generators <https://docs.python.org/3/glossary.html#term-generator>`_,
the secret weapon in Pythons toolbox.
If you don't believe me then ask David Beazley who has done some very fine and informative presentations on
`Generators <https://www.dabeaz.com/generators/>`_

