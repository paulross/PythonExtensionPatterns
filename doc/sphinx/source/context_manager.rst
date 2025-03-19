.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _context manger: https://docs.python.org/3/glossary.html#term-context-manager>
.. _\__enter__(): https://docs.python.org/3/library/stdtypes.html#contextmanager.__enter__
.. _\__exit__(): https://docs.python.org/3/library/stdtypes.html#contextmanager.__exit__

.. _chapter_context_manager:

.. index::
    single: Context Managers

***************************
Context Managers
***************************

This chapter describes how to write
for your C objects.

.. index::
    single: Context Managers; C Functions

===========================
C Functions
===========================

This is a summary of what is required for the C functions implementing a `context manger`_.
The is no specific ``tp_...`` slot for the context manager functions ``__enter__`` and ``__exit__``, instead they are added
to the object as named, looked up, Python methods.

.. index::
    single: Context Managers; __enter__

--------------------------------------
``__enter__``
--------------------------------------

The C function must, at least, increment the reference count of ``self`` and
return ``self``:

.. code-block:: c

    static PyObject *
    ContextManager_enter(ContextManager *self, PyObject *Py_UNUSED(args)) {
        /* Stuff here. */
        Py_INCREF(self);
        return (PyObject *)self;
    }

.. index::
    single: Context Managers; __exit__

--------------------------------------
``__exit__``
--------------------------------------

The `__exit__()`_ function is declared thus.
It takes three arguments so ``METH_VARARGS`` is used.
The three arguments are each ``None`` if no exception has been raised within
the ``with`` block.
If an exception *has* been raised within the ``with`` block then the
three arguments are the exception type, value and the traceback object.

The return value of the ``__exit__`` method tells the interpreter whether
any exception should be suppressed.
If the function returns ``False`` then the exception should be
propagated.
This is the common case.
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

--------------------------------------
Method declarations
--------------------------------------

Note that `__enter__()`_ is declared with ``METH_NOARGS`` and `__exit__()`_  is declared with ``METH_VARARGS``:

.. code-block:: c

    static PyMethodDef ContextManager_methods[] = {
        /* ... */
        {"__enter__", (PyCFunction) ContextManager_enter, METH_NOARGS,
                        PyDoc_STR("__enter__() -> ContextManager")},
        {"__exit__", (PyCFunction) ContextManager_exit, METH_VARARGS,
                        PyDoc_STR("__exit__(exc_type, exc_value, exc_tb) -> bool")},
        /* ... */
        {NULL, NULL, 0, NULL} /* sentinel */
    };

=================================
Understanding the Context Manager
=================================

What is worth understanding is the way that reference counts are incremented and
decremented and the interplay between your C code and the CPython interpreter.

.. index::
    single: Context Managers; Without target

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

#. Creating the ``cCtxMgr.ContextManager()`` calls ``ContextManager_new`` which makes the
   reference count 1.
#. The ``with`` statement causes the CPython interpreter to increment the reference count
   (to 2) and then call ``__enter__`` that is implemented in our C function
   ``ContextManager_enter``.
#. Our ``ContextManager_enter`` function increments the reference count, so it is now 3.
#. As the context manager ends the ``with`` statement the CPython interpreter
   decrements the reference count *twice* to the value 1.
   The logic is:

    #. Decrement the reference count once as we are exiting the ``with`` statement. The reference count is now 2.
    #. Did the ``with`` statement have a target? If not, as in this case, then decrement the reference count once more. The reference count is now 1.

#. after the ``pass`` statement the CPython interpreter then calls ``__exit__`` which is implemented in our function
   ``ContextManager_exit``.
   This does not change the reference count which remains at 1.
#. As the context manager goes out of scope the CPython interpreter decrements the reference
   count to 0 and then calls our C function ``ContextManager_dealloc`` with a reference count
   of 0 and that frees the object.


.. index::
    single: Context Managers; With target

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

#. As above, the reference count becomes 1.
#. As above, the reference count becomes 2.
#. As above, the reference count becomes 3.
#. As the context manager ends the ``with`` statement the CPython interpreter
   decrements the reference count just *once* to the value 2 as there *is* a target, called ``context`` in this case.
#. After the ``pass`` statement the CPython interpreter then calls ``__exit__`` which is implemented in our function
   ``ContextManager_exit``.
   This does not change the reference count which remains at 2.
#. As the context manager goes out of scope the CPython interpreter decrements the reference
   count to 1.
   This ensures the survival of ``context`` after the ``with`` block.
#. When ``context`` goes out of scope, say on a function return or a ``del`` statement the
   CPython interpreter decrements the reference count to 0 and then calls our C function
   ``ContextManager_dealloc`` which frees the object.

.. index::
    single: Context Managers; Minimal in C

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

-----------------------------------------
``__enter__`` and ``__exit__`` Methods
-----------------------------------------

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


-----------------------------------------
Type Declaration
-----------------------------------------

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

-----------------------------------------
Module
-----------------------------------------

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

-----------------------------------------
Setup
-----------------------------------------

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

-----------------------------------------
Testing
-----------------------------------------

The actual code in ``src/cpy/CtxMgr/cCtxMgr.c`` contains extra trace reporting that confirms the reference counts and
(no) memory leakage.

This can be run with:

.. code-block:: bash

    $ pytest tests/unit/test_c_ctxmgr.py -vs

This test:

.. code-block:: python

    def test_very_simple():
        print()
        with cCtxMgr.ContextManager():
            pass

Gives this output:

.. code-block:: text

    tests/unit/test_c_ctxmgr.py::test_very_simple
          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
         ContextManager_exit STRT REFCNT = 1
         ContextManager_exit DONE REFCNT = 1
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4546088432

This test:

.. code-block:: python

    def test_simple():
        print()
        with cCtxMgr.ContextManager() as context:
            assert sys.getrefcount(context) == 3
            assert context.len_buffer_lifetime() == cCtxMgr.BUFFER_LENGTH
            assert context.len_buffer_context() == cCtxMgr.BUFFER_LENGTH
        assert sys.getrefcount(context) == 2
        assert context.len_buffer_lifetime() == cCtxMgr.BUFFER_LENGTH
        assert context.len_buffer_context() == 0
        del context

Gives this output:

.. code-block:: text

    tests/unit/test_c_ctxmgr.py::test_simple
          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
         ContextManager_exit STRT REFCNT = 2
         ContextManager_exit DONE REFCNT = 2
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4546096048


This test:

.. code-block:: python

    def test_memory():
        proc = psutil.Process()
        print()
        print(f'RSS START: {proc.memory_info().rss:12,d}')
        for i in range(8):
            print(f'RSS START {i:5d}: {proc.memory_info().rss:12,d}')
            with cCtxMgr.ContextManager() as context:
                print(f'RSS START CTX: {proc.memory_info().rss:12,d}')
                # Does not work in the debugger due to introspection.
                # assert sys.getrefcount(context) == 3
                assert context.len_buffer_lifetime() == cCtxMgr.BUFFER_LENGTH
                assert context.len_buffer_context() == cCtxMgr.BUFFER_LENGTH
                print(f'RSS   END CTX: {proc.memory_info().rss:12,d}')
            # Does not work in the debugger due to introspection.
            # assert sys.getrefcount(context) == 2
            assert context.len_buffer_lifetime() == cCtxMgr.BUFFER_LENGTH
            assert context.len_buffer_context() == 0
            del context
            print(f'RSS   END {i:5d}: {proc.memory_info().rss:12,d}')
        print(f'RSS  END: {proc.memory_info().rss:12,d}')

Gives this output:

.. code-block:: text

    tests/unit/test_c_ctxmgr.py::test_memory
    RSS START:  300,032,000
    RSS START     0:  300,048,384
          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
    RSS START CTX:  300,048,384
    RSS   END CTX:  300,048,384
         ContextManager_exit STRT REFCNT = 2
         ContextManager_exit DONE REFCNT = 2
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4546096048
    RSS   END     0:  300,048,384
    RSS START     1:  300,048,384
          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
    RSS START CTX:  300,048,384
    RSS   END CTX:  300,048,384
         ContextManager_exit STRT REFCNT = 2
         ContextManager_exit DONE REFCNT = 2
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4546096048
    RSS   END     1:  300,048,384
    RSS START     2:  300,048,384
          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
    RSS START CTX:  300,048,384
    RSS   END CTX:  300,048,384
         ContextManager_exit STRT REFCNT = 2
         ContextManager_exit DONE REFCNT = 2
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4546096048
    RSS   END     2:  300,048,384
    RSS START     3:  300,048,384
          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
    RSS START CTX:  300,048,384
    RSS   END CTX:  300,048,384
         ContextManager_exit STRT REFCNT = 2
         ContextManager_exit DONE REFCNT = 2
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4546096048
    RSS   END     3:  300,048,384
    RSS START     4:  300,048,384
          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
    RSS START CTX:  300,048,384
    RSS   END CTX:  300,048,384
         ContextManager_exit STRT REFCNT = 2
         ContextManager_exit DONE REFCNT = 2
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4546096048
    RSS   END     4:  300,048,384
    RSS START     5:  300,048,384
          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
    RSS START CTX:  300,048,384
    RSS   END CTX:  300,048,384
         ContextManager_exit STRT REFCNT = 2
         ContextManager_exit DONE REFCNT = 2
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4546096048
    RSS   END     5:  300,048,384
    RSS START     6:  300,048,384
          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
    RSS START CTX:  300,048,384
    RSS   END CTX:  300,048,384
         ContextManager_exit STRT REFCNT = 2
         ContextManager_exit DONE REFCNT = 2
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4546096048
    RSS   END     6:  300,048,384
    RSS START     7:  300,048,384
          ContextManager_new DONE REFCNT = 1
        ContextManager_enter STRT REFCNT = 2
        ContextManager_enter DONE REFCNT = 3
    RSS START CTX:  300,048,384
    RSS   END CTX:  300,048,384
         ContextManager_exit STRT REFCNT = 2
         ContextManager_exit DONE REFCNT = 2
      ContextManager_dealloc STRT REFCNT = 0
      ContextManager_dealloc DONE REFCNT = 4546096048
    RSS   END     7:  300,048,384
    RSS  END:  300,048,384
