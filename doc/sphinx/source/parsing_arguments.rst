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
        /* arg as a borrowed reference and the general rule is that you Py_INCREF them
         * whilst you have an interest in them. We do _not_ do that here for reasons
         * explained below.
         */
        // Py_INCREF(arg);
    
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
        /* If we were to treat arg as a borrowed reference and had Py_INCREF'd above we
         * should do this. See below. */
        // Py_DECREF(arg);
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

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Arguments as Borrowed References
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There is some subtlety here as indicated by the comments. ``*arg`` is not our reference, it is a borrowed reference so why don't we increment it at the beginning of this function and decrement it at the end? After all we are trying to protect against calling into some malicious/badly written code that could hurt us. For example:

.. code-block:: c

    static PyObject *foo(PyObject *module,
                         PyObject *arg
                         ) {
        /* arg has a minimum recount of 1. */
        call_malicious_code_that_decrefs_by_one_this_argument(arg);
        /* arg potentially could have had a ref count of 0 and been deallocated. */
        /* ... */
        /* So now doing something with arg could be undefined. */
    }

A solution would be, since ``arg`` is a 'borrowed' reference and borrowed references should always be incremented whilst in use and decremented when done with. This would suggest the following:

.. code-block:: c

    static PyObject *foo(PyObject *module,
                         PyObject *arg
                         ) {
        /* arg has a minimum recount of 1. */
        Py_INCREF(arg);
        /* arg now has a minimum recount of 2. */
        call_malicious_code_that_decrefs_by_one_this_argument(arg);
        /* arg can not have a ref count of 0 so is safe to use. */
        /* Use arg to your hearts content... */
        /* Do a matching decref. */
        Py_DECREF(arg);
        /* But now arg could have had a ref count of 0 so is unsafe to use by the caller. */
    }

But now we have just pushed the burden onto our caller. They created ``arg`` and passed it to us in good faith and whilst we have protected ourselves have not protected the caller and they can fail unexpectedly. So it is best to fail fast, an near the error site, that dastardly ``call_malicious_code_that_decrefs_by_one_this_argument()``.

Side note: Of course this does not protect you from malicious/badly written code that decrements by more than one :-)

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

    def argsKwargs(theString, theOptInt=8):

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
            "theString",
            "theOptInt",
            NULL
        };
    
        /* If you are interested this is a way that you can trace the input.
        PyObject_Print(module, stdout, 0);
        fprintf(stdout, "\n");
        PyObject_Print(args, stdout, 0);
        fprintf(stdout, "\n");
        PyObject_Print(kwargs, stdout, 0);
        fprintf(stdout, "\n");
         * End trace */
    
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

All arguments are keyword arguments so this function can be called in a number of ways, all of the following are equivalent:

.. code-block:: python
    
    argsKwargs('foo')
    argsKwargs('foo', 8)
    argsKwargs(theString='foo')
    argsKwargs(theOptInt=8, theString='foo')
    argsKwargs(theString, theOptInt=8)

If you want the function signature to be ``argsKwargs(theString, theOptInt=8)`` with a single argument and a single optional keyword argument then put an empty string in the kwlist array:

.. code-block:: c

        /* ... */
        static char *kwlist[] = {
            "",
            "theOptInt",
            NULL
        };
        /* ... */

.. note::
    If you use ``|`` in the parser format string you have to set the default values for those optional arguments yourself in the C code. This is pretty straightforward if they are fundamental C types as ``arg2 = 8`` above. For Python values is a bit more tricky as described next.
    
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Keyword Arguments and C++11
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

C++11 compilers warn when creating non-const ``char*`` from string literals as we have done with the keyword array above. The solution is to declare these ``const char*`` however ``PyArg_ParseTupleAndKeywords`` expects a ``char **``. The solution is to cast away const in the call:

.. code-block:: c

    /* ... */
    static const char *kwlist[] = { "foo", "bar", "baz", NULL };
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "OOO",
                                      const_cast<char**>(kwlist),
                                      &foo, &bar, &baz)) {
        return NULL;
    }
    /* ... */


.. _cpython_default_arguments:

--------------------------------------------------------------------------
Being Pythonic with Default Arguments
--------------------------------------------------------------------------

If the arguments default to some C fundamental type the code above is fine. However if the arguments default to Python objects then a little more work is needed. Here is a function that has a tuple and a dict as default arguments, in other words the Python signature:

.. code-block:: python

    def function(arg_0=(42, "this"), arg_1={}):

The first argument is immutable, the second is mutable and so we need to mimic the well known behaviour of Python with mutable arguments. Mutable default arguments are evaluated once only at function definition time and then becomes a (mutable) property of the function. For example:

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

^^^^^^^^^^^^^^^^^^^^^^^^
Simplifying Macros
^^^^^^^^^^^^^^^^^^^^^^^^

For simple default values some macros may help. The first one declares and initialises the default value. It takes three arguments:

* The name of the argument variable, a static ``PyObject`` named ``default_<name>`` will also be created.
* The default value which should return a new reference.
* The value to return on failure to create a default value, usually -1 or ``NULL``.

.. code-block:: c

    #define PY_DEFAULT_ARGUMENT_INIT(name, value, ret) \
        PyObject *name = NULL; \
        static PyObject *default_##name = NULL; \
        if (! default_##name) { \
            default_##name = value; \
            if (! default_##name) { \
                PyErr_SetString(PyExc_RuntimeError, "Can not create default value for " #name); \
                return ret; \
            } \
        }

The second one assigns the argument to the default if it is not initialised and increments the reference count. It just takes the name of the argument:

.. code-block:: c

    #define PY_DEFAULT_ARGUMENT_SET(name) if (! name) name = default_##name; \
        Py_INCREF(name)

And they can be used like this when implementing a Python function signature such as::
    
    def do_something(self, encoding='utf-8', the_id=0, must_log=True):
        # ...
        return None

Here is that function implemented in C:

.. code-block:: c

    static PyObject*
    do_something(something *self, PyObject *args, PyObject *kwds) {
        PyObject *ret = NULL;
        /* Initialise default arguments. Note: these might cause an early return. */
        PY_DEFAULT_ARGUMENT_INIT(encoding,  PyUnicode_FromString("utf-8"),  NULL);
        PY_DEFAULT_ARGUMENT_INIT(the_id,    PyLong_FromLong(0L),            NULL);
        PY_DEFAULT_ARGUMENT_INIT(must_log,  PyBool_FromLong(1L),            NULL);

        static const char *kwlist[] = { "encoding", "the_id", "must_log", NULL };
        if (! PyArg_ParseTupleAndKeywords(args, kwds, "|Oip",
                                          const_cast<char**>(kwlist),
                                          &encoding, &the_id, &must_log)) {
            return NULL;
        }
        /* 
         * Assign absent arguments to defaults and increment the reference count.
         * Don't forget to decrement the reference count before returning!
         */
        PY_DEFAULT_ARGUMENT_SET(encoding);
        PY_DEFAULT_ARGUMENT_SET(the_id);
        PY_DEFAULT_ARGUMENT_SET(must_log);

        /*
         * Use encoding, the_id, must_log from here on...
         */

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
        Py_DECREF(encoding);
        Py_DECREF(the_id);
        Py_DECREF(must_log);
        return ret;
    }

^^^^^^^^^^^^^^^^^^^^^^^^
Simplifying C++11 class
^^^^^^^^^^^^^^^^^^^^^^^^

With C++ we can make this a bit smoother. We declare a class thus:

.. code-block:: cpp

    /** Class to simplify default arguments.
     *
     * Usage:
     *
     * static DefaultArg arg_0(PyLong_FromLong(1L));
     * static DefaultArg arg_1(PyUnicode_FromString("Default string."));
     * if (! arg_0 || ! arg_1) {
     *      return NULL;
     * }
     *
     * if (! PyArg_ParseTupleAndKeywords(args, kwargs, "...",
                                         const_cast<char**>(kwlist),
                                         &arg_0, &arg_1, ...)) {
            return NULL;
        }
     *
     * Then just use arg_0, arg_1 as if they were a PyObject* (possibly
     * might need to be cast to some specific PyObject*).
     *
     * WARN: This class is designed to be statically allocated. If allocated
     * on the heap or stack it will leak memory. That could be fixed by
     * implementing:
     *
     * ~DefaultArg() { Py_XDECREF(m_default); }
     *
     * But this will be highly dangerous when statically allocated as the
     * destructor will be invoked with the Python interpreter in an
     * uncertain state and will, most likely, segfault:
     * "Python(39158,0x7fff78b66310) malloc: *** error for object 0x100511300: pointer being freed was not allocated"
     */
    class DefaultArg {
    public:
        DefaultArg(PyObject *new_ref) : m_arg { NULL }, m_default { new_ref } {}
        // Allow setting of the (optional) argument with
        // PyArg_ParseTupleAndKeywords
        PyObject **operator&() { m_arg = NULL; return &m_arg; }
        // Access the argument or the default if default.
        operator PyObject*() const {
            return m_arg ? m_arg : m_default;
        }
        // Test if constructed successfully from the new reference.
        explicit operator bool() { return m_default != NULL; }
    protected:
        PyObject *m_arg;
        PyObject *m_default;
    };


And we can use ``DefaultArg`` like this:

.. code-block:: c

    static PyObject*
    do_something(something *self, PyObject *args, PyObject *kwds) {
        PyObject *ret = NULL;
        /* Initialise default arguments. */
        static DefaultArg encoding { PyUnicode_FromString("utf-8") };
        static DefaultArg the_id { PyLong_FromLong(0L) };
        static DefaultArg must_log { PyBool_FromLong(1L) };
        
        /* Check that the defaults are non-NULL i.e. succesful. */
        if (!encoding || !the_id || !must_log) {
            return NULL;
        }
        
        static const char *kwlist[] = { "encoding", "the_id", "must_log", NULL };
        /* &encoding etc. accesses &m_arg in DefaultArg because of PyObject **operator&() */
        if (! PyArg_ParseTupleAndKeywords(args, kwds, "|Oip",
                                          const_cast<char**>(kwlist),
                                          &encoding, &the_id, &must_log)) {
            return NULL;
        }
        /*
         * Use encoding, the_id, must_log from here on as PyObject* since we have
         * operator PyObject*() const ...
         *
         * So if we have a function:
         * set_encoding(PyObject *obj) { ... }
         */
         set_encoding(encoding);
        /* ... */
    }
