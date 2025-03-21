.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

.. index::
    single: Pickling

====================================
Pickling C Extension Types
====================================

If you need to provide support for pickling your specialised types from your C extension then you need to implement
some special functions.

This example shows you how to provided pickle support for for the ``custom2.Custom`` type described in the C extension
tutorial in the
`Python documentation <https://docs.python.org/3/extending/newtypes_tutorial.html#adding-data-and-methods-to-the-basic-example>`_.
This defines an ``CustomObject`` object that has three fields; a first name, a last name  and a number.
The ``CustomObject`` definition that needs to be pickled and un-pickled looks like this in C:

.. code-block:: c

    typedef struct {
        PyObject_HEAD
        PyObject *first; /* first name */
        PyObject *last;  /* last name */
        int number;
    } CustomObject;

- The example C code is in ``src/cpy/Pickle/cCustomPickle.c``.
- The test code is in ``tests/unit/test_c_custom_pickle.py``.

.. index::
    single: Pickling; Version Control

Pickle Version Control
-------------------------------

Since the whole point of ``pickle`` is persistence then pickled objects can hang around in databases, file systems,
data from the `shelve <https://docs.python.org/3/library/shelve.html#module-shelve>`_ module and whatnot for a long
time.
It is entirely possible that when un-pickled, sometime in the future, that your C extension has moved on and then
things become awkward.

It is *strongly* recommended that you add some form of version control to your pickled objects.
In this example I just have a single integer version number which I write to the pickled object.
If the number does not match on unpickling then I raise an exception.
When I change the type API I would, judiciously, change this version number.

Clearly more sophisticated strategies are possible by supporting older versions of the pickled object in some way but
this will do for now.

We add some simple pickle version information to the C extension:

.. code-block:: c


    static const char* PICKLE_VERSION_KEY = "_pickle_version";
    static int PICKLE_VERSION = 1;

Now we can implement ``__getstate__`` and ``__setstate__``, think of these as symmetric operations.

First ``__getstate__``.

.. index::
    single: Pickling; __getstate__

Implementing  ``__getstate__``
---------------------------------

``__getstate__`` pickles the object.
``__getstate__`` is expected to return a dictionary of the internal state of the ``Custom`` object.
Note that a ``Custom`` object has two Python objects (``first`` and ``last``) and a C integer (``number``) that need to
be converted to a Python object.
We also need to add the version information.

Here is the C implementation:

.. code-block:: c

    /* Pickle the object */
    static PyObject *
    Custom___getstate__(CustomObject *self, PyObject *Py_UNUSED(ignored)) {
        PyObject *ret = Py_BuildValue("{sOsOsisi}",
                                      "first", self->first,
                                      "last", self->last,
                                      "number", self->number,
                                      PICKLE_VERSION_KEY, PICKLE_VERSION);
        return ret;
    }

.. note::

    Note the careful use of ``Py_BuildValue()``.
    ``"s"`` and ``"i"`` causes a new PyObject to be created in the dict.
    ``"O"`` increments the reference count of an existing PyObject which is inserted into the dict.

    See :ref:`chapter_refcount.stolen.warning_pydict_setitem`

.. index::
    single: Pickling; __setstate__

Implementing ``__setstate__``
---------------------------------

The implementation of ``__setstate__`` un-pickles the object.
This is a little more complicated as there is quite a lot of error checking going on.
We are being passed an arbitrary Python object and need to check:

* It is a Python dictionary.
* It has a version key and the version value is one that we can deal with.
* It has the required keys and values to populate our ``Custom`` object.

Note that our ``__new__`` method (``Custom_new()``) has already been called on ``self``.
Before setting any member value we need to de-allocate the existing value set by ``Custom_new()`` otherwise we will have a memory leak.

.. index::
    single: Pickling; __setstate__
    single: Pickling; __setstate__ Error Checking

Error Checking
^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

    /* Un-pickle the object */
    static PyObject *
    Custom___setstate__(CustomObject *self, PyObject *state) {
        /* Error check. */
        if (!PyDict_CheckExact(state)) {
            PyErr_SetString(PyExc_ValueError, "Pickled object is not a dict.");
            return NULL;
        }
        /* Version check. */
        /* Borrowed reference but no need to increment as we create a C long
         * from it. */
        PyObject *temp = PyDict_GetItemString(state, PICKLE_VERSION_KEY);
        if (temp == NULL) {
            /* PyDict_GetItemString does not set any error state so we have to. */
            PyErr_Format(PyExc_KeyError, "No \"%s\" in pickled dict.",
                         PICKLE_VERSION_KEY);
            return NULL;
        }
        if (!PyLong_Check(temp)) {
            PyErr_Format(
                PyExc_KeyError,
                "Pickled dict version key \"%s\" is not a long but type \"%s\".",
                PICKLE_VERSION_KEY,
                temp->ob_type->tp_name
            );
            return NULL;
        }
        int pickle_version = (int) PyLong_AsLong(temp);
        if (pickle_version != PICKLE_VERSION) {
            PyErr_Format(PyExc_ValueError,
                         "Pickle version mismatch. Got version %d but expected version %d.",
                         pickle_version, PICKLE_VERSION);
            return NULL;
        }

Set the ``first`` Member
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

    /* NOTE: Custom_new() will have been invoked so self->first and self->last
     * will have been allocated so we have to de-allocate them. */
    temp = PyDict_GetItemString(state, "first"); /* Borrowed reference. */
    if (temp == NULL) {
        /* PyDict_GetItemString does not set any error state so we have to. */
        PyErr_SetString(PyExc_KeyError, "No \"first\" in pickled dict.");
        return NULL;
    }
    if (!PyUnicode_Check(temp)) {
        PyErr_Format(
                PyExc_KeyError,
                "Pickled dict key \"first\" is not a str but type \"%s\".",
                temp->ob_type->tp_name
        );
        return NULL;
    }
    Py_DECREF(self->first);
    self->first = temp;
    Py_INCREF(self->first);

Set the ``last`` Member
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This code is very similar to the code for the first member above.

.. code-block:: c

    temp = PyDict_GetItemString(state, "last"); /* Borrowed reference. */
    if (temp == NULL) {
        /* PyDict_GetItemString does not set any error state so we have to. */
        PyErr_SetString(PyExc_KeyError, "No \"last\" in pickled dict.");
        return NULL;
    }
    if (!PyUnicode_Check(temp)) {
        PyErr_Format(
                PyExc_KeyError,
                "Pickled dict key \"last\" is not a str but type \"%s\".",
                temp->ob_type->tp_name
        );
        return NULL;
    }
    Py_DECREF(self->last);
    self->last = temp;
    Py_INCREF(self->last);

Set the ``number`` Member
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This is a C fundamental type so the code is slightly different:

.. code-block:: c

    /* Borrowed reference but no need to incref as we create a C long from it. */
    PyObject *number = PyDict_GetItemString(state, "number");
    if (number == NULL) {
        /* PyDict_GetItemString does not set any error state so we have to. */
        PyErr_SetString(PyExc_KeyError, "No \"number\" in pickled dict.");
        return NULL;
    }
    if (!PyLong_Check(number)) {
        PyErr_Format(
                PyExc_KeyError,
                "Pickled dict key \"number\" is not an int but type \"%s\".",
                number->ob_type->tp_name
        );
        return NULL;
    }
    self->number = (int) PyLong_AsLong(number);

And we are done.

.. code-block:: c

        Py_RETURN_NONE;
    }

``__setstate__`` in Full
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

    static PyObject *
    Custom___setstate__(CustomObject *self, PyObject *state) {

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
        if (!PyLong_Check(temp)) {
            PyErr_Format(
                PyExc_KeyError,
                "Pickled dict version key \"%s\" is not a long but type \"%s\".",
                PICKLE_VERSION_KEY,
                temp->ob_type->tp_name
            );
            return NULL;
        }
        int pickle_version = (int) PyLong_AsLong(temp);
        if (pickle_version != PICKLE_VERSION) {
            PyErr_Format(
                PyExc_ValueError,
                "Pickle version mismatch. Got version %d but expected version %d.",
                pickle_version, PICKLE_VERSION
                );
            return NULL;
        }

        temp = PyDict_GetItemString(state, "first"); /* Borrowed reference. */
        if (temp == NULL) {
            /* PyDict_GetItemString does not set any error state so we have to. */
            PyErr_SetString(PyExc_KeyError, "No \"first\" in pickled dict.");
            return NULL;
        }
        if (!PyUnicode_Check(temp)) {
            PyErr_Format(
                    PyExc_KeyError,
                    "Pickled dict key \"first\" is not a str but type \"%s\".",
                    temp->ob_type->tp_name
            );
            return NULL;
        }
        Py_DECREF(self->first);
        self->first = temp;
        Py_INCREF(self->first);

        temp = PyDict_GetItemString(state, "last"); /* Borrowed reference. */
        if (temp == NULL) {
            /* PyDict_GetItemString does not set any error state so we have to. */
            PyErr_SetString(PyExc_KeyError, "No \"last\" in pickled dict.");
            return NULL;
        }
        if (!PyUnicode_Check(temp)) {
            PyErr_Format(
                    PyExc_KeyError,
                    "Pickled dict key \"last\" is not a str but type \"%s\".",
                    temp->ob_type->tp_name
            );
            return NULL;
        }
        Py_DECREF(self->last);
        self->last = temp;
        Py_INCREF(self->last);

        /* Borrowed reference but no need to increment as we create a C long from it. */
        PyObject *number = PyDict_GetItemString(state, "number");
        if (number == NULL) {
            /* PyDict_GetItemString does not set any error state so we have to. */
            PyErr_SetString(PyExc_KeyError, "No \"number\" in pickled dict.");
            return NULL;
        }
        if (!PyLong_Check(number)) {
            PyErr_Format(
                    PyExc_KeyError,
                    "Pickled dict key \"number\" is not an int but type \"%s\".",
                    number->ob_type->tp_name
            );
            return NULL;
        }
        self->number = (int) PyLong_AsLong(number);

        Py_RETURN_NONE;
    }


Add the Special Methods
---------------------------------

Now we need to add these two special methods to the methods table which now looks like this:

.. code-block:: c

    static PyMethodDef Custom_methods[] = {
        /* Existing methods here... */
        {"__getstate__", (PyCFunction) Custom___getstate__, METH_NOARGS,
                "Pickle the Custom object"
        },
        {"__setstate__", (PyCFunction) Custom___setstate__, METH_O,
                "Un-pickle the Custom object"
        },
        {NULL}  /* Sentinel */
    };

Pickling a ``custom2.Custom`` Object
-------------------------------------

We can test this with code like this that pickles one ``custom2.Custom`` object then creates another ``custom2.Custom``
object from that pickle.
Here is some Python code that exercises our module (tests are in ``tests/unit/test_c_custom_pickle.py``):

.. code-block:: python

    import io
    import pickle
    import pickletools
    import sys

    import pytest

    from cPyExtPatt import cPickle


    def test_module_dir():
        assert dir(cPickle) == [
            'Custom', '__doc__', '__file__', '__loader__',
            '__name__', '__package__', '__spec__',
        ]


    ARGS_FOR_CUSTOM_CLASS = ('FIRST', 'LAST', 11)
    PICKLE_BYTES_FOR_CUSTOM_CLASS = (
        b'\x80\x04\x95f\x00\x00\x00\x00\x00\x00\x00\x8c\x12cPyExtPatt.cPickle\x94'
        b'\x8c\x06Custom\x94\x93\x94)\x81\x94}\x94(\x8c\x05first\x94\x8c\x05FIRST'
        b'\x94\x8c\x04last\x94\x8c\x04LAST\x94\x8c\x06number\x94K\x0b\x8c\x0f_pickle_'
        b'version\x94K\x01ub.'
    )

    def test_pickle_getstate():
        custom = cPickle.Custom(*ARGS_FOR_CUSTOM_CLASS)
        pickled_value = pickle.dumps(custom)
        print()
        print(f'Pickled original is {pickled_value}')
        assert pickled_value == PICKLE_BYTES_FOR_CUSTOM_CLASS
        # result = pickle.loads(pickled_value)


    def test_pickle_setstate():
        custom = pickle.loads(PICKLE_BYTES_FOR_CUSTOM_CLASS)
        assert custom.first == 'FIRST'
        assert custom.last == 'LAST'
        assert custom.number == 11

    def test_pickle_round_trip():
        custom = cPickle.Custom(*ARGS_FOR_CUSTOM_CLASS)
        pickled_value = pickle.dumps(custom)
        result = pickle.loads(pickled_value)
        assert id(result) != id(custom)

So we have pickled one object and recreated a different, but equivalent, instance from the pickle of the original
object which is what we set out to do.

.. index::
    single: Pickling; pickletools

The Pickled Object in Detail
-------------------------------------

If you are curious about the contents of the pickled object the the Python standard library provides the
`pickletools <https://docs.python.org/3/library/pickletools.html#module-pickletools>`_ module.
This allows you to inspect the pickled object.
Here is a test for that:

.. code-block:: python

    import io
    import pickle
    import pickletools
    import sys

    import pytest

    from cPyExtPatt import cPickle

    def test_pickletools():
        outfile = io.StringIO()
        pickletools.dis(PICKLE_BYTES_FOR_CUSTOM_CLASS, out=outfile, annotate=1)
        result = outfile.getvalue()
        expected = '<See below>'
        assert result == expected

The expected output will be something like this:

.. raw:: latex

    \begin{landscape}

.. code-block:: text

    Pickled original is b'\x80\x04\x95[\x00\x00\x00\x00\x00\x00\x00\x8c\x07custom2\x94\x8c\x06Custom\x94\x93\x94)\x81\x94}\x94(\x8c\x05first\x94\x8c\x05FIRST\x94\x8c\x04last\x94\x8c\x04LAST\x94\x8c\x06number\x94K\x0b\x8c\x0f_pickle_version\x94K\x01ub.'
        0: \x80 PROTO      4 Protocol version indicator.
        2: \x95 FRAME      91 Indicate the beginning of a new frame.
       11: \x8c SHORT_BINUNICODE 'custom2' Push a Python Unicode string object.
       20: \x94 MEMOIZE    (as 0)          Store the stack top into the memo.  The stack is not popped.
       21: \x8c SHORT_BINUNICODE 'Custom'  Push a Python Unicode string object.
       29: \x94 MEMOIZE    (as 1)          Store the stack top into the memo.  The stack is not popped.
       30: \x93 STACK_GLOBAL               Push a global object (module.attr) on the stack.
       31: \x94 MEMOIZE    (as 2)          Store the stack top into the memo.  The stack is not popped.
       32: )    EMPTY_TUPLE                Push an empty tuple.
       33: \x81 NEWOBJ                     Build an object instance.
       34: \x94 MEMOIZE    (as 3)          Store the stack top into the memo.  The stack is not popped.
       35: }    EMPTY_DICT                 Push an empty dict.
       36: \x94 MEMOIZE    (as 4)          Store the stack top into the memo.  The stack is not popped.
       37: (    MARK                       Push markobject onto the stack.
       38: \x8c     SHORT_BINUNICODE 'first' Push a Python Unicode string object.
       45: \x94     MEMOIZE    (as 5)        Store the stack top into the memo.  The stack is not popped.
       46: \x8c     SHORT_BINUNICODE 'FIRST' Push a Python Unicode string object.
       53: \x94     MEMOIZE    (as 6)        Store the stack top into the memo.  The stack is not popped.
       54: \x8c     SHORT_BINUNICODE 'last'  Push a Python Unicode string object.
       60: \x94     MEMOIZE    (as 7)        Store the stack top into the memo.  The stack is not popped.
       61: \x8c     SHORT_BINUNICODE 'LAST'  Push a Python Unicode string object.
       67: \x94     MEMOIZE    (as 8)        Store the stack top into the memo.  The stack is not popped.
       68: \x8c     SHORT_BINUNICODE 'number' Push a Python Unicode string object.
       76: \x94     MEMOIZE    (as 9)         Store the stack top into the memo.  The stack is not popped.
       77: K        BININT1    11             Push a one-byte unsigned integer.
       79: \x8c     SHORT_BINUNICODE '_pickle_version' Push a Python Unicode string object.
       96: \x94     MEMOIZE    (as 10)                 Store the stack top into the memo.  The stack is not popped.
       97: K        BININT1    1                       Push a one-byte unsigned integer.
       99: u        SETITEMS   (MARK at 37)            Add an arbitrary number of key+value pairs to an existing dict.
      100: b    BUILD                                  Finish building an object, via __setstate__ or dict update.
      101: .    STOP                                   Stop the unpickling machine.
    highest protocol among opcodes = 4

.. raw:: latex

    \end{landscape}

.. index::
    single: Pickling; External State

Pickling Objects with External State
-----------------------------------------

This is just a simple example, if your object relies on external state such as open files, databases and the like you
need to be careful, and knowledgeable about your state management.
There is some useful information in
`Handling Stateful Objects <https://docs.python.org/3/library/pickle.html#pickle-state>`_


.. note::

    Marshalling support for use with the `marshall <https://docs.python.org/3/library/marshal.html#module-marshal>`_
    module is given by the `C Marshall API <https://docs.python.org/3/c-api/marshal.html>`_

.. index::
    single: Pickling; References

References
-----------------------

* Python API documentation for `__setstate__ <https://docs.python.org/3/library/pickle.html#object.__setstate__>`_
* Python API documentation for `__getstate__ <https://docs.python.org/3/library/pickle.html#object.__getstate__>`_
* Useful documentation for `Handling Stateful Objects <https://docs.python.org/3/library/pickle.html#pickle-state>`_
* Python `pickle module <https://docs.python.org/3/library/pickle.html>`_
* Python `shelve module <https://docs.python.org/3/library/shelve.html>`_
