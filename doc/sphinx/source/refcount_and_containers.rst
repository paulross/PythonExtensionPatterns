.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3


..
    Links, mostly to the Python documentation:

.. _PyTuple_SetItem(): https://docs.python.org/3/c-api/tuple.html#c.PyTuple_SetItem
.. _PyTuple_SET_ITEM(): https://docs.python.org/3/c-api/tuple.html#c.PyTuple_SET_ITEM
.. _Py_BuildValue(): https://docs.python.org/3/c-api/arg.html#c.Py_BuildValue
.. _PyTuple_Pack(): https://docs.python.org/3/c-api/tuple.html#c.PyTuple_Pack

.. _chapter_refcount_and_containers:

======================================
Reference Counts and Python Containers
======================================

The descriptions of *New*, *Stolen* and *Borrowed* references were described in the preceding chapter.
This chapter looks in more detail of how the Python C API works with different containers, tuple, list, set and dict.

Of particular interest is *Setters*, *Getters* and the behaviour of ``Py_BuildValue`` for each of those containers.
This chapter also clarifies the Python documentation where it is inaccurate or misleading.

Buckle up.

--------------------------
Methodology
--------------------------

This chapter explores the CPython C API in several ways:

* Tests of the CPython C API that can be stepped through in the debugger.
  This code is in ``src/cpy/Containers/DebugContainers.h`` and ``src/cpy/Containers/DebugContainers.c``
  and ``asserts`` are used to check the results, particularly reference counts.
  It is exercised by ``src/main.c``.
* Similar test code is in ``src/cpy/RefCount/cRefCount.c`` which is built into the Python module ``cRefCount``.
  This can be run under ``pytest``.
* A review of the Python C API documentation.

Here is an example of exploring reference counts and tuples.

* A tuple of length 1 is created.
* Then a new Python object is created, its reference count is tested as 1.
* That object object is inserted at [0] with ``PyTuple_SetItem()``.
  The objects reference count remains the same at 1 as the tuple has *stolen* the reference.
* In the code below we hold *two* references to that object.
  ``value_0`` and tuple[0] so we can observe the behaviour of the
  tuple and original object.
* Now we create a new Python object, reference count 1
  and insert this new object at [0] with ``PyTuple_SetItem()``.
* What happens to the previous object that occupied [0]?
* It is **discarded**.

.. code-block:: c

    static PyObject *
    dbg_PyTuple_SetItem(PyObject *Py_UNUSED(module)) {
        /* Create a new tuple and check its reference count. */
        PyObject *container = PyTuple_New(1);
        assert(container);
        assert(Py_REFCNT(container) == 1);

        /* Create a new string. */
        PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
        assert(Py_REFCNT(value_0) == 1);

        /* Set it as tuple[0].
         * The reference count is stolen (does not increase).
         */
        PyTuple_SetItem(container, 0, value_0);
        assert(Py_REFCNT(value_0) == 1);

        /* Now replace it. */
        PyObject *value_1 = new_unique_string(__FUNCTION__);
        assert(Py_REFCNT(value_1) == 1);

        /* This will discard value_0 leaving it with a reference count of 1. */
        PyTuple_SetItem(container, 0, value_1);
        assert(Py_REFCNT(value_1) == 1);

        PyObject *get_item = PyTuple_GET_ITEM(container, 0);
        assert(get_item == value_1);
        assert(Py_REFCNT(get_item) == 1);

        Py_DECREF(container);

        /* This is now leaked. */
        assert(Py_REFCNT(value_0) == 1);

        assert(!PyErr_Occurred());
        Py_RETURN_NONE;
    }

Firstly Tuples:

-----------------------
Tuple
-----------------------

The Python documentation for the `Tuple API <https://docs.python.org/3/c-api/tuple.html>`_.

.. list-table:: Tuple API
   :widths: 50 20 40
   :header-rows: 1

   * - Python C API
     - Behaviour
     - Notes
   * - `PyTuple_SetItem()`_
     - Steals, decrements the reference count of the original.
     - More stuff.
   * - `PyTuple_SET_ITEM()`_
     - Steals, leaks original.
     - **Contrary** to the documentation this leaks.
   * - ``Py_BuildValue("(s)", val)``
     - Steals, leaks original.
     - More stuff.


``PyTuple_SetItem()``
---------------------

Basic Usage
^^^^^^^^^^^

`PyTuple_SetItem()`_ *steals* a reference.

.. code-block:: c

    PyObject *container = PyTuple_New(1); /* Reference count will be 1. */
    PyObject *value = new_unique_string(__FUNCTION__, NULL); /* Ref count will be 1. */
    PyTuple_SetItem(container, 0, value); /* Ref count of value will be 1. */
    /* get_item == value and Ref count will be 1. */
    PyObject *get_item = PyTuple_GET_ITEM(container, 0);
    Py_DECREF(container); /* The contents of the container, value, will be decref'd */
    /* Do not do this as the container deals with this. */
    /* Py_DECREF(value); */

For code tests see:

* ``dbg_PyTuple_SetItem_steals`` in ``src/cpy/Containers/DebugContainers.c``.
* ``test_PyTuple_SetItem_steals`` in ``src/cpy/RefCount/cRefCount.c``.
* ``tests.unit.test_c_ref_count.test_test_PyTuple_SetItem_steals``.

Replacement
^^^^^^^^^^^

What happens when you use `PyTuple_SetItem()`_ to replace an existing element in a tuple.
`PyTuple_SetItem()`_ still *steals* a reference, but what happens to the original reference?

.. code-block:: c

    PyObject *container = PyTuple_New(1); /* Reference count will be 1. */
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL); /* Ref count will be 1. */
    PyTuple_SetItem(container, 0, value_a); /* Ref count of value_a will be 1. */
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL); /* Ref count will be 1. */
    PyTuple_SetItem(container, 0, value_b);
    /* Ref count of value_b will be 1, value_a ref count will be decremented. */

For code tests see:

* ``dbg_PyTuple_SetItem_steals_replace`` in ``src/cpy/Containers/DebugContainers.c``.
* ``test_PyTuple_SetItem_steals_replace`` in ``src/cpy/RefCount/cRefCount.c``.
* ``tests.unit.test_c_ref_count.test_test_PyTuple_SetItem_steals_replece``.



``PyTuple_SET_ITEM()``
----------------------

`PyTuple_SET_ITEM()`_


``Py_BuildValue()``
-------------------

`Py_BuildValue()`_


``PyTuple_Pack()``
------------------

`PyTuple_Pack()`_ is a wrapper around `Py_BuildValue()`_ so is not explored any further.

-----------------------
List
-----------------------

-----------------------
Dictionary
-----------------------

-----------------------
Set
-----------------------

Example footnote [#]_.

.. rubric:: Footnotes

.. [#] A footnote.
