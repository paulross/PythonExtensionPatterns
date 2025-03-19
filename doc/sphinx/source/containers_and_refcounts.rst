.. moduleauthor:: Paul Ross <apaulross@gmail.com>
.. sectionauthor:: Paul Ross <apaulross@gmail.com>

.. highlight:: python
    :linenothreshold: 20

.. toctree::
    :maxdepth: 3

..
    Links, mostly to the Python documentation.
    Specific container links are just before the appropriate section.

.. _Py_BuildValue(): https://docs.python.org/3/c-api/arg.html#c.Py_BuildValue


.. index::
    pair: Documentation Lacunae; Containers

.. _chapter_containers_and_refcounts:

======================================
Containers and Reference Counts
======================================

This chapter looks in more detail of how the Python C API works with different containers,
such as ``tuple``, ``list``, ``set`` and ``dict`` [#]_.
It also clarifies and correct the Python documentation where that is inaccurate, incomplete or misleading.
This also shows where the Python C API has some undocumented failure modes, some of which can lead to undefined
behaviour.

This chapter includes examples and tests that you can step through to better understand the interplay
between the container and the objects in that container.

Of particular interest are *Setters*, *Getters* and the behaviour of ``Py_BuildValue`` for each of those
containers [#]_.


.. note::

    The Python documentation for the
    `Concrete Objects Layer <https://docs.python.org/3/c-api/concrete.html#concrete-objects-layer>`_
    has a general warning that passing ``NULL`` values into functions gives rise to undefined behaviour.
    This is not always the case and this chapter explores this in more detail than the official Python documentation.

---------------------------
Some Additional Terminology
---------------------------

As well as :ref:`chapter_refcount.new`, :ref:`chapter_refcount.stolen` and :ref:`chapter_refcount.borrowed`
described in the previous chapter some other *behaviors* of containers are worth defining when they interact
with objects.

.. index::
    single: Reference Counts; Discarded

.. _chapter_containers_and_refcounts.discarded:

Discarded References
---------------------------

This is when a container has a reference to an object but is required to replace it with another object.
In this case the container decrements the reference count of the original object before replacing it with the other
object.
This is to prevent a memory leak of the previous object.

.. warning::

    If the the replacement object is the **same** as the existing object then very bad things *might* happen.
    For example see the warning in `PyTuple_SetItem()`_
    :ref:`chapter_containers_and_refcounts.tuples.PyTuple_SetItem.replacement`.

.. index::
    single: Reference Counts; Abandoned

.. _chapter_containers_and_refcounts.abandoned:

Abandoned References
---------------------------

This is when a container has a reference to an object but is required to replace it with another object.
In this case the container *does not* decrement the reference count of the original object before replacing it with
the other object.
This *will* lead to a memory leak *unless* the replacement object is the same as the existing object.
Of course if the original reference is ``NULL`` there is no leak.

An example of this is ``PyTuple_SET_ITEM()``
:ref:`chapter_containers_and_refcounts.tuples.PyTuple_SET_ITEM.replacement`.

---------------------------
Exploring the CPython C API
---------------------------

The code in this chapter explores the CPython C API in several ways:

* C code that can be stepped through in the debugger.
  This code is in ``src/cpy/Containers/DebugContainers.c``.
  This uses ``asserts`` to check the results, particularly reference counts, so should always be compiled with
  ``-DDEBUG``.
  The tests are exercised by ``src/main.c``.
* Test code is in ``src/cpy/RefCount/cRefCount.c`` which is built into the Python module
  ``cPyExtPatt.cRefCount``.
  This can be run under ``pytest`` for multiple Python versions by ``build_all.sh``.
* A study of the Python source code.
* A review of the Python C API documentation.

.. note::

    The examples below use code that calls a function ``new_unique_string()``.
    This function is designed to create a new, unique,  ``PyObject`` (a string)
    that is never cached so always starts with a reference count of unity.
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

Here is an example test function that checks that `PyTuple_SetItem()`_ *steals* a reference:

.. code-block:: c

    /**
     * A function that checks whether a tuple steals a reference when using PyTuple_SetItem.
     * This can be stepped through in the debugger.
     * asserts are use for the test so this is expected to be run in DEBUG mode.
     */
    void dbg_PyTuple_SetItem_steals(void) {
        printf("%s():\n", __FUNCTION__);
        if (PyErr_Occurred()) {
            fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
            PyErr_Print();
            return;
        }
        assert(!PyErr_Occurred());
        Py_ssize_t ref_count;

        PyObject *container = PyTuple_New(1);
        assert(container);
        ref_count = Py_REFCNT(container);
        assert(ref_count == 1);

        PyObject *value = new_unique_string(__FUNCTION__, NULL);
        ref_count = Py_REFCNT(value);
        assert(ref_count == 1);

        if (PyTuple_SetItem(container, 0, value)) {
            assert(0);
        }
        ref_count = Py_REFCNT(value);
        assert(ref_count == 1);

        PyObject *get_item = PyTuple_GET_ITEM(container, 0);
        ref_count = Py_REFCNT(get_item);
        assert(ref_count == 1);

        Py_DECREF(container);
        /* NO as container deals with this. */
        /* Py_DECREF(value); */
        assert(!PyErr_Occurred());
    }

Firstly Tuples, I'll go into quite a lot of detail here because it is very similar to the
C API for lists which I'll cover with more brevity in a later section.

.. _chapter_containers_and_refcounts.tuples:

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

.. _chapter_containers_and_refcounts.tuples.PyTuple_SetItem:

.. index::
    single: PyTuple_SetItem()
    single: Tuple; PyTuple_SetItem()

``PyTuple_SetItem()``
---------------------

`PyTuple_SetItem()`_ (a C function) inserts an object into a tuple with error checking.
This function returns non-zero on error, these are described below in
:ref:`chapter_containers_and_refcounts.tuples.PyTuple_SetItem.failures`.
The failure of `PyTuple_SetItem()`_ has serious consequences for the value
that is intended to be inserted.

``PyTuple_SetItem()`` Basic Usage
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

* C: in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyTuple_SetItem_steals``
* CPython: in ``src/cpy/RefCount/cRefCount.c``:
    * ``test_PyTuple_SetItem_steals()``
* Python: in ``tests/unit/test_c_ref_count.py``:
    * ``test_PyTuple_SetItem_steals()``.

Whilst we are here this is an example of testing the behaviour by manipulating reference counts which we then check
with ``assert()``.
The rationale is that you can't check reference counts after an object is destroyed.
For example:

.. code-block:: c

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    assert(Py_REFCNT(value) == 1);
    Py_DECREF(value);
    /* This will fail, the reference count will have an arbitrary value. */
    assert(Py_REFCNT(value) == 0);

Once an object has been free'd you can not rely on the reference count field.
Instead, deliberately increment the reference count before the critical section and check it afterwards.
For example:

.. code-block:: c

    PyObject *container = PyTuple_New(1);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    assert(Py_REFCNT(value) == 1);
    PyTuple_SetItem(container, 0, value);
    assert(Py_REFCNT(value) == 1);
    /* Increment the reference count so we can see destruction. */
    Py_INCREF(value);
    assert(Py_REFCNT(value) == 2);
    /* Check destruction. */
    Py_DECREF(container);
    assert(Py_REFCNT(value) == 1);
    /* Clean up. */
    Py_DECREF(value);

.. index::
    single: PyTuple_SetItem(); Replacement
    single: Documentation Lacunae; PyTuple_SetItem() Replacement

.. _chapter_containers_and_refcounts.tuples.PyTuple_SetItem.replacement:

``PyTuple_SetItem()`` Replacement
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

.. warning::

    What happens if you use `PyTuple_SetItem()`_ to replace a value with the *same* value?
    For example:

    .. code-block:: c

        PyObject *container = PyTuple_New(1);
        /* container ref count will be 1. */
        PyObject *value = new_unique_string(__FUNCTION__, NULL);
        /* value ref count will be 1. */
        PyTuple_SetItem(container, 0, value);
        /* value ref count is still 1 as it has been *stolen*. */

    Now repeat the replacement:

    .. code-block:: c

        /* Repeating the same call will only lead to trouble later on (maybe). */
        PyTuple_SetItem(container, 0, value);
        /* And this will segfault as, during execution, it will
         * try to decrement a value that does not exist. */
        Py_DECREF(container);
        /* So what is going on? */

    What is happening is that the second time `PyTuple_SetItem()`_ is called it decrements the reference count of the
    existing member that happens to be ``value``.
    In this case this brings ``value``'s reference count from one down to zero
    At that point ``value`` is free'd.
    Then `PyTuple_SetItem()`_ blithely sets ``value`` which is now, likely, garbage.

    This is not described in the Python documentation.

    A simple change to `PyTuple_SetItem()`_ would prevent this from producing undefined behaviour by checking if the
    replacement is the same as the existing value.

    `PyTuple_SET_ITEM()`_ does not exhibit this problem as it *abandons* values rather than *discarding* them.

For code and tests see:

* C: in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyTuple_SetItem_steals_replace``
    * ``dbg_PyTuple_SetItem_replace_with_same``
* CPython: in ``src/cpy/RefCount/cRefCount.c``:
    * ``test_PyTuple_SetItem_steals_replace``
    *  ``test_PyTuple_SetItem_replace_same``
* Python: in ``tests.unit.test_c_ref_count``
    * ``test_PyTuple_SetItem_steals_replace()``
    * ``test_PyTuple_SetItem_replace_same()``

.. index::
    single: PyTuple_SetItem(); Failures
    pair: Documentation Lacunae; PyTuple_SetItem() Failures

.. _chapter_containers_and_refcounts.tuples.PyTuple_SetItem.failures:

``PyTuple_SetItem()`` Failures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

`PyTuple_SetItem()`_ can return a non-zero error code for these reasons:

* The given container is not a tuple.
* The index is out of range; index < 0 or index >= tuple length (negative indexes are not allowed).

.. warning::

    A consequence of failure is that the value being inserted will be decref'd.
    For example this code will segfault:

    .. code-block:: c

        PyObject *container = PyTuple_New(1); /* Reference count will be 1. */
        PyObject *value = new_unique_string(__FUNCTION__, NULL); /* Ref count will be 1. */
        PyTuple_SetItem(container, 1, value); /* Index out of range. */
        Py_DECREF(value); /* value has already been decref'd and free'd so this will SIGSEGV */

    This is not described in the Python documentation.

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
    It would seem like `PyTuple_SET_ITEM()`_ would be enough.

.. _chapter_containers_and_refcounts.tuples.PyTuple_SET_ITEM:

.. index::
    single: PyTuple_SET_ITEM()
    single: Tuple; PyTuple_SET_ITEM();
    pair: Documentation Lacunae; PyTuple_SET_ITEM() Replacement

``PyTuple_SET_ITEM()``
----------------------

`PyTuple_SET_ITEM()`_ is a function like macro that inserts an object into a tuple without any error checking
(see :ref:`chapter_containers_and_refcounts.tuples.PyTuple_SET_ITEM.failures` below).
However type checking is performed as an assertion if Python is built in
`debug mode <https://docs.python.org/3/using/configure.html#debug-build>`_ or
`with assertions <https://docs.python.org/3/using/configure.html#cmdoption-with-assertions>`_.
Because of the absence of checks, it is slightly faster than `PyTuple_SetItem()`_ .
This is usually used on newly created tuples.

Importantly `PyTuple_SET_ITEM()`_ behaves **differently** to `PyTuple_SetItem()`_
when replacing another object.

``PyTuple_SET_ITEM()`` Basic Usage
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

* C: in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyTuple_PyTuple_SET_ITEM_steals()``
* CPython: in ``src/cpy/RefCount/cRefCount.c``:
    * ``test_PyTuple_PyTuple_SET_ITEM_steals()``
* Python: in ``tests.unit.test_c_ref_count``
    * ``test_PyTuple_PyTuple_SET_ITEM_steals()``

.. index::
    single: PyTuple_SET_ITEM(); Replacement

.. _chapter_containers_and_refcounts.tuples.PyTuple_SET_ITEM.replacement:

``PyTuple_SET_ITEM()`` Replacement
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

`PyTuple_SET_ITEM()`_ **differs** from `PyTuple_SetItem()`_ when replacing an existing
element in a tuple as the original reference will be leaked.
This is because `PyTuple_SET_ITEM()`_ *abandons* the previous reference
(see :ref:`chapter_containers_and_refcounts.abandoned`):

.. code-block:: c

    PyObject *container = PyTuple_New(1); /* Reference count will be 1. */
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL); /* Ref count will be 1. */
    PyTuple_SET_ITEM(container, 0, value_a); /* Ref count of value_a will be 1. */
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL); /* Ref count will be 1. */
    PyTuple_SET_ITEM(container, 0, value_b);
    assert(Py_REFCNT(value_a) == 1);
    /* Ref count of value_b will be 1,
     * value_a ref count will still be at 1 and value_a will be leaked unless decref'd. */

.. note::

    Because `PyTuple_SET_ITEM()`_ *abandons* the previous reference it does not have the problem with
    undefined behaviour that `PyTuple_Set_Item()`_ has.
    For that see the warning about undefined behaviour in `PyTuple_Set_Item()`_
    :ref:`chapter_containers_and_refcounts.tuples.PyTuple_SetItem.replacement`.

For code and tests see:

* C: in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyTuple_SET_ITEM_steals_replace()``
    * ``dbg_PyTuple_SET_ITEM_replace_with_same()``
* CPython: in ``src/cpy/RefCount/cRefCount.c``:
    * ``test_PyTuple_SetItem_steals_replace()``
    * ``test_PyTuple_SET_ITEM_replace_same()``
* Python: in ``tests/unit/test_c_ref_count.py``:
    * ``test_PyTuple_SetItem_steals_replace``
    * ``test_PyTuple_SET_ITEM_replace_same``.

.. index::
    single: PyTuple_SET_ITEM(); Failures

.. _chapter_containers_and_refcounts.tuples.PyTuple_SET_ITEM.failures:

.. index::
    pair: Documentation Lacunae; PyTuple_SET_ITEM() Failures

``PyTuple_SET_ITEM()`` Failures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

`PyTuple_SET_ITEM()`_ minimises the error checking that `PyTuple_SetItem()`_ does, so:

* It does not check if the given container is a Tuple.
* It does not check if the index in range.

If either of those is wrong then `PyTuple_SET_ITEM()`_ is capable of writing to arbitrary
memory locations, and the result is likely to be tragic, mostly undefined behaviour
and/or memory corruption.

Setting and Replacing ``NULL``
------------------------------

This looks at what happens when setting or replacing a ``NULL`` pointer in a tuple.
Both `PyTuple_SetItem()`_ and `PyTuple_SET_ITEM()`_ behave the same way.

.. index::
    single: PyTuple_SetItem(); Setting NULL
    single: PyTuple_SET_ITEM(); Setting NULL
    pair: Documentation Lacunae; PyTuple Setting NULL

Setting ``NULL``
^^^^^^^^^^^^^^^^

Setting a ``NULL`` will not cause an error:

.. code-block:: c

    PyObject *container = PyTuple_New(1);
    assert(!PyErr_Occurred());
    PyTuple_SetItem(container, 0, NULL);
    assert(!PyErr_Occurred());

For code and tests see:

* C: in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyTuple_SetIem_NULL()``
* CPython: in ``src/cpy/RefCount/cRefCount.c``:
    * ``test_PyTuple_SetItem_NULL()``
* Python: in ``tests/unit/test_c_ref_count.py``
    * ``test_PyTuple_SetItem_NULL()``.

And:

.. code-block:: c

    PyObject *container = PyTuple_New(1);
    assert(!PyErr_Occurred());
    PyTuple_SET_ITEM(container, 0, NULL);
    assert(!PyErr_Occurred());

For code and tests see:

* C: in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyTuple_SET_ITEM_NULL()``
* CPython: in ``src/cpy/RefCount/cRefCount.c``:
    * ``test_PyTuple_SET_ITEM_NULL()``
* Python: in ``tests/unit/test_c_ref_count.py``
    * ``test_PyTuple_SET_ITEM_NULL()``.

.. index::
    single: PyTuple_SetItem(); Replacing NULL
    single: PyTuple_SET_ITEM(); Replacing NULL
    pair: Documentation Lacunae; PyTuple Replacing NULL

Replacing ``NULL``
^^^^^^^^^^^^^^^^^^

Replacing a ``NULL`` will not cause an error, the original value is *abandoned*
(see :ref:`chapter_containers_and_refcounts.abandoned`) and the replaced value reference is *stolen*
(see :ref:`chapter_refcount.stolen`):

.. code-block:: c

    PyObject *container = PyTuple_New(1);
    PyTuple_SetItem(container, 0, NULL);
    PyObject *value = new_unique_string(__FUNCTION__, NULL); /* Ref Count of value is 1. */
    PyTuple_SetItem(container, 0, value); /* Ref Count of value is still 1. */

For code and tests see:

* C: in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyTuple_SetIem_NULL_SetIem``
* CPython: in ``src/cpy/RefCount/cRefCount.c``:
    * ``test_PyTuple_SetItem_NULL_SetIem``
* Python: in ``tests/unit/test_c_ref_count.py``:
    * ``test_PyTuple_SetItem_NULL_SetIem()``.

And:

.. code-block:: c

    PyObject *container = PyTuple_New(1);
    PyTuple_SET_ITEM(container, 0, NULL);
    PyObject *value = new_unique_string(__FUNCTION__, NULL); /* Ref Count of value is 1. */
    PyTuple_SET_ITEM(container, 0, value); /* Ref Count of value is still 1. */

For code and tests see:

* C: in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyTuple_SET_ITEM_NULL_SET_ITEM()``
* CPython: in ``src/cpy/RefCount/cRefCount.c``:
    * ``test_PyTuple_SET_ITEM_NULL_SET_ITEM()``
* Python: in ``tests/unit/test_c_ref_count.py``:
    * ``test_PyTuple_SET_ITEM_NULL_SET_ITEM()``.

.. _chapter_containers_and_refcounts.tuples.PyTuple_Pack:

.. index::
    single: PyTuple_Pack()
    single: Tuple; PyTuple_Pack()

``PyTuple_Pack()``
------------------

`PyTuple_Pack()`_ takes a length and a variable argument list of PyObjects.
Each of those PyObjects reference counts will be incremented.
In that sense it behaves as `Py_BuildValue()`_.

.. note::
    `PyTuple_Pack()`_ is implemented as a separate low level routine, it does not invoke
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

* C: in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyTuple_PyTuple_Pack``
* CPython: in ``src/cpy/RefCount/cRefCount.c``:
    * ``test_PyTuple_Py_PyTuple_Pack``
* Python: in ``tests/unit/test_c_ref_count.py``:
    * ``test_PyTuple_Py_PyTuple_Pack()``.

.. _chapter_containers_and_refcounts.tuples.Py_BuildValue:

.. index::
    single: Tuple; Py_BuildValue()

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

* C: in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyTuple_Py_BuildValue()``
* CPython: in ``src/cpy/RefCount/cRefCount.c``:
    * ``test_PyTuple_Py_BuildValue``
* Python: in ``tests/unit/test_c_ref_count.py``:
    * ``test_PyTuple_Py_BuildValue()``.

.. index::
    single: PyTuple_GetItem()
    single: Tuple; PyTuple_GetItem()
    single: PyTuple_GET_ITEM()
    single: Tuple; PyTuple_GET_ITEM()
    pair: Getters; Tuple

.. _chapter_containers_and_refcounts.tuples.Getters:

.. index::
    single: Tuple; Getters
    single: Tuple; PyTuple_GetItem()
    single: Tuple; PyTuple_GET_ITEM()

Tuple Getters
---------------------

There are these APIs for getting an item from a tuple:

* `PyTuple_GetItem()`_, it returns a borrowed reference and will error
  if the supplied container is not tuple or the index is negative or out of range.
* `PyTuple_GET_ITEM()`_, it returns a borrowed reference and there is
  no error checking for the container being a tuple or the index being in range.
  The type checking is performed as an assertion if Python is built in
  `debug mode <https://docs.python.org/3/using/configure.html#debug-build>`_ or
  `with assertions <https://docs.python.org/3/using/configure.html#cmdoption-with-assertions>`_.
  If not the results are undefined.

.. index:: single: Tuple; API Summary

Summary
----------------------

* `PyTuple_SetItem()`_ and `PyTuple_SET_ITEM()`_ *steal* references.
* `PyTuple_SetItem()`_ and `PyTuple_SET_ITEM()`_ behave differently when replacing an existing, different, value.
* `PyTuple_SetItem()`_ and `PyTuple_SET_ITEM()`_ behave differently when replacing the *same* value.
  In particular `PyTuple_SetItem()`_ can produce undefined behaviour.
* If `PyTuple_SetItem()`_ errors it will decrement the reference count of the given value which can produce undefined
  behaviour.
* `PyTuple_Pack()`_ and `Py_BuildValue()`_ increment reference counts and thus may leak.

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
    pair: Documentation Lacunae; Lists

.. _chapter_containers_and_refcounts.lists:

-----------------------
Lists
-----------------------

.. index::
    single: PyList_SetItem()
    single: List; PyList_SetItem()
    single: PyList_SET_ITEM()
    single: List; PyList_SET_ITEM()

.. _chapter_containers_and_refcounts.lists.PyList_SetItem:

.. _chapter_containers_and_refcounts.lists.PyList_SET_ITEM:

.. _chapter_containers_and_refcounts.lists.PyList_SET_ITEM.failures:

.. index::
    pair: Documentation Lacunae; PyList_SetItem()
    pair: Documentation Lacunae; PyList_SET_ITEM()

``PyList_SetItem()`` and ``PyList_SET_ITEM()``
----------------------------------------------

`PyList_SetItem()`_ and `PyList_SET_ITEM()`_ behave identically to their equivalents `PyTuple_SetItem()`_
(link :ref:`chapter_containers_and_refcounts.tuples.PyTuple_SetItem`)
and `PyTuple_SET_ITEM()`_ (link :ref:`chapter_containers_and_refcounts.tuples.PyTuple_SET_ITEM`).

Note that, as with tuples, `PyList_SetItem()`_ and `PyList_SET_ITEM()`_ behave differently on replacement of values
(see :ref:`chapter_containers_and_refcounts.tuples.PyTuple_SET_ITEM.replacement`).
The Python documentation on `PyList_SET_ITEM()`_ correctly identifies when a leak can occur
(unlike `PyTuple_SET_ITEM()`_).

On replacement with `PyList_SetItem()`_ heed the warning in
:ref:`chapter_containers_and_refcounts.tuples.PyTuple_SetItem.replacement` as `PyList_SetItem()`_ holds the same danger.

`Py_BuildValue()`_ also behaves identically, as far as reference counts are concerned, with Lists as it does with
Tuples (see :ref:`chapter_containers_and_refcounts.tuples.Py_BuildValue`).


.. index::
    single: PyList_Append()
    single: List; PyList_Append()

.. _chapter_containers_and_refcounts.lists.PyList_Append:

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
    single: List;  PyList_Insert()
    pair: Documentation Lacunae; PyList_Insert()

.. _chapter_containers_and_refcounts.lists.PyList_Insert:

``PyList_Insert()``
---------------------

`PyList_Insert()`_ (a C function) inserts an object before a specific index in a list with error checking.
This increments the reference count of the given value which will be decremented on container destruction.

`PyList_Insert()`_ is equivalent to ``list.insert(...)``, for example:

.. code-block:: python

    >>> l = []
    >>> l.insert(123, "Hello")
    >>> l.insert(-123, "World")
    >>> l
    ['World', 'Hello']

`PyList_Insert()`_ can fail for two reasons:

* The given container is not a list.
* The given value is NULL.

.. note::

    `PyList_Insert()`_ does not fail because of any value of the index, either positive or negative (this is not
    identified in the Python documentation).

On failure the reference count of value is unchanged, `PyList_Insert()`_ returns -1, and a ``SystemError`` is set with
the text "bad argument to internal function".

For code and tests, including failure modes, see:

* C: ``dbg_PyList_Insert...`` in ``src/cpy/Containers/DebugContainers.c``.
* CPython: ``test_PyList_Insert...`` in ``src/cpy/RefCount/cRefCount.c``.
* Python: ``tests.unit.test_c_ref_count.test_PyList_Insert`` etc.

..
    This note and code blocks are quite big in latex so page break here. Now commented out.

    .. raw:: latex

        [Continued on the next page]

        \pagebreak

.. note::

    The Python documentation does not mention (but implies) that if the index is greater than the list length then the
    value is appended to the list.

    For example (``dbg_PyList_Insert_Is_Truncated()`` in ``src/cpy/Containers/DebugContainers.c``):

    .. code-block:: c

        PyObject *container = PyList_New(0);
        PyObject *value = new_unique_string(__FUNCTION__, NULL);
        // Insert before index 4
        if (PyList_Insert(container, 4L, value)) {
            assert(0);
        }
        assert(Py_REFCNT(value) == 2);
        PyObject *get_item;
        // PyList_Insert at 4 actually inserts at 0.
        assert(PyList_GET_SIZE(container) == 1L);
        get_item = PyList_GET_ITEM(container, 0L);
        assert(get_item == value);

.. note::

    Neither the Python documentation for `PyList_Insert()`_ or ``list.insert()`` does not make this clear that index
    can be negative in which case the index is calculated from the end.
    If that index calculation is less than zero it is truncated to zero.

    For example (``dbg_PyList_Insert_Negative_Index()`` in ``src/cpy/Containers/DebugContainers.c``):

    .. code-block:: c

        PyObject *container = PyList_New(0);
        PyObject *value = new_unique_string(__FUNCTION__, NULL);
        // Insert at end
        if (PyList_Insert(container, -1L, value)) {
            assert(0);
        }
        assert(Py_REFCNT(value) == 2);
        PyObject *get_item;
        // PyList_Insert at -1 actually inserts at 0.
        assert(PyList_GET_SIZE(container) == 1L);
        get_item = PyList_GET_ITEM(container, 0L);
        assert(get_item == value);

.. index::
    single: List; Py_BuildValue()

.. _chapter_containers_and_refcounts.lists.Py_BuildValue:

``Py_BuildValue()``
-------------------

As with tuples :ref:`chapter_containers_and_refcounts.tuples.Py_BuildValue` is a very convenient way to
create lists.
``Py_BuildValue("[O]", value);`` will increment the refcount of value and this can, potentially, leak.

.. index::
    single: PyList_GetItem()
    single: List; PyList_GetItem()
    single: PyList_GET_ITEM()
    single: List; PyList_GET_ITEM()
    single: PyList_GetItemRef()
    single: List; PyList_GetItemRef()
    pair: Getters; List
    pair: Getters; List

.. _chapter_containers_and_refcounts.lists.Getters:

List Getters
---------------------

There are these APIS for getting an item from a list:

* `PyList_GetItem()`_ This is very similar to `PyTuple_GetItem()`_. It returns a borrowed reference and will error
  if the supplied container is not list or the index is negative or out of range.
* `PyList_GET_ITEM()`_ This is very similar to `PyTuple_GET_ITEM()`_. It returns a borrowed reference and there is
  no error checking for the index being in range.
  The type checking is performed as an assertion if Python is built in
  `debug mode <https://docs.python.org/3/using/configure.html#debug-build>`_ or
  `with assertions <https://docs.python.org/3/using/configure.html#cmdoption-with-assertions>`_.
  If not the results are undefined.
* `PyList_GetItemRef()`_ [From Python 3.13 onwards].
  Like `PyList_GetItem()`_ but his returns a new *strong* (i.e. incremented) reference to the existing object.

.. index:: single: List; API Summary

Summary
----------------------

* `PyList_SetItem()`_ and `PyList_SET_ITEM()`_ *steal* references.
* `PyList_SetItem()`_ and `PyList_SET_ITEM()`_ behave differently when replacing an existing, different, value.
* `PyList_SetItem()`_ and `PyList_SET_ITEM()`_ behave differently when replacing the *same* value.
  In particular `PyList_SetItem()`_ can produce undefined behaviour.
* If `PyList_SetItem()`_ errors it will decrement the reference count of the given value which can produce undefined
  behaviour.
* `PyList_Append()`_ Increments the reference count of the given object and thus may leak.
* `PyList_Insert()`_ Increments the reference count of the given object and thus may leak.
* `Py_BuildValue()`_ Increments the reference count of the given object and thus may leak.


.. Links, mostly to the Python documentation:

.. Setters

.. _PyDict_SetItem(): https://docs.python.org/3/c-api/dict.html#c.PyDict_SetItem
.. _PyDict_SetDefault(): https://docs.python.org/3/c-api/dict.html#c.PyDict_SetDefault
.. _PyDict_SetDefaultRef(): https://docs.python.org/3/c-api/dict.html#c.PyDict_SetDefaultRef

.. Getters

.. _PyDict_GetItem(): https://docs.python.org/3/c-api/dict.html#c.PyDict_GetItem
.. _PyDict_GetItemRef(): https://docs.python.org/3/c-api/dict.html#c.PyDict_GetItemRef
.. _PyDict_GetItemWithError(): https://docs.python.org/3/c-api/dict.html#c.PyDict_GetItemWithError

.. Deleters

.. _PyDict_Pop(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Pop
.. _PyDict_DelItem(): https://docs.python.org/3/c-api/dict.html#c.PyDict_DelItem

.. Iterators

.. _PyDict_Items(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Items
.. _PyDict_Keys(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Keys
.. _PyDict_Values(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Values
.. _PyDict_Next(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Next

.. Other

.. _PyDict_Check(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Check
.. _PyObject_Hash(): https://docs.python.org/3/c-api/object.html#c.PyObject_Hash

.. index::
    single: Dictionary
    pair: Documentation Lacunae; Dictionaries


.. _chapter_containers_and_refcounts.dictionaries:

-----------------------
Dictionaries
-----------------------

This section describes how reference counts are affected when building and accessing dictionaries.

.. index::
    single: PyDict_SetItem()
    single: Dictionary; PyDict_SetItem()

.. _chapter_containers_and_refcounts.dictionaries.setitem:

``PyDict_SetItem()``
--------------------

The Python documentation for `PyDict_SetItem()`_ is incomplete.
`PyDict_SetItem()`_ changes the key and value reference counts according to these rules:

* If the key exists in the dictionary then key's reference count remains the same.
* If the key does *not* exist in the dictionary then its reference count will be incremented.
* The value's reference count will always be incremented.
* If the key exists in the dictionary then the previous value reference count will be decremented before the value
  is replaced by the new value (and the new value reference count is incremented).
  See :ref:`chapter_containers_and_refcounts.discarded`.
  If the key exists in the dictionary and the value is the same then this means, effectively, that reference counts of
  both key and value remain unchanged.

.. warning::

    If either the key or the value are NULL this will segfault.
    See ``dbg_PyDict_SetItem_NULL_key()`` and ``dbg_PyDict_SetItem_NULL_value()`` in
    ``src/cpy/Containers/DebugContainers.c``.

This code illustrates `PyDict_SetItem()`_ with ``assert()`` showing the reference count:

.. code-block:: c

    PyObject *container = PyDict_New();
    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    assert(Py_REFCNT(key) == 1);
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
    assert(Py_REFCNT(key) == 2);
    assert(Py_REFCNT(value_a) == 2);
    assert(Py_REFCNT(value_b) == 1);
    if (PyDict_SetItem(container, key, value_b)) {
        assert(0);
    }
    assert(Py_REFCNT(key) == 2);
    assert(Py_REFCNT(value_a) == 1);
    assert(Py_REFCNT(value_b) == 2);

Now replace the value with the same value, reference counts remain the same:

.. code-block:: c

    /* Replace with the same value for the key. */
    if (PyDict_SetItem(container, key, value_b)) {
        assert(0);
    }
    assert(Py_REFCNT(key) == 2);
    assert(Py_REFCNT(value_a) == 1);
    assert(Py_REFCNT(value_b) == 2);

.. _chapter_containers_and_refcounts.dictionaries.setitem.failure:

``PyDict_SetItem()`` Failure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

`PyDict_SetItem()`_ can fail for the following reasons:

* The container is not a dictionary (or a sub-class of a dictionary, see `PyDict_Check()`_).
* The key is not hashable (`PyObject_Hash()`_ returns -1).
* If either the key or the value is NULL this will cause a SIGSEGV (or some other disaster).
  These are checked with asserts if Python is built in
  `debug mode <https://docs.python.org/3/using/configure.html#debug-build>`_ or
  `with assertions <https://docs.python.org/3/using/configure.html#cmdoption-with-assertions>`_.

For code and tests see:

* C: in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyDict_SetItem_*()``
* CPython: in ``src/cpy/RefCount/cRefCount.c``:
    * ``test_PyDict_SetItem_*()``
* Python: in ``tests/unit/test_c_ref_count.py``:
    * ``test_PyDict_SetItem_increments()``

.. note::

    In ``src/cpy/Containers/DebugContainers.c`` there are failure tests that cause a SIGSEGV if ``ACCEPT_SIGSEGV``
    is non zero.
    ``ACCEPT_SIGSEGV`` is defined in ``src/cpy/Containers/DebugContainers.h``.

.. index::
    pair: Documentation Lacunae; PyDict_SetDefault()

.. _chapter_containers_and_refcounts.dictionaries.setdefault:

``PyDict_SetDefault()``
------------------------

`PyDict_SetDefault()`_ is equivalent to the Python method
`dict.setdefault() <https://docs.python.org/3/library/stdtypes.html#dict.setdefault>`_ in Python.
The C function signature is:

.. code-block:: c

    PyObject *PyDict_SetDefault(PyObject *p, PyObject *key, PyObject *defaultobj);

The idea is that if the key exists then the appropriate value is returned and the default value is unused.
If the key does *not* exist then the default value is inserted into the dictionary and returned.

If the Default Value is Unused
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the key already exists in the dictionary the reference counts of the key, existing value and default value are
unchanged so the return value is a borrowed reference (see :ref:`chapter_refcount.borrowed`).

If the Default Value is Used
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the key does *not* exist in the dictionary the reference counts of the key and default value are incremented.

These reference count changes are not particularly clear from the official Python documentation.

For code and tests see:

* C, in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyDict_SetDefault_default_unused()``
    * ``dbg_PyDict_SetDefault_default_used()``
* CPython, in ``src/cpy/RefCount/cRefCount.c``.
    ``test_PyDict_SetDefault_default_unused()``
    ``test_PyDict_SetDefault_default_used()``
* Python, pytest, in ``tests.unit.test_c_ref_count``:
    * ``test_PyDict_SetDefault_default_unused()``
    * ``test_PyDict_SetDefault_default_used()``


.. index::
    single: PyDict_SetDefaultRef()
    single: Dictionary; PyDict_SetDefaultRef()
    pair: Documentation Lacunae; PyDict_SetDefaultRef()

``PyDict_SetDefaultRef()`` [Python 3.13+]
-----------------------------------------

`PyDict_SetDefaultRef()`_ sets a default value and retrieves the actual value.
This is new in Python 3.13+.
The C function signature is:

.. code-block:: c

    int PyDict_SetDefaultRef(
        PyObject *dictionary, PyObject *key, PyObject *default_value, PyObject **result
    );

``*result``
^^^^^^^^^^^

Any previous ``*result`` is always *abandoned* (see :ref:`chapter_containers_and_refcounts.abandoned`).
To emphasise, there is no decrementing the reference count of the existing value (if any).
This is important as the following code snippet shows:

.. code-block:: c

    PyObject *result; /* Refers to an arbitrary memory location. */
    /* Now if PyDict_SetDefaultRef() were to attempt to Py_DECREF(result)
     * the results will be undefined.
    */
    PyDict_SetDefaultRef(container, key, default_value, &result);

.. index::
    single: Dictionary; PyDict_SetDefaultRef(); Key Exists

Key Exists
^^^^^^^^^^

If the key already exists in the dictionary `PyDict_SetDefaultRef()`_ returns 1.
The reference counts are changed as follows:

- key: unchanged.
- value: incremented by one
- default_value: unchanged.

``*result`` is equal to the stored value.

For example:

.. code-block:: c

    PyObject *container = PyDict_New();
    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    PyObject *val = new_unique_string(__FUNCTION__, NULL);
    /* At this point the reference counts are:
     * key: 1
     * val: 1
     */
    // Set the key/value
    PyDict_SetItem(container, key, val);
    /* At this point the reference counts are:
     * key: 2
     * val: 2
     */
    // Create a default value and a result.
    PyObject *default_value = new_unique_string(__FUNCTION__, NULL);
    PyObject *result = NULL;
    /* At this point the reference counts are:
     * default_value: 1
     * result: N/A
     */
    PyDict_SetDefaultRef(container, key, default_value, &result);
    /* Now the reference counts are:
     * key: 2
     * val: 3
     * default_value: 1
     * result: 3 as it equals val.
     */
    Py_DECREF(container);
    /* Now the reference counts are:
     * key: 1
     * val: 2
     * default_value: 1
     * result: 2 as it equals val.
     */


For code and tests see:

* C, in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyDict_SetDefaultRef_default_unused()``
* CPython, in ``src/cpy/RefCount/cRefCount.c``.
    ``test_PyDict_SetDefaultRef_default_unused()``
* Python, pytest, in ``tests.unit.test_c_ref_count``:
    * ``test_PyDict_SetDefaultRef_default_unused()``

.. index::
    single: Dictionary; PyDict_SetDefaultRef(); Key Does not Exist

Key Does not Exist
^^^^^^^^^^^^^^^^^^^

If the key does not exists in the dictionary `PyDict_SetDefaultRef()`_ returns 0.
The reference counts are changed as follows:

- key: incremented by one.
- default_value: incremented by *two*. The rationale is one increment as the default_value is inserted into
  the dictionary then a second increment as the default_value is 'returned' as ``*result``.

``*result`` is equal to the default_value.

For example:

.. code-block:: c

    PyObject *container = PyDict_New();
    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    // Create a default value and a result.
    PyObject *default_value = new_unique_string(__FUNCTION__, NULL);
    PyObject *result = NULL;
    /* At this point the reference counts are:
     * key: 1
     * default_value: 1
     * result: N/A
     */
    PyDict_SetDefaultRef(container, key, default_value, &result);
    /* Now the reference counts are:
     * key: 2
     * default_value: 3
     * result: 3 as it equals default_value.
     */
    Py_DECREF(container);
    /* Now the reference counts are:
     * key: 1
     * default_value: 2
     * result: 2 as it equals default_value.
     */

These reference count changes are not particularly clear from the official Python documentation.

For code and tests see:

* C, in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyDict_SetDefaultRef_default_used()``
* CPython, in ``src/cpy/RefCount/cRefCount.c``.
    ``test_PyDict_SetDefaultRef_default_used()``
* Python, pytest, in ``tests.unit.test_c_ref_count``:
    * ``test_PyDict_SetDefaultRef_default_used()``

.. index::
    single: Dictionary; PyDict_SetDefaultRef(); Failure

Failure
^^^^^^^

.. todo::

    PyDict_SetDefaultRef() failure modes.


.. index::
    single: Dictionary; PyDict_GetItem()
    pair: Getters; Dictionary

``PyDict_GetItem()``
-----------------------------------------

`PyDict_GetItem()`_ returns a borrowed reference (:ref:`chapter_refcount.borrowed`) to an existing value or ``NULL`` if
the key does not exist in the dictionary.

.. warning::

    If the key is ``NULL`` this will segfault.
    See ``dbg_PyDict_GetItem_key_NULL()`` in ``src/cpy/Containers/DebugContainers.c``.


.. index::
    single: Dictionary; PyDict_GetItemRef()
    pair: Getters; Dictionary

``PyDict_GetItemRef()`` [Python 3.13+]
-----------------------------------------

`PyDict_GetItemRef()`_ gets a new strong reference to a value from a dictionary.

The C signature is:

.. code-block:: c

    int PyDict_GetItemRef(PyObject *p, PyObject *key, PyObject **result);

Key is Present
^^^^^^^^^^^^^^

If the key is in the dictionary then increment the reference count of the value and set ``*result`` to the value.
The reference count of the key is unchanged.
The function returns 1.

For code and tests see:

* C, in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyDict_GetItemRef()``
* CPython, in ``src/cpy/RefCount/cRefCount.c``.
    ``test_PyDict_SetDefaultRef_default_used()``
* Python, pytest, in ``tests.unit.test_c_ref_count``:
    * ``test_PyDict_SetDefaultRef_default_used()``

.. index::
    single: Dictionary; PyDict_SetDefaultRef(); Failure
    pair: Setters; Dictionary

Failure
^^^^^^^

.. todo::

    PyDict_GetItemRef() failure modes.

.. index::
    single: Dictionary; PyDict_Pop()

``PyDict_Pop()`` [Python 3.13+]
-----------------------------------------

`PyDict_Pop()`_ removes a specific value.
This is new in Python 3.13+.
The C function signature is:

.. code-block:: c

    int int PyDict_Pop(PyObject *p, PyObject *key, PyObject **result);

``*result``
^^^^^^^^^^^

Any previous ``*result`` is always *abandoned* (see :ref:`chapter_containers_and_refcounts.abandoned`).
To emphasise, there is no decrementing the reference count of the existing value  (if any).
This is important as the following code snippet shows:

.. code-block:: c

    PyObject *result; /* Refers to an arbitrary memory location. */
    /* Now if PyDict_Pop() were to attempt to Py_DECREF(result)
     * the results will be undefined.
    */
    PyDict_Pop(container, key, &result);


.. index::
    single: Dictionary; PyDict_Pop(); Key Exists

Key Exists
^^^^^^^^^^

If the key already exists in the dictionary `PyDict_Pop()`_ returns 1.
The reference counts are changed as follows:

- key: decremented by one
- value: unchanged.

``*result`` is equal to the stored value.

For example:

.. code-block:: c

    PyObject *container = PyDict_New();
    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    PyObject *val = new_unique_string(__FUNCTION__, NULL);
    /* At this point the reference counts are:
     * key: 1
     * val: 1
     */
    // Set the key/value
    PyDict_SetItem(container, key, val);
    /* At this point the reference counts are:
     * key: 2
     * val: 2
     */
    PyObject *result;
    PyDict_Pop(container, key, &result);
    /* Now the reference counts are:
     * key: 1
     * val: 2
     * result: 2 as it equals val.
     */

.. code-block:: c

For code and tests see:

* C, in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyDict_Pop_key_present()``
* CPython, in ``src/cpy/RefCount/cRefCount.c``.
    ``test_PyDict_Pop_key_present()``
* Python, pytest, in ``tests.unit.test_c_ref_count``:
    * ``test_PyDict_Pop_key_present()``

.. index::
    single: Dictionary; PyDict_Pop(); Key Does not Exist

Key Does not Exist
^^^^^^^^^^^^^^^^^^^

If the key does not exists in the dictionary `PyDict_Pop()`_ returns 0.
The reference counts are changed as follows:

- key: unchanged.

``*result`` is set to ``NULL``, any previous value is *abandoned*
(see :ref:`chapter_containers_and_refcounts.abandoned`).

For example:

.. code-block:: c

    PyObject *container = PyDict_New();
    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    // Create a default value and a result.
    PyObject *dummy_value = new_unique_string(__FUNCTION__, NULL);
    PyObject *result = dummy_value;
    /* At this point the reference counts are:
     * key: 1
     * dummy_value: 1
     * result: 1 as it is equal to dummy_value.
     */
    PyDict_Pop(container, key, &result);
    /* Now the reference counts are:
     * key: 1
     * dummy_value: 1
     * result: is NULL.
     */
    Py_DECREF(container);
    /* No change in the reference counts. */

For code and tests see:

* C, in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyDict_Pop_key_absent()``
* CPython, in ``src/cpy/RefCount/cRefCount.c``.
    ``test_PyDict_Pop_key_absent()``
* Python, pytest, in ``tests.unit.test_c_ref_count``:
    * ``test_PyDict_Pop_key_absent()``


.. index::
    single: Dictionary; PyDict_Pop(); Failure

Failure
^^^^^^^

.. todo::

    Finish Dictionary ``PyDict_Pop()`` Failure


.. index::
    single: Dictionary; Other APIs

Other APIs
----------

This section describes other dictionary APIs that are simple to describe and have no complications.

.. note::

    There are no tests for many of these APIs in the current version of this project.

.. todo::

    Finish Dictionary "Other APIs"


.. index::
    single: Dictionary; PyDict_GetItemWithError()

.. _chapter_containers_and_refcounts.dictionaries.pydict_getitemwitherror:

``PyDict_GetItemWithError()``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

`PyDict_GetItemWithError()`_ gets a new *borrowed* reference to a value from a dictionary or NULL.
Unlike `PyDict_GetItem()`_ this will set an exception if appropriate.

The C signature is:

.. code-block:: c

    PyObject *PyDict_GetItemWithError(PyObject *p, PyObject *key);

Currently, the only failure mode is if the first argument is not a dictionary.

For code and tests see:

* C, in ``src/cpy/Containers/DebugContainers.c``:
    * ``dbg_PyDict_GetItemWithError_fails()``

.. index::
    single: Dictionary; PyDict_DelItem()

``PyDict_DelItem()``
^^^^^^^^^^^^^^^^^^^^

`PyDict_DelItem()`_ removes a specific value if it exists. The reference count of the value will be decremented.
The key must be hashable; if it isn’t a ``TypeError`` is set.
If key is not in the dictionary a ``KeyError`` is set.
Returns 0 on success or -1 on failure in which case an exception will have been set.

The C function signature is:

.. code-block:: c

    int PyDict_DelItem(PyObject *p, PyObject *key);

.. todo::

    Complete ``PyDict_DelItem()`` with code examples.

.. index::
    single: Dictionary; PyDict_Items()

``PyDict_Items()``
^^^^^^^^^^^^^^^^^^^^

`PyDict_Items()`_ returns a *new* Python list containing *new* tuples of (key, value).
Each key and value will have their reference count incremented.
That is to say calling ``Py_DECREF`` on the result will change all the reference counts within the dictionary to their
previous values.

The C function signature is:

.. code-block:: c

    Pyobject *PyDict_Items(PyObject *p);

.. todo::

    Complete ``PyDict_Items()`` with code examples.

.. index::
    single: Dictionary; PyDict_Keys()

``PyDict_Keys()``
^^^^^^^^^^^^^^^^^^^^

`PyDict_Keys()`_ returns a *new* Python list containing all the keys.
Each key will have its reference count incremented.
That is to say calling ``Py_DECREF`` on the result will change all the reference counts within the dictionary to their
previous values.

The C function signature is:

.. code-block:: c

    Pyobject *PyDict_Keys(PyObject *p);

.. todo::

    Complete ``PyDict_Keys()`` with code examples.

.. index::
    single: Dictionary; PyDict_Values()

``PyDict_Values()``
^^^^^^^^^^^^^^^^^^^^

`PyDict_Values()`_ returns a *new* Python list containing all the values.
Each value will have its reference count incremented.
That is to say calling ``Py_DECREF`` on the result will change all the reference counts within the dictionary to their
previous values.

The C function signature is:

.. code-block:: c

    Pyobject *PyDict_Values(PyObject *p);

.. todo::

    Complete ``PyDict_Values()`` with code examples.

.. index::
    single: Dictionary; PyDict_Next()

``PyDict_Next()``
^^^^^^^^^^^^^^^^^^^^

`PyDict_Next()`_ is the standard way of iterating through all the keys and values.
Each key and value will is a *borrowed* reference.
The C function signature is:

.. code-block:: c

    int PyDict_Next(PyObject *p, Py_ssize_t *ppos, PyObject **pkey, PyObject **pvalue);

.. todo::

    Complete ``PyDict_Next()`` with code examples.

.. index::
    single: Dictionary; Py_BuildValue()

``Py_BuildValue()``
^^^^^^^^^^^^^^^^^^^^

`Py_BuildValue()`_ is a very convenient way to create dictionaries.
``Py_BuildValue("{OO}", key, value);`` will increment the refcount of the key and value and this can,
potentially, leak.

.. Links, mostly to the Python documentation:

.. Setters

.. _PySet_Add(): https://docs.python.org/3/c-api/set.html#c.PySet_Add
.. _PySet_Discard(): https://docs.python.org/3/c-api/set.html#c.PySet_Discard
.. _PySet_Pop(): https://docs.python.org/3/c-api/set.html#c.PySet_Pop

.. _chapter_containers_and_refcounts.sets:

.. index::
    single: Set

-----------------------
Sets
-----------------------

The set API is simple with no real difficulties for the user.

.. _chapter_containers_and_refcounts.sets.pyset_add:

.. index::
    single: PySet_Add()
    single: Set; PySet_Add()
    pair: Setters; Set

``PySet_Add()``
--------------------

`PySet_Add()`_ is fairly straightforward.
The set will increment the reference count of the value if it is *not* already in the set.

This returns 0 on success or -1 on failure in which case an exception will have been set which will be:

* A ``SystemError`` if set is not an instance of set or its subtype.
* A ``TypeError`` if the key is unhashable.
* A ``MemoryError`` if there is no room to grow.

Here is an example:

.. code-block:: c

    PyObject *container = PySet_New(NULL);
    /* Py_REFCNT(container) is 1 */
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* Py_REFCNT(value) is 1 */
    int ret_val = PySet_Add(container, value);
    assert(ret_val == 0);
    /* Py_REFCNT(value) is now 2 */

    /* Clean up. */
    Py_DECREF(container);
    /* Py_REFCNT(value) is now 1 */
    Py_DECREF(value);

When adding something that already exists in the set does *not* increment the reference count:

.. code-block:: c

    PyObject *container = PySet_New(NULL);
    /* Py_REFCNT(container) is 1 */
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* Py_REFCNT(value) is 1 */
    int ret_val = PySet_Add(container, value);
    assert(ret_val == 0);
    /* Py_REFCNT(value) is now 2 */

    /* Add duplicate. */
    ret_val = PySet_Add(container, value);
    assert(ret_val == 0);
    /* Py_REFCNT(value) is still 2 */

    /* Clean up. */
    Py_DECREF(container);
    /* Py_REFCNT(value) is now 1 */
    Py_DECREF(value);

.. _chapter_containers_and_refcounts.sets.pyset_discard:

.. index::
    single: PySet_Discard()
    single: Set; PySet_Discard()

``PySet_Discard()``
--------------------

`PySet_Discard()`_ is also fairly straightforward.
The set will discard (:ref:`chapter_containers_and_refcounts.discarded`) the value thus:

.. code-block:: c

    PyObject *container = PySet_New(NULL);
    /* Py_REFCNT(container) is 1 */
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* Py_REFCNT(value) is 1 */
    int ret_val = PySet_Add(container, value);
    assert(ret_val == 0);
    /* Py_REFCNT(value) is now 2 */

    /* Discard. */
    ret_val = PySet_Discard(container, value);
    assert(ret_val == 1);
    /* Py_REFCNT(value) is now 1 */

    /* Clean up. */
    Py_DECREF(container);
    /* Py_REFCNT(value) is still 1 */
    Py_DECREF(value);

.. _chapter_containers_and_refcounts.sets.pyset_pop:

.. index::
    single: PySet_Pop()
    single: Set; PySet_Pop()

``PySet_Pop()``
--------------------

`PySet_Pop()`_ will return a *new* reference to the existing object and it is up to caller to decrement the
reference count appropriately.

For example, the reference counts work as follows:

.. code-block:: c

    PyObject *container = PySet_New(NULL);
    /* Py_REFCNT(container) is 1 */
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* Py_REFCNT(value) is 1 */
    int ret_val = PySet_Add(container, value);
    assert(ret_val == 0);
    /* Py_REFCNT(value) is now 2 */

    /* Pop. */
    PyObject *popped_value = PySet_Pop(container);
    assert(popped_value == value);
    /* Py_REFCNT(value) is still 2 */

    /* Clean up. */
    Py_DECREF(container);
    /* Py_REFCNT(value) is still 2 so need to double Py_DECREF calls. */
    Py_DECREF(value);
    Py_DECREF(value);

So this is how `PySet_Pop()`_  might be used in practice:

.. code-block:: c


    void add_to_set(PyObject *container) {
        PyObject *value = new_unique_string(__FUNCTION__, NULL);
        /* Py_REFCNT(value) is 1 */
        int ret_val = PySet_Add(container, value);
        assert(ret_val == 0);
        /* Py_REFCNT(value) is now 2 */
        /* Decrement our local value */
        Py_DECREF(value);
        /* Now the container has the only reference to the value. */
    }

    void pop_from_set(PyObject *container) {
        PyObject *value = PySet_Pop(container);
        /* Do something with the value... */

        /* Then as we 'own' value we need to free it. */
        Py_DECREF(value);
    }

    PyObject *container = PySet_New(NULL);
    add_to_set(container);
    pop_from_set(container);
    /* Clean up. */
    Py_DECREF(container);

--------------
Summary
--------------

This summarises where the Python documentation is wrong, absent or misleading and where you might get into trouble.

.. index::
    single: Tuple; Summary
    pair: Tuple; Gotchas
    single: Tuple; PyTuple_SetItem()
    single: Tuple; PyTuple_SET_ITEM()
    single: Tuple; PyTuple_Pack()
    single: Tuple; Py_BuildValue()
    single: Tuple; PyTuple_GET_ITEM()

Tuples
--------------

- `PyTuple_SetItem()`_ will lead to a SIGSEGV if the existing value is the same as the inserted value.
  See :ref:`chapter_containers_and_refcounts.tuples.PyTuple_SetItem`.
- On failure `PyTuple_SetItem()`_ will decrement the reference count of the given value and this might well lead to a
  SIGSEGV.
  See :ref:`chapter_containers_and_refcounts.tuples.PyTuple_SetItem.failures`.
- `PyTuple_SET_ITEM()`_ will lead to a memory leak when replacing an existing value as it abandons the previous
  reference.
  See :ref:`chapter_containers_and_refcounts.tuples.PyTuple_SET_ITEM`.
- `PyTuple_SET_ITEM()`_ does no error checking so is capable of writing to arbitrary memory locations on error.
  See :ref:`chapter_containers_and_refcounts.tuples.PyTuple_SET_ITEM.failures`.
- Both `PyTuple_SetItem()`_ and `PyTuple_SET_ITEM()`_ accept ``NULL`` as the value argument and behave as documented.
- `PyTuple_Pack()`_ will leak the given arguments. See :ref:`chapter_containers_and_refcounts.tuples.PyTuple_Pack`.
- `Py_BuildValue()`_ will leak the given arguments when used with the ``"O"`` argument.
  See :ref:`chapter_containers_and_refcounts.tuples.Py_BuildValue`.
- `PyTuple_GET_ITEM()`_ can try to access arbitrary memory locations if the arguments are wrong.
  See :ref:`chapter_containers_and_refcounts.tuples.Getters`.


.. index::
    single: List; Summary
    pair: List; Gotchas
    single: List; PyList_SetItem()
    single: List; PyList_SET_ITEM()
    single: List; PyList_Append()
    single: List; PyList_Insert()
    single: List; Py_BuildValue()
    single: List; PyList_GET_ITEM()

Lists
--------------

- `PyList_SetItem()`_ will lead to a SIGSEGV if the existing value is the same as the inserted value.
  See :ref:`chapter_containers_and_refcounts.lists.PyList_SetItem`.
- On failure `PyList_SetItem()`_ will decrement the reference count of the given value and this might well lead to a
  SIGSEGV.
  See :ref:`chapter_containers_and_refcounts.lists.PyList_SET_ITEM.failures`.
- `PyList_SET_ITEM()`_ will lead to a memory leak when replacing an existing value as it abandons the previous
  reference.
  See :ref:`chapter_containers_and_refcounts.lists.PyList_SET_ITEM`.
- `PyList_SET_ITEM()`_ does no error checking so is capable of writing to arbitrary memory locations on error.
  See :ref:`chapter_containers_and_refcounts.lists.PyList_SET_ITEM.failures`.
- Both `PyList_SetItem()`_ and `PyList_SET_ITEM()`_ accept ``NULL`` as the value argument and behave as documented.
- `PyList_Append()`_ will increment the reference count of the given argument and so may leak.
  `PyList_Append()`_ uses `PyList_SET_ITEM()`_ in its implementation so those appropriate warnings apply.
  See :ref:`chapter_containers_and_refcounts.lists.PyList_Append`
  and :ref:`chapter_containers_and_refcounts.lists.PyList_SET_ITEM`.
- `PyList_Insert()`_ is poorly documented in the official Python documentation.
  See :ref:`chapter_containers_and_refcounts.lists.PyList_Insert`.
- `Py_BuildValue()`_ will leak the given arguments when used with the ``"O"`` argument.
  Simlar to tuples, describe here: :ref:`chapter_containers_and_refcounts.tuples.Py_BuildValue`.
- `PyList_GET_ITEM()`_ can try to access arbitrary memory locations if the arguments are wrong.
  See :ref:`chapter_containers_and_refcounts.lists.Getters`.


.. index::
    single: Dictionary; Summary
    pair: Dictionary; Gotchas
    single: Dictionary; PyDict_SetItem()
    single: Dictionary; PyDict_SetDefault()

Dictionaries
--------------

- `PyDict_SetItem()`_ generally increments the reference count of the given key/value however this API
  has some complicated rules about key/value reference counts that are not described in the Python documentation.
  See :ref:`chapter_containers_and_refcounts.dictionaries.setitem`.
- With `PyDict_SetItem()`_ if either the key or value is ``NULL`` there will be a SIGSEGV.
  See :ref:`chapter_containers_and_refcounts.dictionaries.setitem`.
- Otherwise `PyDict_SetItem()`_ has generally graceful behaviour on failure.
  See :ref:`chapter_containers_and_refcounts.dictionaries.setitem.failure`.
- `PyDict_SetDefault()`_ has generally graceful behaviour on failure.
  See :ref:`chapter_containers_and_refcounts.dictionaries.setdefault`.
- `PyDict_GetItemWithError()`_ is incorrectly implemented or documented.
  See :ref:`chapter_containers_and_refcounts.dictionaries.pydict_getitemwitherror`.

.. todo::

    Complete the summary of dictionary APIs and their documentation lacunae.

Sets
--------------

The set API is much cleaner than the others and contains few gotchas.
In summary:

- `PySet_Add()`_ will increment the reference count of the value if it is not already in the set.
  See :ref:`chapter_containers_and_refcounts.sets.pyset_add`.
- `PySet_Discard()`_ will decrement the reference count of the value and returns it.
  It is up to the caller to decrement the reference count of the returned value.
  See :ref:`chapter_containers_and_refcounts.sets.pyset_discard`.
- `PySet_Pop()`_ does not decrement the reference count of the returned value, it is
  *abandoned* (see :ref:`chapter_containers_and_refcounts.abandoned`).
  It is up to the caller to decrement the reference count of the returned value.
  See :ref:`chapter_containers_and_refcounts.sets.pyset_pop`.


.. Example footnote [#]_.

.. rubric:: Footnotes

.. [#] The official `Python documentation <https://docs.python.org/3/c-api/concrete.html>`_ categorises tuples and lists
    as *sequence objects* because they support the `Sequence Protocol <https://docs.python.org/3/c-api/sequence.html>`_
    and dictionaries and sets as *container objects* because they support the
    `Mapping Protocol <https://docs.python.org/3/c-api/mapping.html>`_.
    In this chapter I use looser language by describing all four as *containers*.

.. [#] ``Py_BuildValue`` can not create a set, only a tuple, list or dictionary.
