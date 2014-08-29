.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

=================================
Parsing Python Arguments 
=================================

This section describes how you write functions that accept Python ``*args`` and ``**kwargs`` arguments and how to extract ``PyObject`` or C fundamental types from them.


------------------------------------
No Arguments
------------------------------------

The simplest from is a global function in a module that takes no arguments at all:

.. code-block:: c

    static PyObject *_parse_no_args(PyObject *module) {
        PyObject *ret = NULL;
    
        /* Your code here...*/
    
        Py_INCREF(Py_None);
        ret = Py_None;
        assert(! PyErr_Occurred());
        assert(ret);
        goto finally;
    except:
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        return ret;
    }

This function is added to the module methods with the ``METH_NOARGS`` value. The Python interpreter will raise a ``TypeError`` in any arguments are offered.

.. code-block:: c

    static PyMethodDef cParseArgs_methods[] = {
        /* Other functions here... */
        {"argsNone", (PyCFunction)_parse_no_args, METH_NOARGS,
            "No arguments."
        },
        /* Other functions here... */
        {NULL, NULL, 0, NULL}  /* Sentinel */
    };

------------------------------------
One Argument
------------------------------------

There is no parsing required here, a single ``PyObject`` is expected:

.. code-block:: c

    static PyObject *_parse_one_arg(PyObject *module,
                                    PyObject *arg
                                    ) {
        PyObject *ret = NULL;
        assert(arg);
        /* Treat arg as a borrowed reference. */
        Py_INCREF(arg);
    
        /* Your code here...*/
    
        Py_INCREF(Py_None);
        ret = Py_None;
        assert(! PyErr_Occurred());
        assert(ret);
        goto finally;
    except:
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        /* Treat arg as a borrowed reference. */
        Py_DECREF(arg);
        return ret;
    }

This function can be added to the module with the ``METH_O`` flag:

.. code-block:: c

    static PyMethodDef cParseArgs_methods[] = {
        /* Other functions here... */
        {"argsOne", (PyCFunction)_parse_one_arg, METH_O,
            "One argument."
        },
        /* Other functions here... */
        {NULL, NULL, 0, NULL}  /* Sentinel */
    };

----------------------------------------------------
Variable Number of Arguments
----------------------------------------------------

The function will be called with two arguments, the module and a ``PyListObject`` that contains a list of arguments. You can either parse this list yourself or use a helper method to parse it into Python and C types.

In the following code we are expecting a string, an integer and an optional integer whose default value is 8. In Python the equivalent function declaration would be::

    def argsOnly(theString, theInt, theOptInt=8):

Here is the C code, note the string that describes the argument types passed to ``PyArg_ParseTuple``, if these types are not present a ``ValueError`` will be set.

.. code-block:: c

    static PyObject *_parse_args(PyObject *module,
                                 PyObject *args
                                 ) {
        PyObject *ret = NULL;
        PyObject *pyStr = NULL;
        int arg1, arg2;
    
        arg2 = 8; /* Default value. */
        if (! PyArg_ParseTuple(args, "Si|i", &pyStr, &arg1, &arg2)) {
            goto except;
        }
    
        /* Your code here...*/
    
        Py_INCREF(Py_None);
        ret = Py_None;
        assert(! PyErr_Occurred());
        assert(ret);
        goto finally;
    except:
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        return ret;
    }

This function can be added to the module with the ``METH_VARARGS`` flag:

.. code-block:: c

    static PyMethodDef cParseArgs_methods[] = {
        /* Other functions here... */
        {"argsOnly", (PyCFunction)_parse_args, METH_VARARGS,
            "Reads args only."
        },
        /* Other functions here... */
        {NULL, NULL, 0, NULL}  /* Sentinel */
    };

--------------------------------------------------------------------------
Variable Number of Arguments and Keyword Arguments
--------------------------------------------------------------------------

The function will be called with two arguments, the module, a ``PyListObject`` that contains a list of arguments and a ``PyDictObject`` that contains a dictionary of keyword arguments. You can either parse these yourself or use a helper method to parse it into Python and C types.

In the following code we are expecting a string, an integer and an optional integer whose default value is 8. In Python the equivalent function declaration would be::

    def argsOnly(theString, theInt, theOptInt=8):

Here is the C code, note the string that describes the argument types passed to ``PyArg_ParseTuple``, if these types are not present a ``ValueError`` will be set.

.. code-block:: c

    static PyObject *_parse_args_kwargs(PyObject *module,
                                        PyObject *args,
                                        PyObject *kwargs
                                        ) {
        PyObject *ret = NULL;
        PyObject *pyStr = NULL;
        int arg2;
        static char *kwlist[] = {
            "argOne", /* bytes object. */
            "argTwo",
            NULL
        };
    
        PyObject_Print(module, stdout, 0);
        fprintf(stdout, "\n");
        PyObject_Print(args, stdout, 0);
        fprintf(stdout, "\n");
        PyObject_Print(kwargs, stdout, 0);
        fprintf(stdout, "\n");
    
        arg2 = 8; /* Default value. */
        if (! PyArg_ParseTupleAndKeywords(args, kwargs, "S|i",
                                          kwlist, &pyStr, &arg2)) {
            goto except;
        }
    
        /* Your code here...*/
    
        Py_INCREF(Py_None);
        ret = Py_None;
        assert(! PyErr_Occurred());
        assert(ret);
        goto finally;
    except:
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        return ret;
    }


This function can be added to the module with the ``METH_VARARGS`` and ``METH_KEYWORDS`` flags:

.. code-block:: c

    static PyMethodDef cParseArgs_methods[] = {
        /* Other functions here... */
        {"argsKwargs", (PyCFunction)_parse_args_kwargs,
            METH_VARARGS | METH_KEYWORDS,
            _parse_args_kwargs_docstring
        },
        /* Other functions here... */
        {NULL, NULL, 0, NULL}  /* Sentinel */
    };

.. note::
    If you use ``|`` in the parser format string you have to set the default values for those optional arguments yourself in the C code. This is pretty straightforward if they are fundamental C types as ``arg2 = 8`` above. For Python values is a bit more tricky as described next.

--------------------------------------------------------------------------
Being Pythonic with Default Arguments
--------------------------------------------------------------------------

If the arguments default to some C fundamental type the code above is fine. However if the arguments default to Python objects then a little more work is needed. Here is a function that has a tuple and a dict as default arguments, in other words the Python signature:

.. code-block:: python

    def function(arg_0=(42, "this"), arg_1={}):

The first argument is immmutable, the second is mutable and so we need to mimic the well known behaviour of Python with mutable arguments. Mutable default arguments are evaluated once only at function definition time and then becomes a (mutable) property of the function. For example:

.. code-block:: python

    >>> def f(l=[]):
    ...   l.append(9)
    ...   print(l)
    ... 
    >>> f()
    [9]
    >>> f()
    [9, 9]
    >>> f([])
    [9]
    >>> f()
    [9, 9, 9]

In C we can get this behaviour by treating the mutable argument as ``static``, the immutable argument does not need to be ``static`` but it will do no harm if it is (if non-``static`` it will have to be initialised on every function call).

My advice: Always make all ``PyObject*`` references to default arguments ``static``.

So first we declare a ``static PyObject*`` for each default argument:

.. code-block:: c

    static PyObject *_parse_args_with_python_defaults(PyObject *module, PyObject *args) {
        PyObject *ret = NULL;
        
        /* This first pointer need not be static as the argument is immutable
         * but if non-static must be NULL otherwise the following code will be undefined.
         */
        static PyObject *pyObjDefaultArg_0; 
        static PyObject *pyObjDefaultArg_1; /* Must be static if mutable. */

Then we declare a ``PyObject*`` for each argument that will either reference the default or the passed in argument. It is important that these ``pyObjArg_...`` pointers are NULL so that we can subsequently detect if ``PyArg_ParseTuple`` has set them non-``NULL``.

.. code-block:: c

        /* These 'working' pointers are the ones we use in the body of the function
         * They either reference the supplied argument or the default (static) argument.
         * We must treat these as "borrowed" references and so must incref them
         * while they are in use then decref them when we exit the function.
         */
        PyObject *pyObjArg_0 = NULL;
        PyObject *pyObjArg_1 = NULL;
 
Then, if the default values have not been initialised, initialise them. In this case it is a bit tedious merely because of the nature of the arguments. So in practice this might be clearer if this was in separate function:

.. code-block:: c

        /* Initialise first argument to its default Python value. */
        if (! pyObjDefaultArg_0) {
            pyObjDefaultArg_0 = PyTuple_New(2);
            if (! pyObjDefaultArg_0) {
                PyErr_SetString(PyExc_RuntimeError, "Can not create tuple!");
                goto except;
            }
            if(PyTuple_SetItem(pyObjDefaultArg_0, 0, PyLong_FromLong(42))) {
                PyErr_SetString(PyExc_RuntimeError, "Can not set tuple[0]!");
                goto except;
            }
            if(PyTuple_SetItem(pyObjDefaultArg_0, 1, PyUnicode_FromString("This"))) {
                PyErr_SetString(PyExc_RuntimeError, "Can not set tuple[1]!");
                goto except;
            }
        }
        /* Now the second argument. */
        if (! pyObjDefaultArg_1) {
            pyObjDefaultArg_1 = PyDict_New();
        }
        

Now parse the given arguments to see what, if anything, is there. ``PyArg_ParseTuple`` will set each working pointer non-``NULL`` if the argument is present. As we set the working pointers ``NULL`` prior to this call we can now tell if any argument is present.
    
.. code-block:: c

        if (! PyArg_ParseTuple(args, "|OO", &pyObjArg_0, &pyObjArg_1)) {
            goto except;
        }

Now switch our working pointers to the default argument if no argument is given. We also treat these as "borrowed" references regardless of whether they are default or supplied so increment the refcount (we must decrement the refcount when done).

.. code-block:: c

        /* First argument. */
        if (! pyObjArg_0) {
            pyObjArg_0 = pyObjDefaultArg_0;
        }
        Py_INCREF(pyObjArg_0);
        
        /* Second argument. */
        if (! pyObjArg_1) {
            pyObjArg_1 = pyObjDefaultArg_1;
        }
        Py_INCREF(pyObjArg_1);

Now write the main body of your function and that must be followed by this clean up code: 

.. code-block:: c

        /* Your code here using pyObjArg_0 and pyObjArg_1 ...*/
    
        Py_INCREF(Py_None);
        ret = Py_None;
        assert(! PyErr_Occurred());
        assert(ret);
        goto finally;

Now the two blocks ``except`` and ``finally``.

.. code-block:: c

    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        /* Decrement refcount to match the increment above. */
        Py_XDECREF(pyObjArg_0);
        Py_XDECREF(pyObjArg_1);
        return ret;
    }

An important point here is the use of ``Py_XDECREF`` in the ``finally:`` block, we can get here through a number of paths, including through the ``except:`` block and in some cases the ``pyObjArg_...`` will be ``NULL`` (for example if ``PyArg_ParseTuple`` fails). So  ``Py_XDECREF`` it must be.

Here is the complete C code:

.. code-block:: c
    :linenos:

    static PyObject *_parse_args_with_python_defaults(PyObject *module, PyObject *args) {
        PyObject *ret = NULL;
        static PyObject *pyObjDefaultArg_0;
        static PyObject *pyObjDefaultArg_1;
        PyObject *pyObjArg_0 = NULL;
        PyObject *pyObjArg_1 = NULL;
    
        if (! pyObjDefaultArg_0) {
            pyObjDefaultArg_0 = PyTuple_New(2);
            if (! pyObjDefaultArg_0) {
                PyErr_SetString(PyExc_RuntimeError, "Can not create tuple!");
                goto except;
            }
            if(PyTuple_SetItem(pyObjDefaultArg_0, 0, PyLong_FromLong(42))) {
                PyErr_SetString(PyExc_RuntimeError, "Can not set tuple[0]!");
                goto except;
            }
            if(PyTuple_SetItem(pyObjDefaultArg_0, 1, PyUnicode_FromString("This"))) {
                PyErr_SetString(PyExc_RuntimeError, "Can not set tuple[1]!");
                goto except;
            }
        }
        if (! pyObjDefaultArg_1) {
            pyObjDefaultArg_1 = PyDict_New();
        }
    
        if (! PyArg_ParseTuple(args, "|OO", &pyObjArg_0, &pyObjArg_1)) {
            goto except;
        }
        if (! pyObjArg_0) {
            pyObjArg_0 = pyObjDefaultArg_0;
        }
        Py_INCREF(pyObjArg_0);
        if (! pyObjArg_1) {
            pyObjArg_1 = pyObjDefaultArg_1;
        }
        Py_INCREF(pyObjArg_1);
    
        /* Your code here...*/
    
        Py_INCREF(Py_None);
        ret = Py_None;
        assert(! PyErr_Occurred());
        assert(ret);
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        Py_XDECREF(pyObjArg_0);
        Py_XDECREF(pyObjArg_1);
        return ret;
    }
