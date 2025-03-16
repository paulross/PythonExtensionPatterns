.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _chapter_iterators_generators:

..
    Links, mostly to the Python documentation.

.. _Generator: https://docs.python.org/3/glossary.html#term-generator


***************************
Iterators and Generators
***************************

This chapter describes how to write iterators for your C objects.
These iterators allow your objects to be used with a `Generator`_.

.. index::
    single: Iterators

===========================
Iterators
===========================

The iterator concept is actually fairly straight forward:

- You have some object that contains some data.
- You have some iterator that traverses the data.

That iterator:

- Has a strong reference to the originating object, thus its data.
  This strong reference keeps the originating object alive as long as the iterator is alive.
- It has a notion of *state*, in other words 'where I was before so know where to go next'.

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

    Because of the entwined nature of the sequence object and the iterator object the code in
    ``src/cpy/Iterators/cIterator.c`` occasionally appears out of order.

Firstly, here is the C declaration of the ``SequenceOfLong`` struct:

.. code-block:: c

    #define PY_SSIZE_T_CLEAN

    #include <Python.h>
    #include "structmember.h"

    typedef struct {
        PyObject_HEAD
        long *array_long;
        size_t size;
    } SequenceOfLong;

This will be instantiated with a Python sequence of integers:

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

And initialised with a Python sequence of integers:

.. note::

    The use of the `Sequence Protocol <https://docs.python.org/3/c-api/sequence.html>`_ API such as
    `PySequence_Length() <https://docs.python.org/3/c-api/sequence.html#c.PySequence_Length>`_
    and `PySequence_GetItem() <https://docs.python.org/3/c-api/sequence.html#c.PySequence_GetItem>`_

.. code-block:: c

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

This will be used thus:

.. code-block:: python

    from cPyExtPatt.Iterators import cIterator

    sequence = cIterator.SequenceOfLong([1, 7, 4])

But we can't (yet) iterate across the sequence.
To do that we need to add an iterator.

.. index::
    single: Iterators; Adding an Iterator

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

Here is the initialisation function, note the line ``Py_INCREF(sequence);`` that keeps the original sequence alive.

.. code-block:: c

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

Here is the de-allocation function once the iterator is deleted.
Note the line ``Py_XDECREF(self->sequence);`` that allows the original sequence to be free'd.

.. code-block:: c

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
  The use of ``PyObject_SelfIter`` merely says "I am an iterator".
- ``tp_iternext`` is the function to call with the iterator as its sole argument
  and this returns the next item in the sequence.

.. note::

    `PyObject_SelfIter() <https://docs.python.org/3/c-api/object.html#c.PyObject_SelfIter>`_
    is a supported CPython API and is implemented thus:

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

.. index::
    single: Iterators; Module Initialisation

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
not exposed from the module.

.. index::
    single: Iterators; Iterating Python In C

------------------------------
Iterating a Python Object in C
------------------------------

In ``src/cpy/Iterators/cIterator.c`` there is an example of iterating a
Python object using the
`Iterator Protocol <https://docs.python.org/3/c-api/iter.html>`_.
There is a function ``iterate_and_print`` that takes an object supporting
the Iterator Protocol and iterates across it printing out each item.

The equivalent Python code is:

.. code-block:: python

    def iterate_and_print(sequence: typing.Iterable) -> None:
        print('iterate_and_print:')
        for i, item in enumerate(sequence):
            print(f'[{i}]: {item}')
        print('iterate_and_print: DONE')

The C code looks like this:

.. code-block:: c

    static PyObject *
    iterate_and_print(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
        assert(!PyErr_Occurred());
        static char *kwlist[] = {"sequence", NULL};
        PyObject *sequence = NULL;

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &sequence)) {
            return NULL;
        }
        PyObject *iterator = PyObject_GetIter(sequence);
        if (iterator == NULL) {
            /* propagate error */
            assert(PyErr_Occurred());
            return NULL;
        }
        PyObject *item = NULL;
        long index = 0;
        fprintf(stdout, "%s:\n", __FUNCTION__ );
        while ((item = PyIter_Next(iterator))) {
            /* do something with item */
            fprintf(stdout, "[%ld]: ", index);
            if (PyObject_Print(item, stdout, Py_PRINT_RAW) == -1) {
                /* Handle error. */
                Py_DECREF(item);
                Py_DECREF(iterator);
                if (!PyErr_Occurred()) {
                    PyErr_Format(PyExc_RuntimeError,
                                 "Can not print an item of type %s",
                                 Py_TYPE(sequence)->tp_name);
                }
                return NULL;
            }
            fprintf(stdout, "\n");
            ++index;
            /* release reference when done */
            Py_DECREF(item);
        }
        Py_DECREF(iterator);
        if (PyErr_Occurred()) {
            /* propagate error */
            return NULL;
        }
        fprintf(stdout, "%s: DONE\n", __FUNCTION__ );
        assert(!PyErr_Occurred());
        Py_RETURN_NONE;
    }

This function is added to the cIterator module thus:

.. code-block:: c

    static PyMethodDef cIterator_methods[] = {
            {"iterate_and_print", (PyCFunction) iterate_and_print, METH_VARARGS,
             "Iteratee through the argument printing the values."},
            {NULL, NULL, 0, NULL} /* Sentinel */
    };

    static PyModuleDef iterator_cmodule = {
            PyModuleDef_HEAD_INIT,
            .m_name = "cIterator",
            .m_doc = (
                    "Example module that creates an extension type"
                    "that has forward and reverse iterators."
            ),
            .m_size = -1,
            .m_methods = cIterator_methods,
    };

An example of using this is shown below.

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

Using the builtin ``next()``:

.. code-block:: python

    iterator = iter(sequence)
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4
    # next() will raise a StopIteration.

Using the builtin ``sorted()``:

.. code-block:: python

    sequence = cIterator.SequenceOfLong([1, 7, 4])
    result = sorted(sequence)
    assert result == [1, 4, 7, ]

And to iterate across a Python object:

.. code-block:: python

    cIterator.iterate_and_print('abc')

Result in the stdout:

.. code-block:: text

    iterate_and_print:
    [0]: a
    [1]: b
    [2]: c
    iterate_and_print: DONE

.. note::

    If you are running under pytest you can capture the output to
    stdout from C using the ``capfd`` fixture:


    .. code-block:: python

        import pytest

        @pytest.mark.parametrize(
            'arg, expected',
            (
                    (
                            'abc',
                            """iterate_and_print:
        [0]: a
        [1]: b
        [2]: c
        iterate_and_print: DONE
        """
                    ),
            )
        )
        def test_iterate_and_print(arg, expected, capfd):
            cIterator.iterate_and_print(arg)
            captured = capfd.readouterr()
            assert captured.out == expected


.. _PySequenceMethods: https://docs.python.org/3/c-api/typeobj.html#c.PySequenceMethods
.. _reversed(): https://docs.python.org/3/library/functions.html#reversed
.. _\__reversed__(): https://docs.python.org/3/reference/datamodel.html#object.__reversed__
.. _\__len__(): https://docs.python.org/3/reference/datamodel.html#object.__len__
.. _\__getitem__(): https://docs.python.org/3/reference/datamodel.html#object.__getitem__

.. index::
    single: Reverse Iterators
    single: Iterators; Reverse
    single: PySequenceMethods
    single: __reversed__()
    single: __len__()
    single: __getitem__()

-----------------------------------------
Reverse Iterators
-----------------------------------------

If we try and use the `reversed()`_ function on our current ``SequenceOfLong`` we will get an error:

.. code-block:: text

    TypeError: 'SequenceOfLong' object is not reversible

Reverse iterators are slightly unusual, the `reversed()`_ function calls the object `__reversed__()`_ method if
available or falls back on using `__len__()`_ and `__getitem__()`_.

`reversed()`_ acts like this python code:

.. code-block:: python

    def reversed(obj: object):
        if hasattr(obj, '__reversed__'):
            yield from obj.__reversed__()
        elif hasattr(obj, '__len__') and hasattr(obj, '__getitem__'):
            i = len(obj) - 1
            while i >= 0:
                yield obj[i]
                i -= 1
        else:
            raise TypeError(f'{type(object)} is not reversible')


To support this in C we can implement `__len__()`_ and `__getitem__()`_ using the `PySequenceMethods`_ method table.

First the implementation of `__len__()`_ in C:

.. code-block:: c

    static Py_ssize_t
    SequenceOfLong_sq_length(PyObject *self) {
        return ((SequenceOfLong *)self)->size;
    }

Then the implementation of `__getitem__()`_, note here that we support negative indexes and set and exception if the
index is out of range:

.. code-block:: c

    static PyObject *
    SequenceOfLong_sq_item(PyObject *self, Py_ssize_t index) {
        Py_ssize_t my_index = index;
        if (my_index < 0) {
            my_index += SequenceOfLong_sq_length(self);
        }
        if (my_index > SequenceOfLong_sq_length(self)) {
            PyErr_Format(
                PyExc_IndexError,
                "Index %ld is out of range for length %ld",
                index,
                SequenceOfLong_sq_length(self)
            );
            return NULL;
        }
        return PyLong_FromLong(((SequenceOfLong *)self)->array_long[my_index]);
    }

Create `PySequenceMethods`_ method table:

.. code-block:: c

    PySequenceMethods SequenceOfLong_sequence_methods = {
        .sq_length = &SequenceOfLong_sq_length,
        .sq_item = &SequenceOfLong_sq_item,
    };

Add this method table into the type specification:

.. code-block:: c

    static PyTypeObject SequenceOfLongType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        /* Other stuff. */
        .tp_dealloc = (destructor) SequenceOfLong_dealloc,
        .tp_as_sequence = &SequenceOfLong_sequence_methods,
        /* Other stuff. */
    };

And we can test it thus:

.. code-block:: python

    def test_c_iterator_reversed():
        sequence = cIterator.SequenceOfLong([1, 7, 4])
        result = reversed(sequence)
        assert list(result) == [4, 7, 1,]


.. index::
    single: Generators

===========================
Generators
===========================

Iterators are a requirement for `Generators <https://docs.python.org/3/glossary.html#term-generator>`_,
the secret weapon in Pythons toolbox.
If you don't believe me then ask David Beazley who has done some very fine and informative
`presentations on Generators <https://www.dabeaz.com/generators/>`_


---------------------------------
Our Iterator as a Generator
---------------------------------

Now we have an iterator we can write generator:

.. code-block:: python

    def yield_from_an_iterator_times_two(iterator):
        for value in iterator:
            yield 2 * value

And test it:

.. code-block:: python

    def test_c_iterator_yield_forward():
        sequence = cIterator.SequenceOfLong([1, 7, 4])
        iterator = iter(sequence)
        result = []
        for v in yield_from_an_iterator_times_two(iterator):
            result.append(v)
        assert result == [2, 14, 8]

And create a `generator expression <https://docs.python.org/3/glossary.html#term-generator-expression>`_:

.. code-block:: python

    sequence = cIterator.SequenceOfLong([1, 7, 4])
    result = sum(v * 4 for v in sequence)
    assert result == 4 * (1 + 7 + 4)
