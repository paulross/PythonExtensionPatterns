.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _chapter_context_manager:

***************************
Context Managers
***************************

This chapter describes how to write
`context mangers <https://docs.python.org/3/glossary.html#term-context-manager>`_
for your C objects.

===========================
C Functions
===========================

This is a summary of what is required for the C functions implementing a context
manager.

--------------------------------------
``__enter__``
--------------------------------------

The is no specific ``tp_...`` slot for context managers, instead they are added
to the object as a normal Python method.
Note that ``__enter__`` is declared with ``METH_NOARGS``:

.. code-block:: c

    static PyMethodDef ContextManager_methods[] = {
        {"__enter__", (PyCFunction) ContextManager_enter, METH_NOARGS,
                        PyDoc_STR("__enter__() -> ContextManager")},
        /* ... */
        {NULL, NULL, 0, NULL} /* sentinel */
    };

The C function must, at least, increment the reference count of ``self`` and
return ``self``:

.. code-block:: c

    static PyObject *
    ContextManager_enter(ContextManager *self, PyObject *Py_UNUSED(args)) {
        /* Stuff here. */
        Py_INCREF(self);
        return (PyObject *)self;
    }

--------------------------------------
``__exit__``
--------------------------------------

The ``__exit__`` function is declared thus.
It takes three arguments thus ``METH_VARARGS`` is used.

.. code-block:: c

    static PyMethodDef ContextManager_methods[] = {
        {"__exit__", (PyCFunction) ContextManager_exit, METH_VARARGS,
                        PyDoc_STR("__exit__(exc_type, exc_value, exc_tb) -> bool")},
        /* ... */
        {NULL, NULL, 0, NULL} /* sentinel */
    };

The three arguments are ``None`` if no exception has been raised within
the ``with`` block.
If an exception *has* been raised within the ``with`` block then the
three arguments are the exception type, value and the traceback object.

The return value of the ``__exit__`` method tells the interpreter whether
any exception should be suppressed.
If the function returns ``False`` then the exception should be
propagated.
This is usually the common case.
If the function returns ``True`` then the exception should be
suppressed and execution continues with the statement immediately
after the ``with`` statement.

The exit method is defined in C thus.
Note that there is no change to
the reference count, that is all done appropriately by the
CPython interpreter:

.. code-block:: c

    static PyObject *
    ContextManager_exit(ContextManager *self, PyObject *args) {
        /* Stuff. */
        Py_RETURN_FALSE;
    }

=================================
Understanding the Context Manager
=================================

What is worth understanding is the way that reference counts are incremented and
decremented and the interplay between your C code and the CPython interpreter.

----------------------------------
A Context Manager Without a Target
----------------------------------

Take this simple code:

.. code-block:: python

    from cPyExtPatt import cCtxMgr

    with cCtxMgr.ContextManager():
        pass

..
    def test_very_simple():

    Gives:

          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
         ContextManager_exit STRT REFCNT = 1
         ContextManager_exit DONE REFCNT = 1
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4413491472

The sequence of reference count changes are as follows:

1. Creating the ``cCtxMgr.ContextManager()`` calls ``ContextManager_new`` which makes the
   reference count 1.
2. The ``with`` statement causes the CPython interpreter to increment the reference count
   (to 2) and then call ``__enter__`` that is implemented in our C function
   ``ContextManager_enter``.
3. Our ``ContextManager_enter`` function increments the reference count, so it is now 3.
4. As the context manager exists the scope of the ``with`` statement the CPython interpreter
   decrements the reference count *twice* to the value 1.
   See below for a discussion of this.
5. The CPython interpreter then calls ``__exit__`` which is implemented in our function
   ``ContextManager_exit``.
   This does not change the reference count which remains at 1.
6. As the context manager goes out of scope the CPython interpreter decrements the reference
   count to 0 and then calls our C function ``ContextManager_dealloc`` with a reference count
   of 0 and that frees the object.

The CPython interpreter behaviour at step 4 is interesting, essentially it is this:

- Decrement the reference count once as we are exiting the ``with`` statement.
- Did the ``with`` statement have a target?
  If not, as in this case, then decrement the reference count once more.

----------------------------------
A Context Manager With a Target
----------------------------------

The importance of the ``with`` statement having a target is because a context manager
can be used like this:

.. code-block:: python

    from cPyExtPatt import cCtxMgr

    with cCtxMgr.ContextManager() as context:
        pass
    # context survives here with a reference count of 1.
    # This will be decremented when context goes out of scope.
    # For example on a function return.

..
    def test_simple():

    Gives:

          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
         ContextManager_exit STRT REFCNT = 2
         ContextManager_exit DONE REFCNT = 2
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4413491440

In this case the ``context`` survives the ``with`` statement and is available to any following code.
So step 4 above would only decrement the reference count once leaving it with a reference count of 2.
The ``context`` variable reference count will be finally decremented when it goes out of scope, for
example on a function return.

The sequence of reference count changes are now as follows:

1. As above, the reference count becomes 1.
2. As above, the reference count becomes 2.
3. As above, the reference count becomes 3.
4. As the context manager exists the scope of the ``with`` statement the CPython interpreter
   decrements the reference count *once* to the value 2.
5. The CPython interpreter then calls ``__exit__`` which is implemented in our function
   ``ContextManager_exit``.
   This does not change the reference count which remains at 2.
6. As the context manager goes out of scope the CPython interpreter decrements the reference
   count to 1.
   This ensures the survival of ``context`` after the ``with`` block.
7. When ``context`` goes out of scope, say on a function return or a ``del`` statement the
   CPython interpreter decrements the reference count to 0 and then calls our C function
   ``ContextManager_dealloc`` which frees the object.

===============================
Minimal Context Manager in C
===============================

Here is the minimal, complete, C code that implements context manager.
There is example code in ``src/cpy/CtxMgr/cCtxMgr.c`` and tests in
``tests/unit/test_c_ctxmgr.py``

First the object declaration, allocation and de-allocation functions:

.. code-block:: c

    #define PY_SSIZE_T_CLEAN

    #include "Python.h"

    typedef struct {
        PyObject_HEAD
    } ContextManager;

    static ContextManager *
    ContextManager_new(PyObject *Py_UNUSED(arg)) {
        return PyObject_New(ContextManager, &ContextManager_Type);;
    }

    static void
    ContextManager_dealloc(ContextManager *self) {
        PyObject_Del(self);
    }

The ``__enter__`` and ``__exit__`` methods:

.. code-block:: c

    static PyObject *
    ContextManager_enter(ContextManager *self, PyObject *Py_UNUSED(args)) {
        Py_INCREF(self);
        return (PyObject *)self;
    }

    static PyObject *
    ContextManager_exit(ContextManager *Py_UNUSED(self), PyObject *Py_UNUSED(args)) {
        Py_RETURN_FALSE;
    }

    static PyMethodDef ContextManager_methods[] = {
            {"__enter__", (PyCFunction) ContextManager_enter, METH_VARARGS,
                            PyDoc_STR("__enter__() -> ContextManager")},
            {"__exit__", (PyCFunction) ContextManager_exit, METH_VARARGS,
                            PyDoc_STR("__exit__() -> bool")},
            {NULL, NULL, 0, NULL} /* sentinel */
    };

The type declaration:

.. code-block:: c

    static PyTypeObject ContextManager_Type = {
            PyVarObject_HEAD_INIT(NULL, 0)
            .tp_name = "cObject.ContextManager",
            .tp_basicsize = sizeof(ContextManager),
            .tp_dealloc = (destructor) ContextManager_dealloc,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_methods = ContextManager_methods,
            .tp_new = (newfunc) ContextManager_new,
    };


Finally the module:

.. code-block:: c

    PyDoc_STRVAR(module_doc, "Example of a context manager.");

    static struct PyModuleDef cCtxMgr = {
            PyModuleDef_HEAD_INIT,
            .m_name = "cCtxMgr",
            .m_doc = module_doc,
            .m_size = -1,
    };

    PyMODINIT_FUNC
    PyInit_cCtxMgr(void) {
        PyObject *m = NULL;
        m = PyModule_Create(&cCtxMgr);
        if (m == NULL) {
            goto fail;
        }
        if (PyType_Ready(&ContextManager_Type) < 0) {
            goto fail;
        }
        if (PyModule_AddObject(m, "ContextManager", (PyObject *) &ContextManager_Type)) {
            goto fail;
        }
        return m;
    fail:
        Py_XDECREF(m);
        return NULL;
    }

.. note::

    The actual code in ``src/cpy/CtxMgr/cCtxMgr.c`` contains extra trace
    reporting that confirms the reference counts and (no) memory leakage.


This code is added to the ``setup.py`` file:

.. code-block:: python

    Extension(f"{PACKAGE_NAME}.cCtxMgr", sources=['src/cpy/CtxMgr/cCtxMgr.c', ],
              include_dirs=['/usr/local/include', ],
              library_dirs=[os.getcwd(), ],
              extra_compile_args=extra_compile_args_c,
              language='c',
              ),

And can be used thus:

.. code-block:: python

    from cPyExtPatt import cCtxMgr

    with cCtxMgr.ContextManager():
        pass
