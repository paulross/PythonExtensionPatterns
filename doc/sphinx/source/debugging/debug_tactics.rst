.. highlight:: python
    :linenothreshold: 10

.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3


=================================
Debugging Tactics
=================================

So what is the problem that you are trying to solve?


----------------------------
Access After Free
----------------------------

^^^^^^^^^^^^^^^^^^^
Problem
^^^^^^^^^^^^^^^^^^^

You suspect that you are accessing a Python object after it has been free'd. Code such as this:

.. code-block:: c

    static PyObject *access_after_free(PyObject *pModule) {
        PyObject *pA = PyLong_FromLong(1024L);
        Py_DECREF(pA);
        PyObject_Print(pA, stdout, 0);
        Py_RETURN_NONE;
    }

``pA`` has been freed before ``PyObject_Print`` is called.

^^^^^^^^^^^^^^^^^^^
Solution
^^^^^^^^^^^^^^^^^^^

.. code-block:: c

    Python 3.4.3 (default, Sep 16 2015, 16:56:10) 
    [GCC 4.2.1 Compatible Apple LLVM 6.0 (clang-600.0.51)] on darwin
    Type "help", "copyright", "credits" or "license" for more information.
    >>> import cPyRefs
    >>> cPyRefs.afterFree()
    <refcnt -2604246222170760229 at 0x10a474130>
    >>> 

----------------------------------
``Py_INCREF`` called too often
----------------------------------

**Summary:** If ``Py_INCREF`` is called once or more too often then memory will be  held onto despite those objects not being visible in ``globals()`` or ``locals()``.

**Symptoms:** You run a test that creates objects and uses them. As the test exits all the objects should go out of scope and be de-alloc'd however you observe that memory is being permanently lost.

^^^^^^^^^^^^^^^^^^^
Problem
^^^^^^^^^^^^^^^^^^^

We can create a simulation of this by creating two classes, with one we will create a leak caused by an excessive ``Py_INCREF`` (we do this by calling ``cPyRefs.incref()``). The other class instance will not be leaked.

Here is the code for incrementing the reference count in the ``cPyRefs`` module:

.. code-block:: c

    /* Just increfs a PyObject. */
    static PyObject *incref(PyObject *pModule, PyObject *pObj) {
        fprintf(stdout, "incref(): Ref count was: %zd\n", pObj->ob_refcnt);
        Py_INCREF(pObj);
        fprintf(stdout, "incref(): Ref count now: %zd\n", pObj->ob_refcnt);
        Py_RETURN_NONE;
    }

And the Python interpreter session we create two instances and excessively incref one of them:

.. code-block:: python

    >>> import cPyRefs # So we can create a leak
    >>> class Foo : pass # Foo objects will be leaked
    ...
    >>> class Bar : pass # Bar objects will not be leaked
    ...
    >>> def test_foo_bar():
    ...     f = Foo()
    ...     b = Bar()
    ...     # Now increment the reference count of f, but not b
    ...     # This simulates what might happen in a leaky extension
    ...     cPyRefs.incref(f)
    ... 
    >>> # Call the test, the output comes from cPyRefs.incref()
    >>> test_foo_bar()
    incref(): Ref count was: 2
    incref(): Ref count now: 3


^^^^^^^^^^^^^^^^^^^
Solution
^^^^^^^^^^^^^^^^^^^

Use a debug version of Python with ``COUNT_ALLOCS`` defined. This creates ``sys.getcounts()`` which lists, for each type, how many allocs and de-allocs have been made. Notice the difference between ``Foo`` and ``Bar``:

.. code-block:: python

    >>> import sys
    >>> sys.getcounts()
    [
        ('Bar', 1, 1, 1),
        ('Foo', 1, 0, 1),
        ...
    ]

This should focus your attention on the leaky type ``Foo``.

You can find the count of all live objects by doing this:

.. code-block:: python

    >>> still_live = [(v[0], v[1] - v[2]) for v in sys.getcounts() if v[1] > v[2]]
    >>> still_live
    [('Foo', 1), ... ('str', 7783), ('dict', 714), ('tuple', 3875)]

