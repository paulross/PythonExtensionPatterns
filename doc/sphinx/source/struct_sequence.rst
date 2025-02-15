.. highlight:: python
    :linenothreshold: 25

.. toctree::
    :maxdepth: 3

..
    Links, mostly to the Python documentation.
    Specific container links are just before the appropriate section.

.. _namedtuple: https://docs.python.org/3/library/collections.html#collections.namedtuple
.. _namedtuples: https://docs.python.org/3/library/collections.html#collections.namedtuple

.. _Struct Sequence API: https://docs.python.org/3/c-api/tuple.html#struct-sequence-objects
.. _Struct Sequence Object: https://docs.python.org/3/c-api/tuple.html#struct-sequence-objects
.. _Struct Sequence Objects: https://docs.python.org/3/c-api/tuple.html#struct-sequence-objects
.. _PyStructSequence_NewType(): https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_NewType
.. _PyStructSequence_InitType(): https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_InitType
.. _PyStructSequence_InitType2(): https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_InitType2
.. _PyStructSequence_Desc: https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_Desc
.. _PyStructSequence_Field: https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_Field
.. _PyStructSequence_UnnamedField: https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_UnnamedField
.. _PyStructSequence_New(): https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_New
.. _PyStructSequence_GetItem(): https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_GetItem
.. _PyStructSequence_GET_ITEM(): https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_GET_ITEM
.. _PyStructSequence_SetItem(): https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_SetItem
.. _PyStructSequence_SET_ITEM(): https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_SET_ITEM

.. _PyTypeObject: https://docs.python.org/3/c-api/type.html#c.PyTypeObject

.. _chapter_struct_sequence:

.. index:: single: Struct Sequence

==================================================
Struct Sequences (a ``namedtuple`` in C)
==================================================

A `Struct Sequence Object`_ object is, more or less, the C equivalent of Python's `namedtuple`_ type.

As a reminder here is how named tuples work in Python:

.. code-block:: python

    >>> from collections import namedtuple
    >>> nt_type = namedtuple('MyNamedTuple', ['field_one', 'field_two'])
    >>> nt_type
    <class '__main__.MyNamedTuple'>
    >>> nt_type._fields
    ('field_one', 'field_two')
    >>> nt = nt_type(['foo', 'bar'])
    >>> nt.field_one
    'foo'
    >>> nt.index('bar')
    1

The C `Struct Sequence API`_ allows you to define and create `Struct Sequence Objects`_ within C but act (almost) like
``collections.namedtuple`` objects.
These are very useful in creating the equivalent of a C ``struct`` in Python.


.. index:: single: Struct Sequence; Differences from namedtuple

------------------------------------------------------------------
Differences Between a C Struct Sequence and a Python `namedtuple`_
------------------------------------------------------------------

Unlike a Python `namedtuple`_ a C Struct Sequence does *not* have the following functions and attributes
(the official Python documentation does not point this out):

- `_make() <https://docs.python.org/3/library/collections.html#collections.somenamedtuple._make>`_
- `_asdict() <https://docs.python.org/3/library/collections.html#collections.somenamedtuple._asdict>`_
- `_replace() <https://docs.python.org/3/library/collections.html#collections.somenamedtuple._replace>`_
- `_fields <https://docs.python.org/3/library/collections.html#collections.somenamedtuple._fields>`_
- `_field_defaults <https://docs.python.org/3/library/collections.html#collections.somenamedtuple._field_defaults>`_

`Struct Sequence Objects`_ also differ from `namedtuples`_ in the way that members can be accessed.
`namedtuples`_ can access *all* their members either by name or by index.
A `Struct Sequence Object`_ can be designed so that any attribute can be accessed by either name or index or both
(or even neither!).
TODO: Check this against the n_in_sequence documentation below.

.. index:: single: Struct Sequence; Basic Example

------------------------------------------------------------------
A Basic C Struct Sequence
------------------------------------------------------------------

Here is an example of defining a Struct Sequence in C (the code is in ``src/cpy/StructSequence/cStructSequence.c``).

.. index:: single: Struct Sequence; Basic Example; Documentation String

Documentation String
--------------------

First create a named documentation string:

.. code-block:: c

    PyDoc_STRVAR(
        BasicNT_docstring,
        "A basic named tuple type with two fields."
    );

.. index:: single: Struct Sequence; Basic Example; Field Specifications

Field Specifications
--------------------

Now create the field definitions as an array of `PyStructSequence_Field`_.
These are just pairs of ``{field_name, field_description}``:

.. code-block:: c

    static PyStructSequence_Field BasicNT_fields[] = {
        {"field_one", "The first field of the named tuple."},
        {"field_two", "The second field of the named tuple."},
        {NULL, NULL}
    };

.. index:: single: Struct Sequence; Basic Example; Type Specification

Struct Sequence Type Specification
----------------------------------

Now create the `PyStructSequence_Desc`_ that is a name, documentation, fields and the number of fields visible in
Python.
The latter value is explained later but for the moment make it the number of declared fields.

.. code-block:: c

    static PyStructSequence_Desc BasicNT_desc = {
        "cStructSequence.BasicNT",
        BasicNT_docstring,
        BasicNT_fields,
        2,
    };

.. note::

    If the given number of fields (`n_in_sequence`_) is greater than the length of the fields array then
    `PyStructSequence_NewType()`_ will return NULL.

    There is a test example of this ``dbg_PyStructSequence_n_in_sequence_too_large()`` in
    ``src/cpy/Containers/DebugContainers.c``.

.. index:: single: Struct Sequence; Basic Example; Creating an Instance

Creating an Instance
--------------------

Here is a function ``BasicNT_create()`` that creates a Struct Sequence from arguments provided from a Python session.
Things to note:

- There is a static `PyTypeObject`_ which holds a reference to the Struct Sequence type.
- This is initialised with `PyStructSequence_NewType()`_ that takes the ``BasicNT_desc`` described above.
- Then the function `PyStructSequence_New()`_ is used to create a new, empty, instance of that type.
- Finally `PyStructSequence_SetItem()`_ is used to set each individual field from the given arguments.

.. note::

    The careful use of ``Py_INCREF`` when setting the fields.

.. code-block:: c

    static PyObject *
    BasicNT_create(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
        assert(!PyErr_Occurred());
        static char *kwlist[] = {"field_one", "field_two", NULL};
        PyObject *field_one = NULL;
        PyObject *field_two = NULL;
        static PyTypeObject *static_BasicNT_Type = NULL;

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO", kwlist, &field_one, &field_two)) {
            return NULL;
        }
        /* The two fields are PyObjects. If your design is that those arguments should be
         * specific types then take the opportunity here to test that they are the
         * expected types.
         */

        /* Now check that the type is available. */
        if (!static_BasicNT_Type) {
            static_BasicNT_Type = PyStructSequence_NewType(&BasicNT_desc);
            if (!static_BasicNT_Type) {
                PyErr_SetString(
                        PyExc_MemoryError,
                        "Can not initialise a BasicNT type with PyStructSequence_NewType()"
                );
                return NULL;
            }
        }
        PyObject *result = PyStructSequence_New(static_BasicNT_Type);
        if (!result) {
            PyErr_SetString(
                    PyExc_MemoryError,
                    "Can not create a Struct Sequence with PyStructSequence_New()"
            );
            return NULL;
        }
        /* PyArg_ParseTupleAndKeywords with "O" gives a borrowed reference.
         * https://docs.python.org/3/c-api/arg.html#other-objects
         * "A new strong reference to the object is not created
         * (i.e. its reference count is not increased)."
         *
         * So we increment as PyStructSequence_SetItem seals the reference otherwise
         * if the callers arguments goes out of scope we will/may get undefined behaviour
         * when accessing the named tuple fields.
         */
        Py_INCREF(field_one);
        Py_INCREF(field_two);
        PyStructSequence_SetItem(result, 0, field_one);
        PyStructSequence_SetItem(result, 1, field_two);
        return result;
    }

This function is then added to the ``cStructSequence`` module like this:

.. code-block:: c

    static PyMethodDef cStructSequence_methods[] = {
        /* More stuff here... */
        {
            "BasicNT_create",
            (PyCFunction) BasicNT_create,
            METH_VARARGS | METH_KEYWORDS,
            "Create a BasicNT from the given values."
        },
        /* More stuff here... */
        {NULL, NULL, 0, NULL} /* Sentinel */
    };

Using an Instance
--------------------

And can be used like this:

.. code-block:: python

    from cPyExtPatt import cStructSequence

    def test_basic_nt_create():
        basic_nt = cStructSequence.BasicNT_create('foo', 'bar')
        assert str(type(basic_nt)) == "<class 'cStructSequence.BasicNT'>"


    def test_basic_nt_create_attributes():
        basic_nt = cStructSequence.BasicNT_create('foo', 'bar')
        assert basic_nt.field_one == "foo"
        assert basic_nt.field_two == "bar"
        assert basic_nt.index("foo") == 0
        assert basic_nt.index("bar") == 1
        assert basic_nt.n_fields == 2
        assert basic_nt.n_sequence_fields == 2
        assert basic_nt.n_unnamed_fields == 0


-------------------------------------------------
Whether to Provide Access to the Type from Python
-------------------------------------------------

One decision to be made is whether to expose your Struct Sequence *type* from the module.
There are only two use cases for this:

- Do you want the user to be able to create your Struct Sequence/``namedtuple`` directly from Python?
  In which case then you need to expose the type of your Struct Sequence from the module.
  Then anyone can create these objects directly from Python.
- If the objects are created only in C then you do not need to expose the *type* in the module
  but you can create functions the create those Python objects (and their types) dynamically.

Firstly, exposing the Struct Sequence type to Python.

.. index:: single: Struct Sequence; Exposing the Type

Exposing the Type from the CPython Module
-----------------------------------------

In this case the Struct Sequence can be created *from* Python (as well as from C).

For example here is a simple Struct Sequence ``cStructSequence.NTRegistered`` that contains two fields.

First, creating the documentation:

.. code-block:: c

    PyDoc_STRVAR(
            NTRegistered_docstring,
            "A namedtuple type with two fields that is"
            "registered with the cStructSequence module."
    );

Defining the fields, this is an array of `PyStructSequence_Field`_ which are pairs of
``{field_name, field_documentation}`` (both strings) and terminated with a NULL sentinel:

.. code-block:: c

    static PyStructSequence_Field NTRegistered_fields[] = {
            {"field_one", "The first field of the namedtuple."},
            {"field_two", "The second field of the namedtuple."},
            {NULL, NULL}
    };

Creating the Struct Sequence description that will define the type.
This is a `PyStructSequence_Desc`_ that consists of:

- The Struct Sequence name, this must include the module name so is of the form
  ``"module_name.struct_sequence_name"``:
- The documentation string.
- The fields as an array of `PyStructSequence_Field`_.
- The number of fields exposed to Python.

.. code-block:: c

    static PyStructSequence_Desc NTRegistered_desc = {
            "cStructSequence.NTRegistered",
            NTRegistered_docstring,
            NTRegistered_fields,
            2,
    };

Then the module initialisation code looks like this, this uses `PyStructSequence_NewType()`_ to create the type:

.. code-block:: c

    PyMODINIT_FUNC
    PyInit_cStructSequence(void) {
        PyObject *m;
        m = PyModule_Create(&cStructSequence_cmodule);
        if (m == NULL) {
            return NULL;
        }
        /* Initialise NTRegisteredType */
        PyObject *NTRegisteredType = (PyObject *) PyStructSequence_NewType(
            &NTRegistered_desc
        );
        if (NTRegisteredType == NULL) {
            Py_DECREF(m);
            return NULL;
        }
        Py_INCREF(NTRegisteredType);
        PyModule_AddObject(m, "NTRegisteredType", NTRegisteredType);

        /*
         * Other module initialisation code here.
         */

        return m;
    }

This can be used thus in Python:

.. code-block:: python

    from cPyExtPatt import cStructSequence

    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    assert str(nt) == "cStructSequence.NTRegistered(field_one='foo', field_two='bar')"


There are tests for this in ``tests/unit/test_c_struct_sequence.py``.

.. index:: single: Struct Sequence; Hiding the Type

Hiding the Type in the Module
---------------------------------

In this case the Struct Sequence can be *not* be created from Python, only in C.
Even though the constructor is not accessible from Python the type is, as we will see.

For example here is a simple Struct Sequence ``cStructSequence.NTUnRegistered`` that contains two fields.
It is, initially, very similar to the above.

.. code-block:: c

    PyDoc_STRVAR(
            NTUnRegistered_docstring,
            "A namedtuple type with two fields that is"
            " not registered with the cStructSequence module."
    );

    static PyStructSequence_Field NTUnRegistered_fields[] = {
            {"field_one", "The first field of the namedtuple."},
            {"field_two", "The second field of the namedtuple."},
            {NULL, NULL}
    };

    static PyStructSequence_Desc NTUnRegistered_desc = {
            "cStructSequence.NTUnRegistered",
            NTUnRegistered_docstring,
            NTUnRegistered_fields,
            2,
    };

However as the type is not initialised in the module definition it remains static to the module C code.
It is, as yet, uninitialised:

.. code-block:: c

    /* Type initailised dynamically by NTUnRegistered_create(). */
    static PyTypeObject *static_NTUnRegisteredType = NULL;

This type, and objects created from it, can be made with a function call,
in this case taking variable and keyword arguments:

.. code-block:: c

    /* A function that creates a cStructSequence.NTUnRegistered dynamically. */
    PyObject *
    NTUnRegistered_create(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
        assert(!PyErr_Occurred());
        static char *kwlist[] = {"field_one", "field_two", NULL};
        PyObject *field_one = NULL;
        PyObject *field_two = NULL;

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO", kwlist, &field_one, &field_two)) {
            return NULL;
        }
        /* The two fields are PyObjects. If your design is that those arguments should be
         * specific types then take the opportunity here to test that they are the
         * expected types.
         */

        /* Initialise the static static_NTUnRegisteredType.
         * Note: PyStructSequence_NewType returns a new reference.
         */
        if (!static_NTUnRegisteredType) {
            static_NTUnRegisteredType = PyStructSequence_NewType(&NTUnRegistered_desc);
            if (!static_NTUnRegisteredType) {
                PyErr_SetString(
                    PyExc_MemoryError,
                    "Can not initialise a type with PyStructSequence_NewType()"
                );
                return NULL;
            }
        }
        PyObject *result = PyStructSequence_New(static_NTUnRegisteredType);
        if (!result) {
            PyErr_SetString(
                PyExc_MemoryError,
                "Can not create a Struct Sequence with PyStructSequence_New()"
            );
            return NULL;
        }
        /* PyArg_ParseTupleAndKeywords with "O" gives a borrowed reference.
         * https://docs.python.org/3/c-api/arg.html#other-objects
         * "A new strong reference to the object is not created
         * (i.e. its reference count is not increased)."
         * So we increment as PyStructSequence_SetItem seals the reference otherwise if
         * the callers arguments go out of scope we will/may get undefined behaviour when
         * accessing the namedtuple fields.
         */
        Py_INCREF(field_one);
        Py_INCREF(field_two);
        PyStructSequence_SetItem(result, 0, field_one);
        PyStructSequence_SetItem(result, 1, field_two);
        return result;
    }

And this can be used thus:

.. code-block:: python

    from cPyExtPatt import cStructSequence

    ntu = cStructSequence.NTUnRegistered_create('foo', 'bar')
    # Note that the type is available
    assert str(type(ntu)) == "<class 'cStructSequence.NTUnRegistered'>"

A common use of this is converting a C ``struct`` to a Python ``namedtuple``.

.. index:: single: Struct Sequence; C structs

-----------------------------------------
Converting a C ``struct`` to a namedtuple
-----------------------------------------

A common use case for *not* exposing the ``namedtuple`` type from the module is when the data object can *only* be
created in C.
Suppose that we have a simple struct representing a transaction.

.. code-block:: c

    /**
     * Representation of a simple transaction.
     */
    struct cTransaction {
        long id;            /* The transaction id. */
        char *reference;    /* The transaction reference. */
        double amount;      /* The transaction amount. */
    };

An we have a C function that can recover a transaction given its ID:

.. code-block:: c

    /**
     * An example function that might recover a transaction from within C code,
     * possibly a C library.
     * In practice this will actually do something more useful that this function does!
     *
     * @param id The transaction ID.
     * @return A struct cTransaction corresponding to the transaction ID.
     */
    static struct cTransaction get_transaction(long id) {
        struct cTransaction ret = {id, "Some reference.", 42.76};
        return ret;
    }

Then we create a ``namedtuple`` type that mirrors the C ``struct Transaction``:

.. code-block:: c

    PyDoc_STRVAR(
        cTransaction_docstring,
        "Example of a named tuple type representing a transaction created in C."
        " The type is not registered with the cStructSequence module."
    );

    static PyStructSequence_Field cTransaction_fields[] = {
            {"id", "The transaction id."},
            {"reference", "The transaction reference."},
            {"amount", "The transaction amount."},
            {NULL, NULL}
    };

    static PyStructSequence_Desc cTransaction_desc = {
            "cStructSequence.cTransaction",
            cTransaction_docstring,
            cTransaction_fields,
            3,
    };

This Python type is declared static and initialised dynamically when necessary.
As this might be used by multiple functions so we give it an API:

.. code-block:: c

    /* Type initialised dynamically by get_cTransactionType(). */
    static PyTypeObject *static_cTransactionType = NULL;

    static PyTypeObject *get_cTransactionType(void) {
        if (static_cTransactionType == NULL) {
            static_cTransactionType = PyStructSequence_NewType(&cTransaction_desc);
            if (static_cTransactionType == NULL) {
                PyErr_SetString(
                    PyExc_MemoryError,
                    "Can not initialise a cTransaction type with PyStructSequence_NewType()"
                );
                return NULL;
            }
        }
        return static_cTransactionType;
    }

Now the Python/C interface function:

.. code-block:: c

    /* A function that creates a cStructSequence.NTUnRegistered dynamically. */
    PyObject *
    cTransaction_get(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
        assert(!PyErr_Occurred());
        static char *kwlist[] = {"id", NULL};
        long id = 0l;
        if (!PyArg_ParseTupleAndKeywords(args, kwds, "l", kwlist, &id)) {
            return NULL;
        }
        PyObject *result = PyStructSequence_New(get_cTransactionType());
        if (!result) {
            assert(PyErr_Occurred());
            return NULL;
        }

        struct cTransaction transaction = get_transaction(id);
        PyStructSequence_SetItem(result, 0, PyLong_FromLong(transaction.id));
        PyStructSequence_SetItem(result, 1, PyUnicode_FromString(transaction.reference));
        PyStructSequence_SetItem(result, 2, PyFloat_FromDouble(transaction.amount));
        return result;
    }

Add to the module methods:

.. code-block:: c

    static PyMethodDef cStructSequence_methods[] = {
        /* Other stuff... */
        {
            "cTransaction_get",
            (PyCFunction) cTransaction_get,
            METH_VARARGS | METH_KEYWORDS,
            "Example of getting a transaction."
        },
        /* Other stuff... */
        {NULL, NULL, 0, NULL} /* Sentinel */
    };

And then this can be called from Python like this:

.. code-block:: c

    nt = cStructSequence.cTransaction_get(17145)
    assert nt.id == 17145
    assert nt.reference == "Some reference."
    assert nt.amount == 42.76

.. _n_in_sequence: https://docs.python.org/3/c-api/tuple.html#c.PyStructSequence_Desc.n_in_sequence

.. index:: single: Struct Sequence; Controlling Member Access

---------------------------------------------
Controlling Member Access
---------------------------------------------

`Struct Sequence Objects`_ differ from `namedtuples`_ in the way that members can be accessed.
A `Struct Sequence Object`_ can be designed so that any attribute can be accessed by either name or index or both
(or even neither!).
This describes how to do this.

.. index:: single: Struct Sequence; n_in_sequence

The Importance of the ``n_in_sequence`` Field
---------------------------------------------

`PyStructSequence_Desc`_ has a field `n_in_sequence`_ which needs some explaining (the Python documentation is pretty
silent on this).
Normally `n_in_sequence`_ is equal to the number of fields, however what happens if it is not?

``n_in_sequence`` > Number of Fields
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As mentioned above if the given number of fields (`n_in_sequence`_) is greater than the length of the fields array then
`PyStructSequence_NewType()`_ will return NULL.

There is a test example of this ``dbg_PyStructSequence_n_in_sequence_too_large()`` in
``src/cpy/Containers/DebugContainers.c``.

``n_in_sequence`` < Number of Fields
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In this case the members with an index >= `n_in_sequence`_ will raise an ``IndexError``.
However that same member can always be accessed from Python by name.

There some illustrative tests ``test_excess_nt_*`` in ``tests/unit/test_c_struct_sequence.py`` for this.

.. index:: single: Struct Sequence; Unnamed Fields

Unnamed Fields
---------------------------------------------

TODO: Finish this.
