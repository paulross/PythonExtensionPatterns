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

