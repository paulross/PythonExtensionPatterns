.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. index::
    single: Exceptions; General

=================================
Exception Raising 
=================================

This is a brief interlude on how to communicate error conditions from C code to Python.
The example code for this is at ``src/cpy/cExceptions.c`` and the test examples are in
``tests/unit/test_c_exceptions.py``.

These CPython calls are the most useful:

* ``PyErr_SetString(...)`` - To set an exception type with a fixed string.
  `PyErr_SetString() documentation <https://docs.python.org/3/c-api/exceptions.html#c.PyErr_SetString>`_
* ``PyErr_Format(...)`` - To set an exception type with a formatted string.
  `PyErr_Format() documentation <https://docs.python.org/3/c-api/exceptions.html#c.PyErr_Format>`_
* ``PyErr_Occurred()`` - To check if an exception has already been set in the flow of control.
  This returns the current exception or NULL if nothing set.
  `PyErr_Occurred() documentation <https://docs.python.org/3/c-api/exceptions.html#c.PyErr_Occurred>`_
* ``PyErr_Clear()`` - Clearing any set exceptions, have good reason to do this!
  `PyErr_Clear() documentation <https://docs.python.org/3/c-api/exceptions.html#c.PyErr_Clear>`_
* ``PyErr_Print()`` - Print a representation of the current exception then clear any set exceptions.
  `PyErr_Print() documentation <https://docs.python.org/3.13/c-api/exceptions.html#c.PyErr_Print>`_

Indicating an error condition is a two stage process; your code must register an exception and then indicate failure
by returning ``NULL``. Here is a C function doing just that:

.. code-block:: c

    static PyObject *raise_error(PyObject *module) {
        PyErr_SetString(PyExc_ValueError, "Ooops.");
        return NULL;
    }

You might want some dynamic information in the exception object, in that case ``PyErr_Format`` will do:

.. code-block:: c

    static PyObject *raise_error_formatted(PyObject *module) {
        PyErr_Format(PyExc_ValueError,
                     "Can not read %d bytes when offset %d in byte length %d.", \
                     12, 25, 32
                     );
        return NULL;
    }

If one of the two actions is missing then the exception will not be raised correctly.
For example returning ``NULL`` without setting an exception type will raise a ``SystemError``.
For example:

.. code-block:: c

    /* Illustrate returning NULL but not setting an exception. */
    static PyObject *raise_error_bad(PyObject *module) {
        return NULL;
    }

Executing this from Python will produce a clear error message::

    >>> cException.raise_error_bad()
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    SystemError: error return without exception set

If the opposite error is made, that is setting an exception but not signalling then the function will succeed but leave
a later runtime error:

.. code-block:: c

    static PyObject *raise_error_silent(PyObject *module) {
        PyErr_SetString(PyExc_ValueError, "ERROR: raise_error_mixup()");
        Py_RETURN_NONE;
    }

The confusion can arise is that if a subsequent function then tests to see if an exception is set, if so signal it.
It will appear to the Python interpreter that the error is coming from the second function when actually it is from
the first:

.. code-block:: c

    static PyObject *raise_error_silent_test(PyObject *module) {
        if (PyErr_Occurred()) {
            return NULL;
        }
        Py_RETURN_NONE;
    }

A common defensive pattern to use in functions to check an exception has been set by another function but not signalled
is to use ``assert(!PyErr_Occurred());`` at the begining of the function.

The other thing to note is that if there are multiple calls to ``PyErr_SetString`` only the last one counts:

.. code-block:: c

    static PyObject *raise_error_overwrite(PyObject *module) {
        /* This will be ignored. */
        PyErr_SetString(PyExc_RuntimeError, "FORGOTTEN.");
        PyErr_SetString(PyExc_ValueError, "ERROR: raise_error_overwrite()");
        assert(PyErr_Occurred());
        return NULL;
    }

.. index::
    single: Exceptions; Common Exception Patterns

---------------------------------
Common Exception Patterns
---------------------------------

Here are some common use cases for raising exceptions.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Type Checking
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A common requirement is to check the types of the arguments and raise a ``TypeError`` if they are wrong.
Here is an example where we require a ``bytes`` object:

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
            return NULL;
        }
        /* ... */
    }

That's fine if you have a macro such as ``PyBytes_Check`` and for your own types you can create a couple of suitable
macros:

.. code-block:: c

    #define PyMyType_CheckExact(op) (Py_TYPE(op) == &PyMyType_Type)     /* Exact match. */
    #define PyMyType_Check(op) PyObject_TypeCheck(op, &PyMyType_Type)   /* Exact or derived. */

Incidentally ``PyObject_TypeCheck`` is defined as:

.. code-block:: c

    #define PyObject_TypeCheck(ob, tp) \
        (Py_TYPE(ob) == (tp) || PyType_IsSubtype(Py_TYPE(ob), (tp)))

.. index::
    single: Exceptions; Specialised Exceptions

---------------------------------
Creating Specialised Exceptions
---------------------------------

Often you need to create an Exception class that is specialised to a particular module.
The following C code is equivalent to the Python code:

.. code-block:: python

    class ExceptionBase(Exception):
        pass

    class SpecialisedError(ExceptionBase):
        pass

Declaring Specialised Exceptions
---------------------------------

Firstly declare the ``PyObject *`` exception:

.. code-block:: c

    /** Specialise exceptions base exception. */
    static PyObject *ExceptionBase = NULL;
    /** Specialise exceptions derived from base exception. */
    static PyObject *SpecialisedError = NULL;

    /* NOTE: Functions that might raise one of these exceptions will go here. See below. */



Example Module
-----------------------------------

Now define the module, ``cExceptions_methods`` is explained later:

.. code-block:: c

    static PyModuleDef cExceptions_module = {
            PyModuleDef_HEAD_INIT,
            "cExceptions",
            "Examples of raising exceptions.",
            -1,
            cExceptions_methods,
            NULL, NULL, NULL, NULL,
    };

Initialising Specialised Exceptions
-----------------------------------

This can be done quite easily using either the ``PyErr_NewException`` or the ``PyErr_NewExceptionWithDoc`` functions.
These create new exception classes that can be added to a module.
For example, initialise the module, this registers the exception types and the class hierarchy:

.. code-block:: c

    PyMODINIT_FUNC
    PyInit_cExceptions(void) {
        PyObject *m = PyModule_Create(&cExceptions_module);
        if (m == NULL) {
            return NULL;
        }
        /* Initialise exceptions here.
         *
         * Firstly a base class exception that inherits from the builtin Exception.
         * This is achieved by passing NULL as the PyObject* as the third argument.
         *
         * PyErr_NewExceptionWithDoc returns a new reference.
         */
        ExceptionBase = PyErr_NewExceptionWithDoc(
                "cExceptions.ExceptionBase", /* char *name */
                "Base exception class for the module.", /* char *doc */
                NULL, /* PyObject *base, resolves to PyExc_Exception. */
                NULL /* PyObject *dict */);
        /* Error checking: this is oversimplified as it should decref
         * anything created above such as m.
         */
        if (!ExceptionBase) {
            return NULL;
        } else {
            Py_INCREF(ExceptionBase);
            PyModule_AddObject(m, "ExceptionBase", ExceptionBase);
        }
        /* Now a subclass exception that inherits from the base exception above.
         * This is achieved by passing non-NULL as the PyObject* as the third argument.
         *
         * PyErr_NewExceptionWithDoc returns a new reference.
         */
        SpecialisedError = PyErr_NewExceptionWithDoc(
                "cExceptions.SpecialsiedError", /* char *name */
                "Some specialised problem description here.", /* char *doc */
                ExceptionBase, /* PyObject *base, declared above. */
                NULL /* PyObject *dict */);
        if (!SpecialisedError) {
            return NULL;
        } else {
            Py_INCREF(SpecialisedError);
            PyModule_AddObject(m, "SpecialisedError", SpecialisedError);
        }
        /* END: Initialise exceptions here. */
        return m;
    }

Raising Specialise Exceptions
-----------------------------

To illustrate how you raise one of these exceptions suppose we have a function to test raising one of these exceptions:

.. code-block:: c

    static PyMethodDef Noddy_module_methods[] = {
        // ...
        {
            "raise_exception_base",
            (PyCFunction) raise_exception_base,
            METH_NOARGS,
            "Raises a ExceptionBase."
        },
        {
            "raise_specialised_error",
            (PyCFunction) raise_specialised_error,
            METH_NOARGS,
            "Raises a SpecialisedError."
        },
        // ...
        {NULL, NULL, 0, NULL}  /* Sentinel */
    };

We can either access the exception type directly:

.. code-block:: c

    /** Raises a ExceptionBase. */
    static PyObject *raise_exception_base(PyObject *Py_UNUSED(module)) {
        if (ExceptionBase) {
            PyErr_Format(ExceptionBase, "One %d two %d three %d.", 1, 2, 3);
        } else {
            PyErr_SetString(
                PyExc_RuntimeError,
                "Can not raise exception, module not initialised correctly"
            );
        }
        return NULL;
    }

    /** Raises a SpecialisedError. */
    static PyObject *raise_specialised_error(PyObject *Py_UNUSED(module)) {
        if (SpecialisedError) {
            PyErr_Format(SpecialisedError, "One %d two %d three %d.", 1, 2, 3);
        } else {
            PyErr_SetString(
                PyExc_RuntimeError,
                "Can not raise exception, module not initialised correctly"
            );
        }
        return NULL;
    }

Or fish it out of the module (this will be slower):

.. code-block:: c

    static PyObject *raise_specialised_error(PyObject *module)
    {
        PyObject *err = PyDict_GetItemString(PyModule_GetDict(module), "SpecialisedError");
        if (err) {
            PyErr_Format(err, "One %d two %d three %d.", 1, 2, 3);
        } else {
            PyErr_SetString(PyExc_RuntimeError, "Can not find exception in module");
        }
        return NULL;
    }

Here is some test code from ``tests/unit/test_c_exceptions.py``:

.. code-block:: python

    from cPyExtPatt import cExceptions

    def test_raise_exception_base():
        with pytest.raises(cExceptions.ExceptionBase) as err:
            cExceptions.raise_exception_base()
        assert err.value.args[0] == 'One 1 two 2 three 3.'


    def test_raise_specialised_error():
        with pytest.raises(cExceptions.SpecialisedError) as err:
            cExceptions.raise_specialised_error()
        assert err.value.args[0] == 'One 1 two 2 three 3.'
