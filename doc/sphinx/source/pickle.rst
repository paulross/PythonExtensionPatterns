.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

====================================
Pickling and C Extensions
====================================

If you need to provide support for pickling your specialised types from your C extension then you need to implement some special functions.

This example shows you how to provided pickle support for for the ``custom2`` type described in the C extension tutorial in the
`Python documentation <https://docs.python.org/3/extending/newtypes_tutorial.html#adding-data-and-methods-to-the-basic-example>`_.

Pickle Version Control
-------------------------------

Since the whole point of ``pickle`` is persistence then pickled objects can hang around in databases, file systems, data from the `shelve <https://docs.python.org/3/library/shelve.html#module-shelve>`_ module and whatnot for a long time.
It is entirely possible that when un-pickled, sometime in the future, that your C extension has moved on and then things become awkward.

It is *strongly* recommended that you add some form of version control to your pickled objects.
In this example I just have a single integer version number which I write to the pickled object.
If the number does not match on unpickling then I raise an exception.
When I change the type API I would, judiciously, change this version number.

Clearly more sophisticated strategies are possible by supporting older versions of the pickled object in some way but this will do for now.

We add some simple pickle version information to the C extension:

.. code-block:: c


    static const char* PICKLE_VERSION_KEY = "_pickle_version";
    static int PICKLE_VERSION = 1;

Now we can implement ``__getstate__`` and ``__setstate__``, think of these as symmetric operations. First ``__getstate__``.

Implementing ``__getstate__``
---------------------------------

``__getstate__`` pickles the object.
``__getstate__`` is expected to return a dictionary of the internal state of the ``Custom`` object.
Note that a ``Custom`` object has two Python objects (``first`` and ``last``) and a C integer (``number``) that need to be converted to a Python object.
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
        Py_DECREF(self->first);
        self->first = PyDict_GetItemString(state, "first"); /* Borrowed reference. */
        if (self->first == NULL) {
            /* PyDict_GetItemString does not set any error state so we have to. */
            PyErr_SetString(PyExc_KeyError, "No \"first\" in pickled dict.");
            return NULL;
        }
        /* Increment the borrowed reference for our instance of it. */
        Py_INCREF(self->first);

Set the ``last`` Member
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

        /* Similar to self->first above. */
        Py_DECREF(self->last);
        self->last = PyDict_GetItemString(state, "last"); /* Borrowed reference. */
        if (self->last == NULL) {
            /* PyDict_GetItemString does not set any error state so we have to. */
            PyErr_SetString(PyExc_KeyError, "No \"last\" in pickled dict.");
            return NULL;
        }
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
        self->number = (int) PyLong_AsLong(number);

And we are done.

.. code-block:: c

        Py_RETURN_NONE;
    }

``__setstate__`` in Full
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
        int pickle_version = (int) PyLong_AsLong(temp);
        if (pickle_version != PICKLE_VERSION) {
            PyErr_Format(PyExc_ValueError,
                         "Pickle version mismatch. Got version %d but expected version %d.",
                         pickle_version, PICKLE_VERSION);
            return NULL;
        }

        /* NOTE: Custom_new() will have been invoked so self->first and self->last
         * will have been allocated so we have to de-allocate them. */
        Py_DECREF(self->first);
        self->first = PyDict_GetItemString(state, "first"); /* Borrowed reference. */
        if (self->first == NULL) {
            /* PyDict_GetItemString does not set any error state so we have to. */
            PyErr_SetString(PyExc_KeyError, "No \"first\" in pickled dict.");
            return NULL;
        }
        /* Increment the borrowed reference for our instance of it. */
        Py_INCREF(self->first);

        /* Similar to self->first above. */
        Py_DECREF(self->last);
        self->last = PyDict_GetItemString(state, "last"); /* Borrowed reference. */
        if (self->last == NULL) {
            /* PyDict_GetItemString does not set any error state so we have to. */
            PyErr_SetString(PyExc_KeyError, "No \"last\" in pickled dict.");
            return NULL;
        }
        Py_INCREF(self->last);

        /* Borrowed reference but no need to incref as we create a C long from it. */
        PyObject *number = PyDict_GetItemString(state, "number");
        if (number == NULL) {
            /* PyDict_GetItemString does not set any error state so we have to. */
            PyErr_SetString(PyExc_KeyError, "No \"number\" in pickled dict.");
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
        {"name", (PyCFunction) Custom_name, METH_NOARGS,
                "Return the name, combining the first and last name"
        },
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

We can test this with code like this that pickles one ``custom2.Custom`` object then creates another ``custom2.Custom`` object from that pickle.
            {"name", (PyCFunction) Custom_name, METH_NOARGS,
                    "Return the name, combining the first and last name"
            },
            {"__getstate__", (PyCFunction) Custom___getstate__, METH_NOARGS,
                    "Pickle the Custom object"
            },
            {"__setstate__", (PyCFunction) Custom___setstate__, METH_O,
                    "Un-pickle the Custom object"
            },
            {NULL}  /* Sentinel */
    };

Example of Using ``custom2.Custom``
-------------------------------------

We can test this with code like this that pickles one object then creates another object from that pickle.
Here is some Python code that exercises our module:

.. code-block:: python

    import pickle

    import custom2

    original = custom2.Custom('FIRST', 'LAST', 11)
    print(
        f'original is {original} @ 0x{id(original):x} first: {original.first} last: {original.last}'
        ' number: {original.number} name: {original.name()}'
    )
    pickled_value = pickle.dumps(original)
    print(f'Pickled original is {pickled_value}')
    result = pickle.loads(pickled_value)
    print(
        f'result is {result} @ 0x{id(result):x} first: {result.first} last: {result.last}'
        ' number: {result.number} name: {result.name()}'
    )


.. code-block:: sh

    $ python main.py
    original is <custom2.Custom object at 0x1049e6810> @ 0x1049e6810
    original first: FIRST last: LAST number: 11 name: FIRST LAST
    Pickled original is b'\x80\x04\x95[\x00\x00\x00\x00\x00\x00\x00\x8c\x07custom2\x94\x8c\x06Custom\x94\x93\x94)\x81\x94}\x94(\x8c\x05first\x94\x8c\x05FIRST\x94\x8c\x04last\x94\x8c\x04LAST\x94\x8c\x06number\x94K\x0b\x8c\x0f_pickle_version\x94K\x01ub.'
    result is <custom2.Custom object at 0x1049252d0> @ 0x1049252d0
    result first: FIRST last: LAST number: 11 name: FIRST LAST

So we have pickled one object and recreated a different, but equivalent, instance from the pickle of the original object which is what we set out to do.
    original is <custom2.Custom object at 0x102b00810> @ 0x102b00810 first: FIRST last: LAST number: 11 name: FIRST LAST
    Pickled original is b'\x80\x04\x95[\x00\x00\x00\x00\x00\x00\x00\x8c\x07custom2\x94\x8c\x06Custom\x94\x93\x94)\x81\x94}\x94(\x8c\x05first\x94\x8c\x05FIRST\x94\x8c\x04last\x94\x8c\x04LAST\x94\x8c\x06number\x94K\x0b\x8c\x0f_pickle_version\x94K\x01ub.'
    result is <custom2.Custom object at 0x102a3f510> @ 0x102a3f510 first: FIRST last: LAST number: 11 name: FIRST LAST

So we have pickled one object and recreated a different, but equivalent, instance from that object.

Pickling Objects with External State
-----------------------------------------

This is just a simple example, if your object relies on external state such as open files, databases and the like you need to be careful, and knowledgeable about your state management.
There is some useful information here: `Handling Stateful Objects <https://docs.python.org/3/library/pickle.html#pickle-state>`_

References
-----------------------

* Python API documentation for `__setstate__ <https://docs.python.org/3/library/pickle.html#object.__setstate__>`_
* Python API documentation for `__getstate__ <https://docs.python.org/3/library/pickle.html#object.__getstate__>`_
* Useful documentation for `Handling Stateful Objects <https://docs.python.org/3/library/pickle.html#pickle-state>`_
* Python `pickle module <https://docs.python.org/3/library/pickle.html>`_
* Python `shelve module <https://docs.python.org/3/library/shelve.html>`_

