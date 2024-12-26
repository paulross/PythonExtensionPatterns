.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

..
    Links, mostly to the Python documentation.
    Specific container links are just before the appropriate section.

.. _Py_BuildValue(): https://docs.python.org/3/c-api/arg.html#c.Py_BuildValue

.. _chapter_refcount_and_containers:

.. index:: single: Containers

======================================
Reference Counts and Python Containers
======================================

The descriptions of *New*, *Stolen* and *Borrowed* references were described in the preceding chapter.
This chapter looks in more detail of how the Python C API works with different containers,
such as ``tuple``, ``list``, ``set`` and ``dict`` [#]_.

This chapter includes examples and tests that you can step through to better understand the interplay
between the container and the object in that container.

Of particular interest is *Setters*, *Getters* and the behaviour of ``Py_BuildValue`` for each of those
containers [#]_.
This chapter also clarifies the Python documentation where that is inaccurate, incomplete or misleading.

---------------------------
Exploring the CPython C API
---------------------------

The code in this chapter explores the CPython C API in several ways:

* C code that can be stepped through in the debugger.
  This code is in ``src/cpy/Containers/DebugContainers.c``.
  ``asserts`` are used to check the results, particularly reference counts.
  It is exercised by ``src/main.c``.
* Test code is in ``src/cpy/RefCount/cRefCount.c`` which is built into the Python module
  ``cPyExtPatt.cRefCount``.
  This can be run under ``pytest`` for multiple Python versions by ``build_all.sh``.
* A study of the Python source code.
* A review of the Python C API documentation.

.. note::

    The examples below use code that calls a function ``new_unique_string()``.
    This function is designed to create a new, unique,  ``PyObject`` (a string)
    that is never cached so always has a reference count of unity.
    The implementation is in ``src/cpy/Containers/DebugContainers.c`` and looks something like this:

    .. code-block:: c

        static long debug_test_count = 0L;

        PyObject *
        new_unique_string(const char *function, const char *suffix) {
            if (suffix){
                return PyUnicode_FromFormat(
                    "%s-%s-%ld", function, suffix, debug_test_count++
                );
            }
            return PyUnicode_FromFormat("%s-%ld", function, debug_test_count++);
        }

Firstly Tuples, I'll go into quite a lot of detail here because it covers much of the other containers as well.

.. _chapter_refcount_and_containers.tuples:

..
    Links, mostly to the Python documentation:

.. _PyTuple_SetItem(): https://docs.python.org/3/c-api/tuple.html#c.PyTuple_SetItem
.. _PyTuple_SET_ITEM(): https://docs.python.org/3/c-api/tuple.html#c.PyTuple_SET_ITEM
.. _PyTuple_Pack(): https://docs.python.org/3/c-api/tuple.html#c.PyTuple_Pack
.. _PyTuple_GetItem(): https://docs.python.org/3/c-api/tuple.html#c.PyTuple_GetItem
.. _PyTuple_GET_ITEM(): https://docs.python.org/3/c-api/tuple.html#c.PyTuple_GET_ITEM


.. index::
    single: Tuple

-----------------------
Tuples
-----------------------

The Python documentation for the `Tuple C API <https://docs.python.org/3/c-api/tuple.html>`_
is here.

Firstly setters, there are two APIs for setting an item in a tuple; `PyTuple_SetItem()`_ and `PyTuple_SET_ITEM()`_.

.. _chapter_refcount_and_containers.tuples.PyTuple_SetItem:

.. index::
    single: PyTuple_SetItem()
    pair: PyTuple_SetItem(); Tuple

``PyTuple_SetItem()``
---------------------

`PyTuple_SetItem()`_ (a C function) inserts an object into a tuple with error checking.
This function returns non-zero on error, these are described below in
:ref:`chapter_refcount_and_containers.tuples.PyTuple_SetItem.failures`.
The failure of `PyTuple_SetItem()`_ has serious consequences for the value
that is intended to be inserted.

Basic Usage
^^^^^^^^^^^

`PyTuple_SetItem()`_ *steals* a reference, that is, the container assumes responsibility
for freeing that value when the container is free'd ('freeing' meaning decrementing the reference count).
For example:

.. code-block:: c

    PyObject *container = PyTuple_New(1); /* container ref count will be 1. */
    PyObject *value = new_unique_string(__FUNCTION__, NULL); /* value ref count will be 1. */
    PyTuple_SetItem(container, 0, value); /* value ref count will remain at 1. */
    /* get_item == value and value ref count will be 1. */
    PyObject *get_item = PyTuple_GET_ITEM(container, 0);
    /* The contents of the container and value, will be decref'd
     * In this particular case both will go to zero and free'd. */
    Py_DECREF(container);
    /* Do not do this as the container has dealt with this. */
    /* Py_DECREF(value); */

For code and tests see:

* C: ``dbg_PyTuple_SetItem_steals`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_SetItem_steals`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_SetItem_steals``.

Replacement
^^^^^^^^^^^

What happens when you use `PyTuple_SetItem()`_ to replace an existing element in a tuple?
`PyTuple_SetItem()`_ still *steals* a reference, but what happens to the original reference?
Lets see:

.. code-block:: c

    PyObject *container = PyTuple_New(1); /* container ref count will be 1. */
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    /* value_a ref count will be 1. */
    PyTuple_SetItem(container, 0, value_a); /* value_a ref count will remain 1. */
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL);
    /* value_b ref count will be 1. */
    PyTuple_SetItem(container, 0, value_b);
    /* Now value_b ref count will remain 1 and value_a ref count will have been decremented
     * In this case value_a will have been free'd. */

For code and tests see:

* C: ``dbg_PyTuple_SetItem_steals_replace`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_SetItem_steals_replace`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_SetItem_steals_replace``.

.. _chapter_refcount_and_containers.tuples.PyTuple_SetItem.failures:

``PyTuple_SetItem()`` Failures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

`PyTuple_SetItem()`_ can fail for these reasons:

* The given container is not a tuple.
* The index is out of range; index < 0 or index >= tuple length (negative indexes are not allowed).

A consequence of failure is that the value being inserted will be decref'd.
For example this code will segfault:

.. code-block:: c

    PyObject *container = PyTuple_New(1); /* Reference count will be 1. */
    PyObject *value = new_unique_string(__FUNCTION__, NULL); /* Ref count will be 1. */
    PyTuple_SetItem(container, 1, value); /* Index out of range. */
    Py_DECREF(value); /* value has already been decref'd and free'd so this will SIGSEGV */

For code tests see, when the container is not a tuple:

* C: ``dbg_PyTuple_SetItem_fails_not_a_tuple`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_SetItem_fails_not_a_tuple`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_SetItem_fails_not_a_tuple``.

And, when the index out of range:

* C: ``dbg_PyTuple_SetItem_fails_out_of_range`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_SetItem_fails_out_of_range`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_SetItem_fails_out_of_range``.

.. note::

    I'm not really sure why the `PyTuple_SetItem()`_ API exists.
    Tuples are meant to be immutable but this API treats existing tuples as mutable.
    It would seem like the `PyTuple_SET_ITEM()`_ would be enough.

.. _chapter_refcount_and_containers.tuples.PyTuple_SET_ITEM:

.. index::
    single: PyTuple_SET_ITEM()
    pair: PyTuple_SET_ITEM(); Tuple

``PyTuple_SET_ITEM()``
----------------------

`PyTuple_SET_ITEM()`_ is a function like macro that inserts an object into a tuple without any error checking
(see :ref:`chapter_refcount_and_containers.tuples.PyTuple_SET_ITEM.failures` below) although the type
checking is performed as an assertion if Python is built in
`debug mode <https://docs.python.org/3/using/configure.html#debug-build>`_ or
`with assertions <https://docs.python.org/3/using/configure.html#cmdoption-with-assertions>`_.
Because of that, it is slightly faster than `PyTuple_SetItem()`_ .
This is usually used on newly created tuples.

Importantly `PyTuple_SET_ITEM()`_ behaves **differently** to `PyTuple_SetItem()`_
when replacing another object.

Basic Usage
^^^^^^^^^^^

`PyTuple_SET_ITEM()`_ *steals* a reference just like `PyTuple_SetItem()`_.

.. code-block:: c

    PyObject *container = PyTuple_New(1); /* Reference count will be 1. */
    PyObject *value = new_unique_string(__FUNCTION__, NULL); /* Ref count will be 1. */
    PyTuple_SET_ITEM(container, 0, value); /* Ref count of value will be 1. */
    /* get_item == value and Ref count will be 1. */
    PyObject *get_item = PyTuple_GET_ITEM(container, 0);
    assert(get_item == value && Py_REFCNT(value) == 1);
    Py_DECREF(container); /* The contents of the container, value, will be decref'd */
    /* Do not do this as the container deals with this. */
    /* Py_DECREF(value); */

For code and tests see:

* C: ``dbg_PyTuple_PyTuple_SET_ITEM_steals`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_PyTuple_SET_ITEM_steals`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_PyTuple_SET_ITEM_steals``.

.. _chapter_refcount_and_containers.tuples.PyTuple_SET_ITEM.replacement:

Replacement
^^^^^^^^^^^

`PyTuple_SET_ITEM()`_ **differs** from `PyTuple_SetItem()`_ when replacing an existing
element in a tuple as the original reference will be leaked:

.. code-block:: c

    PyObject *container = PyTuple_New(1); /* Reference count will be 1. */
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL); /* Ref count will be 1. */
    PyTuple_SET_ITEM(container, 0, value_a); /* Ref count of value_a will be 1. */
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL); /* Ref count will be 1. */
    PyTuple_SET_ITEM(container, 0, value_b);
    assert(Py_REFCNT(value_a) == 1);
    /* Ref count of value_b will be 1,
     * value_a ref count will still be at 1 and value_a will be leaked unless decref'd. */

For code and tests see:

* C: ``dbg_PyTuple_SET_ITEM_steals_replace`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_SetItem_steals_replace`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_SetItem_steals_replace``.

.. _chapter_refcount_and_containers.tuples.PyTuple_SET_ITEM.failures:

``PyTuple_SET_ITEM()`` Failures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

`PyTuple_SET_ITEM()`_ minimises the error checking that `PyTuple_SetItem()`_ does, so:

* It does not check if the given container is a Tuple.
* It does not check if the index in range.

If either of those is wrong then `PyTuple_SET_ITEM()`_ is capable of writing to arbitrary
memory locations, and the result is likely to be tragic, mostly undefined behaviour
and/or memory corruption:

Setting and Replacing ``NULL``
------------------------------

This looks at what happens when setting or replacing a ``NULL`` pointer in a tuple.
Both `PyTuple_SetItem()`_ and `PyTuple_SET_ITEM()`_ behave the same way.

Setting ``NULL``
^^^^^^^^^^^^^^^^

Setting a ``NULL`` will not cause an error:

.. code-block:: c

    PyObject *container = PyTuple_New(1);
    assert(!PyErr_Occurred());
    PyTuple_SetItem(container, 0, NULL);
    assert(!PyErr_Occurred());

For code and tests see:

* C: ``dbg_PyTuple_SetIem_NULL`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_SetItem_NULL`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_SetItem_NULL``.

And:

.. code-block:: c

    PyObject *container = PyTuple_New(1);
    assert(!PyErr_Occurred());
    PyTuple_SET_ITEM(container, 0, NULL);
    assert(!PyErr_Occurred());

For code and tests see:

* C: ``dbg_PyTuple_SET_ITEM_NULL`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_SET_ITEM_NULL`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_SET_ITEM_NULL``.

Replacing ``NULL``
^^^^^^^^^^^^^^^^^^

Replacing a ``NULL`` will not cause an error, the replaced value reference is *stolen*:

.. code-block:: c

    PyObject *container = PyTuple_New(1);
    PyTuple_SetItem(container, 0, NULL);
    PyObject *value = new_unique_string(__FUNCTION__, NULL); /* Ref Count of value is 1. */
    PyTuple_SetItem(container, 0, value); /* Ref Count of value is still 1. */

For code and tests see:

* C: ``dbg_PyTuple_SetIem_NULL_SetIem`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_SetItem_NULL_SetIem`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_SetItem_NULL_SetIem``.

And:

.. code-block:: c

    PyObject *container = PyTuple_New(1);
    PyTuple_SET_ITEM(container, 0, NULL);
    PyObject *value = new_unique_string(__FUNCTION__, NULL); /* Ref Count of value is 1. */
    PyTuple_SET_ITEM(container, 0, value); /* Ref Count of value is still 1. */

For code and tests see:

* C: ``dbg_PyTuple_SET_ITEM_NULL_SET_ITEM`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_SET_ITEM_NULL_SET_ITEM`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_SET_ITEM_NULL_SET_ITEM``.

.. _chapter_refcount_and_containers.tuples.PyTuple_Pack:

.. index::
    single: PyTuple_Pack()
    pair: PyTuple_Pack(); Tuple

``PyTuple_Pack()``
------------------

`PyTuple_Pack()`_ takes a length and a variable argument list of PyObjects.
Each of those PyObjects reference counts will be incremented.
In that sense it behaves as `Py_BuildValue()`_.

.. note::
    `PyTuple_Pack()`_ is implemented as a low level routine, it does not invoke
    `PyTuple_SetItem()`_ or `PyTuple_SET_ITEM()`_ .

For example:

.. code-block:: c

    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL);
    PyObject *container = PyTuple_Pack(2, value_a, value_b);
    assert(Py_REFCNT(value_a) == 2);
    assert(Py_REFCNT(value_b) == 2);

    Py_DECREF(container);

    /* Leaks: */
    assert(Py_REFCNT(value_a) == 1);
    assert(Py_REFCNT(value_b) == 1);
    /* Fix leaks: */
    Py_DECREF(value_a);
    Py_DECREF(value_b);


For code and tests see:

* C: ``dbg_PyTuple_PyTuple_Pack`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_Py_PyTuple_Pack`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_Py_PyTuple_Pack``.

.. _chapter_refcount_and_containers.tuples.Py_BuildValue:

.. index::
    pair: Py_BuildValue(); Tuple

``Py_BuildValue()``
-------------------

`Py_BuildValue()`_ is a very convenient way to create tuples, lists and dictionaries.
``Py_BuildValue("(O)", value);`` will increment the refcount of value and this can,
potentially, leak:

.. code-block:: c

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* value reference count is 1. */
    PyObject *container = Py_BuildValue("(O)", value);
    /* value reference count is incremented to 2. */
    assert(Py_REFCNT(value) == 2);

    Py_DECREF(container);
    /* value reference count is decremented to 1. */
    assert(Py_REFCNT(value_a) == 1);

    /* value is leaked if Py_DECREF(value) is not called. */
    /* Fix leak. */
    Py_DECREF(value);

For code and tests see:

* C: ``dbg_PyTuple_Py_BuildValue`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyTuple_Py_BuildValue`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyTuple_Py_BuildValue``.

Summary
----------------------

* `PyTuple_SetItem()`_ and `PyTuple_SET_ITEM()`_ *steal* references.
* `PyTuple_SetItem()`_ and `PyTuple_SET_ITEM()`_ behave differently when replacing an existing value.
* If `PyTuple_SetItem()`_ errors it will decrement the reference count of the given value.
  Possibly with surprising results.
* `PyTuple_Pack()`_ and `Py_BuildValue()`_ increment reference counts and thus may leak.


.. _chapter_refcount_and_containers.lists:

..
    Links, mostly to the Python documentation:

.. _PyList_SetItem(): https://docs.python.org/3/c-api/list.html#c.PyList_SetItem
.. _PyList_SET_ITEM(): https://docs.python.org/3/c-api/list.html#c.PyList_SET_ITEM
.. _PyList_Append(): https://docs.python.org/3/c-api/list.html#c.PyList_Append
.. _PyList_Insert(): https://docs.python.org/3/c-api/list.html#c.PyList_Insert
.. _PyList_GetItem(): https://docs.python.org/3/c-api/list.html#c.PyList_GetItem
.. _PyList_GET_ITEM(): https://docs.python.org/3/c-api/list.html#c.PyList_GET_ITEM
.. _PyList_GetItemRef(): https://docs.python.org/3/c-api/list.html#c.PyList_GetItemRef

.. index::
    single: List

-----------------------
Lists
-----------------------

.. index::
    single: PyList_SetItem()
    pair: PyList_SetItem(); List
    single: PyList_SET_ITEM()
    pair: PyList_SET_ITEM(); List

``PyList_SetItem()`` and ``PyList_SET_ITEM()``
----------------------------------------------

`PyList_SetItem()`_ and `PyList_SET_ITEM()`_ behave identically to their equivalents `PyTuple_SetItem()`_
(see :ref:`chapter_refcount_and_containers.tuples.PyTuple_SetItem`)
and `PyTuple_SET_ITEM()`_ (see :ref:`chapter_refcount_and_containers.tuples.PyTuple_SET_ITEM`).

Note that, as with tuples, `PyList_SetItem()`_ and `PyList_SET_ITEM()`_ behave differently on replacement of values
(see :ref:`chapter_refcount_and_containers.tuples.PyTuple_SET_ITEM.replacement`).
The Python documentation on `PyList_SET_ITEM()`_ correctly identifies when a leak can occur
(unlike `PyTuple_SetItem()`_).

`Py_BuildValue()`_ also behaves identically, as far as reference counts are concerned, with Lists as it does with
Tuples (see :ref:`chapter_refcount_and_containers.tuples.Py_BuildValue`).

.. index::
    single: PyList_Append()
    pair: PyList_Append(); List

``PyList_Append()``
---------------------

`PyList_Append()`_ (a C function) adds an object onto the end of a list with error checking.
This increments the reference count of the given value which will be decremented on container destruction.
For example:

.. code-block:: c

    PyObject *container = PyList_New(0);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    PyList_Append(container, value);
    assert(Py_REFCNT(value) == 2);
    Py_DECREF(container);
    /* A leak unless decref'd. */
    assert(Py_REFCNT(value) == 1);

`PyList_Append()`_ uses `PyList_SET_ITEM()`_ in its implementation.
`PyList_Append()`_ can fail for two reasons:

* The given container is not a list.
* The given value is NULL.

On failure the reference count of value is unchanged and a ``SystemError`` is raised with the text
"bad argument to internal function".

For code and tests, including failure modes, see:

* C: ``dbg_PyList_Append...`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyList_Append...`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyList_Append`` etc.


.. index::
    single: PyList_Insert()
    pair: PyList_Insert(); List

``PyList_Insert()``
---------------------

`PyList_Insert()`_ (a C function) inserts an object before a specific index in a list with error checking.
This increments the reference count of the given value which will be decremented on container destruction.

`PyList_Insert()`_ can fail for two reasons:

* The given container is not a list.
* The given value is NULL.

On failure the reference count of value is unchanged and a ``SystemError`` is raised with the text
"bad argument to internal function".

For code and tests, including failure modes, see:

* C: ``dbg_PyList_Insert...`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyList_Insert...`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyList_Insert`` etc.

..
    This note and code blocks are quite big in latex so page break here.

.. raw:: latex

    [Continued on the next page]

    \pagebreak

.. note::

    Although the Python documentation for `PyList_Insert()`_ does not make this clear the index can be negative in
    which case the index is calculated from the end.

    For example (``dbg_PyList_Insert_Negative_Index()`` in ``src/cpy/Containers/DebugContainers.c``):

    .. code-block:: c

        PyObject *container = PyList_New(0);
        PyObject *value = new_unique_string(__FUNCTION__, NULL);
        // Insert at end
        if (PyList_Insert(container, -1L, value)) {
            assert(0);
        }
        PyObject *get_item;
        // PyList_Insert at -1 actually inserts at 0.
        assert(PyList_GET_SIZE(container) == 1L);
        get_item = PyList_GET_ITEM(container, 0L);
        assert(get_item == value);

    Also, not mentioned (but implied) in the documentation is that if the index is greater than the list length then the
    value is appended to the list.

    For example (``dbg_PyList_Insert_Is_Truncated()`` in ``src/cpy/Containers/DebugContainers.c``):

    .. code-block:: c

        PyObject *container = PyList_New(0);
        PyObject *value = new_unique_string(__FUNCTION__, NULL);
        // Insert before index 4
        if (PyList_Insert(container, 4L, value)) {
            assert(0);
        }
        PyObject *get_item;
        // PyList_Insert at 4 actually inserts at 0.
        assert(PyList_GET_SIZE(container) == 1L);
        get_item = PyList_GET_ITEM(container, 0L);
        assert(get_item == value);


.. index::
    single: PyList_GetItem()
    pair: PyList_GetItem(); List
    single: PyList_GET_ITEM()
    pair: PyList_GET_ITEM(); List
    single: PyList_GetItemRef()
    pair: PyList_GetItemREf(); List


List Getters
---------------------

There are three APIS for getting an item from a list:

* `PyList_GetItem()`_ This is very similar to `PyTuple_GetItem()`_. It returns a borrowed reference and will error
  if the supplied container is not list or the index is negative or out of range.
* `PyList_GET_ITEM()`_ This is very similar to `PyTuple_GET_ITEM()`_. It returns a borrowed reference and there is
  no error checking for the index being in range.
  The type checking is performed as an assertion if Python is built in
  `debug mode <https://docs.python.org/3/using/configure.html#debug-build>`_ or
  `with assertions <https://docs.python.org/3/using/configure.html#cmdoption-with-assertions>`_.
  If not the results aer undefined.
* `PyList_GetItemRef()`_ [From Python 3.13 onwards].
  Like `PyList_GetItem()`_ but his returns a new *strong* reference to the existing object.

Summary
----------------------

* `PyList_SetItem()`_ and `PyList_SET_ITEM()`_ *steal* references.
* `PyList_SetItem()`_ and `PyList_SET_ITEM()`_ behave differently when replacing an existing value.
* If `PyList_SetItem()`_ errors it will decrement the reference count of the given value.
  Possibly with surprising results.
* `PyList_Append()`_ Increments the reference count of the given object and thus may leak.
* `PyList_Insert()`_ Increments the reference count of the given object and thus may leak.
* `Py_BuildValue()`_ Increments the reference count of the given object and thus may leak.


.. _chapter_refcount_and_containers.dictionaries:

..
    Links, mostly to the Python documentation:
    TODO: Investigate/create tests for all of these.

.. _PyDict_Check(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Check
.. _PyDict_SetItem(): https://docs.python.org/3/c-api/dict.html#c.PyDict_SetItem
.. _PyDict_DelItem(): https://docs.python.org/3/c-api/dict.html#c.PyDict_DelItem
.. _PyDict_GetItemRef(): https://docs.python.org/3/c-api/dict.html#c.PyDict_GetItemRef
.. _PyDict_GetItem(): https://docs.python.org/3/c-api/dict.html#c.PyDict_GetItem
.. _PyDict_GetItemWithError(): https://docs.python.org/3/c-api/dict.html#c.PyDict_GetItemWithError
.. _PyDict_SetDefault(): https://docs.python.org/3/c-api/dict.html#c.PyDict_SetDefault
.. _PyDict_SetDefaultRef(): https://docs.python.org/3/c-api/dict.html#c.PyDict_SetDefaultRef
.. _PyDict_Pop(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Pop
.. _PyDict_Items(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Items
.. _PyDict_Keys(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Keys
.. _PyDict_Values(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Values

.. _PyObject_Hash(): https://docs.python.org/3/c-api/object.html#c.PyObject_Hash

..
    TODO: Py_BuildValue with a dict.

.. index::
    single: Dictionary

-----------------------
Dictionaries
-----------------------

.. index::
    single: PyDict_SetItem()
    pair: PyDict_SetItem(); Dictionary

``PyDict_SetItem()``
--------------------

The Python documentation for `PyDict_SetItem()`_ is incomplete.
In summary `PyDict_SetItem()`_ does this with the key and value reference counts:

* If the key exists in the dictionary then key's reference count remains the same.
* If the key does *not* exist in the dictionary then its reference count will be incremented.
* The value reference count will always be incremented.
* If the key exists in the dictionary then the previous value reference count will be decremented before the value
  is replaced by the new value.
  If the key exists in the dictionary and the value is the same then this, effectively, means reference counts of
  both key and value remain unchanged.

This code illustrates `PyDict_SetItem()`_ with ``assert()`` showing the reference count:

.. code-block:: c

    PyObject *container = PyDict_New();
    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    assert(Py_REFCNT(key) == 1);
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    assert(Py_REFCNT(value_a) == 1);
    /* Insert a new key value. */
    if (PyDict_SetItem(container, key, value_a)) {
        assert(0);
    }
    assert(Py_REFCNT(key) == 2);
    assert(Py_REFCNT(value_a) == 2);

Now replace the value with another value:

.. code-block:: c

    /* Replace a value for the key. */
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL);
    if (PyDict_SetItem(container, key, value_b)) {
        assert(0);
    }
    assert(Py_REFCNT(key) == 2);
    assert(Py_REFCNT(value_a) == 1);
    assert(Py_REFCNT(value_b) == 2);

Now replace the value with the same value:

.. code-block:: c

    /* Replace with the same value for the key. */
    if (PyDict_SetItem(container, key, value_b)) {
        assert(0);
    }
    assert(Py_REFCNT(key) == 2);
    assert(Py_REFCNT(value_a) == 1);
    assert(Py_REFCNT(value_b) == 2);

``PyDict_SetItem()`` Failure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

`PyDict_SetItem()`_ can fail for the following reasons:

* The container is not a dictionary (or a sub-class of a dictionary, see `PyDict_Check()`_).
* The key is not hashable (`PyObject_Hash()`_ returns -1).
* If either the key or the values is NULL this will cause a SIGSEGV.
  These are checked with asserts if Python is built in
  `debug mode <https://docs.python.org/3/using/configure.html#debug-build>`_ or
  `with assertions <https://docs.python.org/3/using/configure.html#cmdoption-with-assertions>`_.

For code and tests see:

* C: ``dbg_PyDict_SetItem_*`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyDict_SetItem_*`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyDict_SetItem_increments``.

.. note::

    In ``src/cpy/Containers/DebugContainers.c`` there are failure tests that cause a SIGSEGV if ``ACCEPT_SIGSEGV``
    controlled in  ``src/cpy/Containers/DebugContainers.h``.


TODO:

.. _chapter_refcount_and_containers.sets:

.. index::
    single: Set

-----------------------
Sets
-----------------------

TODO:

.. _chapter_refcount_and_containers.summary:

-----------------------
Summary
-----------------------

TODO:

.. Example footnote [#]_.

.. todo::

    Chapter `Struct Sequence Objects <https://docs.python.org/3/c-api/tuple.html#struct-sequence-objects>`_

.. todo::

    Chapter on watchers, e.g. dict watchers [since Python 3.12]:
    https://docs.python.org/3/c-api/dict.html#c.PyDict_AddWatcher
    Also type watchers etc. There does not seem to be a PEP for this.
    This change has example tests: https://github.com/python/cpython/pull/31787/files

.. rubric:: Footnotes

.. [#] The official `Python documentation <https://docs.python.org/3/c-api/concrete.html>`_ categorises tuples and lists
    as *sequence objects* because they support the `Sequence Protocol <https://docs.python.org/3/c-api/sequence.html>`_
    and dictionaries and sets as *container objects* because they support the
    `Mapping Protocol <https://docs.python.org/3/c-api/mapping.html>`_.
    In this chapter I use looser language by describing all four as *containers*.

.. [#] ``Py_BuildValue`` can not create a set, only a tuple, list or dictionary.
