.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

=================================
Exception Raising 
=================================

A brief interlude on how to communicate error conditions from C code to Python.

These CPython calls are the most useful:

* ``PyErr_SetString(...)`` - To set an exception type with a fixed string.
* ``PyErr_Format(...)`` - To set an exception type with a formatted string.
* ``PyErr_Occurred()`` - To check if an exception has already been set in the flow of control.
* ``PyErr_Clear()`` - Clearing any set exceptions, have good reason to do this!

Indicating an error condition is a two stage process; your code must register an exception and then indicate failure by returning ``NULL``. Here is a C function doing just that:

.. code-block:: c

    static PyObject *_raise_error(PyObject *module) {
    
        PyErr_SetString(PyExc_ValueError, "Ooops.");
        return NULL;
    }

You might want some dynamic information in the exception object, in that case ``PyErr_Format`` will do:

.. code-block:: c

    static PyObject *_raise_error_formatted(PyObject *module) {
    
        PyErr_Format(PyExc_ValueError,
                     "Can not read %d bytes when offset %d in byte length %d.", \
                     12, 25, 32
                     );
        return NULL;
    }

If one of the two actions is missing then the exception will not be raised correctly. For example returning ``NULL`` without setting an exception type:

.. code-block:: c

    /* Illustrate returning NULL but not setting an exception. */
    static PyObject *_raise_error_bad(PyObject *module) {
        return NULL;
    }

Executing this from Python will produce a clear error message (the C function ``_raise_error_bad()`` is mapped to the Python function ``cExcep.raiseErrBad()`` ::

    >>> cExcep.raiseErrBad()
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    SystemError: error return without exception set

If the opposite error is made, that is setting an exception but not signalling then the function will succeed but leave a later runtime error:

.. code-block:: c

    static PyObject *_raise_error_mixup(PyObject *module) {
        PyErr_SetString(PyExc_ValueError, "ERROR: _raise_error_mixup()");
        Py_RETURN_NONE;
    }

The confusion can arise is that if a subsequent function then tests to see if an exception is set, if so signal it. It will appear that the error is coming from the second function when actually it is from the first:

.. code-block:: c

    static PyObject *_raise_error_mixup_test(PyObject *module) {
        if (PyErr_Occurred()) {
            return NULL;
        }
        Py_RETURN_NONE;
    }

The other thing to note is that if there are multiple calls to ``PyErr_SetString`` only the last one counts:

.. code-block:: c

    static PyObject *_raise_error_overwrite(PyObject *module) {
        PyErr_SetString(PyExc_RuntimeError, "FORGOTTEN.");
        PyErr_SetString(PyExc_ValueError, "ERROR: _raise_error_overwrite()");
        assert(PyErr_Occurred());
        return NULL;
    }

---------------------------------
Common Exception Patterns
---------------------------------

Here are some common use cases for raising exceptions.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Type Checking
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A common requirement is to check the types of the arguments and raise a ``TypeError`` if they are wrong. Here is an example where we require a ``bytes`` object:

.. code-block:: c
    :linenos:
    :emphasize-lines: 4-9

    static PyObject*
    function(PyObject *self, PyObject *arg) {
        /* ... */
        if (! PyBytes_Check(arg)) {
            PyErr_Format(PyExc_TypeError,
                         "Argument \"value\" to %s must be a bytes object not a \"%s\"",
                         __FUNCTION__, Py_TYPE(arg)->tp_name);
            goto except;
        }
        /* ... */
    }

Thats fine if you have a macro such as ``PyBytes_Check`` and for your own types you can create a couple of suitable macros:

.. code-block:: c

    #define PyMyType_CheckExact(op) (Py_TYPE(op) == &PyMyType_Type)
    #define PyMyType_Check(op) PyObject_TypeCheck(op, &PyMyType_Type)

Incidentially ``PyObject_TypeCheck`` is defined as:

.. code-block:: c

    #define PyObject_TypeCheck(ob, tp) \
        (Py_TYPE(ob) == (tp) || PyType_IsSubtype(Py_TYPE(ob), (tp)))

---------------------------------
Creating Specialised Excpetions
---------------------------------

Often you need to create an Exception class that is specialised to a particular module. This can be done quite easily using either the ``PyErr_NewException`` or the ``PyErr_NewExceptionWithDoc`` functions. These create new exception classes that can be added to a module. For example:

.. code-block:: c

    /* Exception types as static to be initialised during module initialisation. */
    static PyObject *ExceptionBase;
    static PyObject *SpecialisedError;

    /* Standard module initialisation: */
    static PyModuleDef noddymodule = {
        PyModuleDef_HEAD_INIT,
        "noddy",
        "Example module that creates an extension type.",
        -1,
        NULL, NULL, NULL, NULL, NULL
    };

    PyMODINIT_FUNC
    PyInit_noddy(void) 
    {
        PyObject* m;

        noddy_NoddyType.tp_new = PyType_GenericNew;
        if (PyType_Ready(&noddy_NoddyType) < 0)
            return NULL;

        m = PyModule_Create(&noddymodule);
        if (m == NULL)
            return NULL;

        Py_INCREF(&noddy_NoddyType);
        PyModule_AddObject(m, "Noddy", (PyObject *)&noddy_NoddyType);
        
        /* Initialise exceptions here.
         *
         * Firstly a base class exception that inherits from the builtin Exception.
         * This is acheieved by passing NULL as the PyObject* as the third argument.
         *
         * PyErr_NewExceptionWithDoc returns a new reference.
         */
        ExceptionBase = PyErr_NewExceptionWithDoc(
            "noddy.ExceptionBase", /* char *name */
            "Base exception class for the noddy module.", /* char *doc */
            NULL, /* PyObject *base */
            NULL /* PyObject *dict */);
        /* Error checking: this is oversimplified as it should decref
         * anything created above such as m.
         */
        if (! ExceptionBase) {
            return NULL;
        } else {
            PyModule_AddObject(m, "ExceptionBase", ExceptionBase);
        }
        /* Now a sub-class exception that inherits from the base exception above.
         * This is acheieved by passing non-NULL as the PyObject* as the third argument.
         *
         * PyErr_NewExceptionWithDoc returns a new reference.
         */
        SpecialisedError = PyErr_NewExceptionWithDoc(
            "noddy.SpecialsiedError", /* char *name */
            "Some specialised problem description here.", /* char *doc */
            ExceptionBase, /* PyObject *base */
            NULL /* PyObject *dict */);
        if (! SpecialisedError) {
            return NULL;
        } else {
            PyModule_AddObject(m, "SpecialisedError", SpecialisedError);
        }
        /* END: Initialise exceptions here. */
        
        return m;
    }

To illustrate how you raise one of these exceptions suppose we have a function to test raising one of these exceptions:

.. code-block:: c

    static PyMethodDef Noddy_module_methods[] = {
        ...
        {"_test_raise", (PyCFunction)Noddy__test_raise, METH_NOARGS, "Raises a SpecialisedError."},
        ...
        {NULL, NULL, 0, NULL}  /* Sentinel */
    };


We can either access the exception type directly:

.. code-block:: c

    static PyObject *Noddy__test_raise(PyObject *_mod/* Unused */)
    {
        if (SpecialisedError) {
            PyErr_Format(SpecialisedError, "One %d two %d three %d.", 1, 2, 3);
        } else {
            PyErr_SetString(PyExc_RuntimeError, "Can not raise exception, module not initialised correctly");
        }
        return NULL;
    }


Or fish it out of the module (this will be slower):

.. code-block:: c

    static PyObject *Noddy__test_raise(PyObject *mod)
    {
        PyObject *err = PyDict_GetItemString(PyModule_GetDict(mod), "SpecialisedError");
        if (err) {
            PyErr_Format(err, "One %d two %d three %d.", 1, 2, 3);
        } else {
            PyErr_SetString(PyExc_RuntimeError, "Can not find exception in module");
        }
        return NULL;
    }
