.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

..
    Links, mostly to the Python documentation:

.. _PyTuple_SetItem(): https://docs.python.org/3/c-api/tuple.html#c.PyTuple_SetItem
.. _PyTuple_SET_ITEM(): https://docs.python.org/3/c-api/tuple.html#c.PyTuple_SET_ITEM
.. _PyTuple_Pack(): https://docs.python.org/3/c-api/tuple.html#c.PyTuple_Pack

.. _PyList_SetItem(): https://docs.python.org/3/c-api/list.html#c.PyList_SetItem
.. _PyList_SET_ITEM(): https://docs.python.org/3/c-api/list.html#c.PyList_SET_ITEM
.. _PyList_GetItem(): https://docs.python.org/3/c-api/list.html#c.PyList_GetItem
.. _PyList_GET_ITEM(): https://docs.python.org/3/c-api/list.html#c.PyList_GET_ITEM
.. _PyList_GetItemRef(): https://docs.python.org/3/c-api/list.html#c.PyList_GetItemRef
.. _PyList_Insert(): https://docs.python.org/3/c-api/list.html#c.PyList_Insert
.. _PyList_Append(): https://docs.python.org/3/c-api/list.html#c.PyList_Append

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
(see :ref:`chapter_refcount_and_containers.tuples.PyTuple_SET_ITEM.failures` below) although the types checking
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

-----------------------
Lists
-----------------------

`PyList_SetItem()`_ and `PyList_SET_ITEM()`_ behave identically to their equivalents `PyTuple_SetItem()`_
(see :ref:`chapter_refcount_and_containers.tuples.PyTuple_SetItem`)
and `PyTuple_SET_ITEM()`_ (see :ref:`chapter_refcount_and_containers.tuples.PyTuple_SET_ITEM`).

Note that, as with tuples, `PyList_SetItem()`_ and `PyList_SET_ITEM()`_ behave differently on replacement of values
(see :ref:`chapter_refcount_and_containers.tuples.PyTuple_SET_ITEM.replacement`).

`Py_BuildValue()`_ also behaves identically, as far as reference counts are concerned, with Lists as it does with
Tuples (see :ref:`chapter_refcount_and_containers.tuples.Py_BuildValue`).

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
    /* Possible leak. */
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


TODO:

https://docs.python.org/3/c-api/list.html#c.PyList_Insert
PyList_Insert raises bad internal call on insert NULL.

https://docs.python.org/3/c-api/list.html#c.PyList_GetItemRef

https://docs.python.org/3/c-api/list.html#c.PyList_GetItem

https://docs.python.org/3/c-api/list.html#c.PyList_GET_ITEM

Summary
----------------------

* `PyList_SetItem()`_ and `PyList_SET_ITEM()`_ *steal* references.
* `PyList_SetItem()`_ and `PyList_SET_ITEM()`_ behave differently when replacing an existing value.
* If `PyList_SetItem()`_ errors it will decrement the reference count of the given value.
  Possibly with surprising results.
* `Py_BuildValue()`_ increment reference counts and thus may leak.




.. _chapter_refcount_and_containers.dictionaries:

-----------------------
Dictionaries
-----------------------

TODO:

.. _chapter_refcount_and_containers.sets:

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

.. rubric:: Footnotes

.. [#] The official `Python documentation <https://docs.python.org/3/c-api/concrete.html>`_ categorises tuples and lists
    as *sequence objects* because they support the `Sequence Protocol <https://docs.python.org/3/c-api/sequence.html>`_
    and dictionaries and sets as *container objects* because they support the
    `Mapping Protocol <https://docs.python.org/3/c-api/mapping.html>`_.
    In this chapter I use looser language by describing all four as *containers*.

.. [#] ``Py_BuildValue`` can not create a set, only a tuple, list or dictionary.
