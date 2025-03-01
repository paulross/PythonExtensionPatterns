.. highlight:: python
    :linenothreshold: 10

.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. index::
    single: Debugging; Tactics

=================================
Debugging Tactics
=================================

So what is the problem that you are trying to solve?


.. index::
    single: Access After Free
    single: Debugging; Access After Free

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
    >>> from cPyExtPatt import cPyRefs
    >>> cPyRefs.afterFree()
    <refcnt -2604246222170760229 at 0x10a474130>
    >>> 
