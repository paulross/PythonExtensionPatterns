.. highlight:: python
    :linenothreshold: 20

.. toctree::
    :maxdepth: 3

***************************
Parsing Python Arguments
***************************

This section describes how you write functions that accept Python ``*args`` and ``**kwargs``
arguments and how to extract ``PyObject`` or C fundamental types from them.


Specifying the Function Arguments
====================================

Python has a myriad of ways of function arguments and the C API reflects that.
Two important features of C functions are:

- Declaring them correctly with the right signature and flags.
- Parsing the arguments and checking their types.

These are described below.

C Function Declaration
-------------------------------

The C function signature must be declared correctly depending on the arguments it is expected to work with.
Here is a small summary of the required declaration.

.. list-table:: Function Arguments
   :widths: 10 10 30 20
   :header-rows: 1

   * - Arguments
     - Flags
     - C Function Signature
     - Description
   * - None
     - `METH_NOARGS <https://docs.python.org/3/c-api/structures.html#c.METH_NOARGS>`_
     - ``PyObject *PyCFunction(PyObject *self, PyObject *args);`` `Docs <https://docs.python.org/3/c-api/structures.html#c.PyCFunction>`_
     - Second argument will be ``NULL``.
   * - One
     - `METH_O <https://docs.python.org/3/c-api/structures.html#c.METH_O>`_
     - ``PyObject *PyCFunction(PyObject *self, PyObject *args);`` `Docs <https://docs.python.org/3/c-api/structures.html#c.PyCFunction>`_
     - Second argument will be the single argument.
   * - Positional
     - `METH_VARARGS <https://docs.python.org/3/c-api/structures.html#c.METH_VARARGS>`_
     - ``PyObject *PyCFunction(PyObject *self, PyObject *args);`` `Docs <https://docs.python.org/3/c-api/structures.html#c.PyCFunction>`_
     - Second value will be a sequence of arguments.
   * - Positional and keywords
     - `METH_NOARGS | METH_KEYWORDS <https://docs.python.org/3/c-api/structures.html#c.METH_KEYWORDS>`_
     - ``PyObject *PyCFunctionWithKeywords(PyObject *self, PyObject *args, PyObject *kwargs);`` `Docs <https://docs.python.org/3/c-api/structures.html#c.PyCFunctionWithKeywords>`_
     -

.. note::

    I don't cover the use of `METH_FASTCALL <https://docs.python.org/3/c-api/structures.html#c.METH_FASTCALL>`_
    in this tutorial.

.. note::

    `METH_KEYWORDS <https://docs.python.org/3/c-api/structures.html#c.METH_KEYWORDS>`_
    can only be used in combination with other flags.

Example
^^^^^^^^

A function that takes positional and keyword arguments would be declared as:

.. code-block:: c

    static PyObject *
    parse_args_kwargs(PyObject *module, PyObject *args, PyObject *kwargs);

And this would be added to the module, say, by using:

.. code-block:: c

    static PyMethodDef cParseArgs_methods[] = {
            /* ... */
            {
                "parse_args_kwargs",
                (PyCFunction) parse_args_kwargs,
                METH_VARARGS | METH_KEYWORDS,
                "Documentation for parse_args_kwargs()."
            },
            /* ... */
            {NULL, NULL, 0, NULL} /* Sentinel */
    };

Parsing the Arguments
------------------------------

Once whe have the C function correctly declared then the arguments have to parsed according to their types.
This is done using the `PyArg_ParseTuple <https://docs.python.org/3/c-api/arg.html#c.PyArg_ParseTuple>`_
and `PyArg_ParseTupleAndKeywords <https://docs.python.org/3/c-api/arg.html#c.PyArg_ParseTupleAndKeywords>`_
(ignoring “old-style” functions which use `PyArg_Parse <https://docs.python.org/3/c-api/arg.html#c.PyArg_Parse>`_).

These use formatting strings that can become bewilderingly complex so this tutorial uses examples as an introduction.
The reference documentation is excellent: `argument parsing and building values <https://docs.python.org/3/c-api/arg.html>`_.

.. warning::

    Error messages can be slightly misleading as the argument index is the *C* index
    not the *Python* index. For example a C function using ``METH_VARARGS`` declared as
    ``static PyObject *parse_args(PyObject *module, PyObject *args);``
    which expects the Python argument[0] to be a bytes object and the Python argument[1]
    to be an integer by using ``PyArg_ParseTuple(args, "Si", &arg0, &arg1)``.

    Calling this from Python with ``parse_args(21, 22)`` will give
    ``TypeError: argument 1 must be bytes, not int``.

    The "1" here refers to the C index not the Python index.
    The correct call would be to change the 0th Python argument to
    ``cParseArgs.parse_args(b'21', 22)``

    Also consider the signature ``def parse_args(a: bytes, b: int, c: str = '') -> int:``
    Called with ``parse_args(b'bytes', '456')`` gives the error "'str' object cannot be interpreted as an integer"
    without specifying which argument it is referring to.

Examples
====================================

These examples are in ``src/cpy/cParseArgs.c`` and their tests are in ``tests/unit/test_c_parse_args.py``.

No Arguments
------------------------------------

The simplest form is a global function in a module that takes no arguments at all:

.. code-block:: c

    static PyObject *parse_no_args(PyObject *Py_UNUSED(module)) {

        /* Your code here...*/

        Py_RETURN_NONE;
    }

This function is added to the module methods with the ``METH_NOARGS`` value.
The Python interpreter will raise a ``TypeError`` in any arguments are offered.

.. code-block:: c

    static PyMethodDef cParseArgs_methods[] = {
        /* Other functions here... */
        {
            "parse_no_args",
            (PyCFunction)parse_no_args,
            METH_NOARGS,
            "No arguments."
        },
        /* Other functions here... */
        {NULL, NULL, 0, NULL}  /* Sentinel */
    };


One Argument
------------------------------------

There is no parsing required here, a single ``PyObject`` is expected:

.. code-block:: c

    static PyObject *parse_one_arg(PyObject *Py_UNUSED(module), PyObject *Py_UNUSED(arg)) {
        /* arg as a borrowed reference and the general rule is that you Py_INCREF them
         * whilst you have an interest in them.
         */
        // Py_INCREF(arg);

        /* Your code here...*/

        /* If we were to treat arg as a borrowed reference and had Py_INCREF'd above we
         * should do this. See below. */
        // Py_DECREF(arg);
        Py_RETURN_NONE;
    }



This function can be added to the module with the ``METH_O`` flag:

.. code-block:: c

    static PyMethodDef cParseArgs_methods[] = {
        /* Other functions here... */
        {
            "parse_one_arg",
            (PyCFunction) parse_one_arg,
            METH_O,
            "One argument."
        },
        /* Other functions here... */
        {NULL, NULL, 0, NULL}  /* Sentinel */
    };


Arguments as Borrowed References
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There is some subtlety here as indicated by the comments. ``*arg`` is not our reference, it is a borrowed reference
so why don't we increment it at the beginning of this function and decrement it at the end?
After all we are trying to protect against calling into some malicious/badly written code that could hurt us.
For example:

.. code-block:: c

    static PyObject *foo(PyObject *module, PyObject *arg) {
        /* arg has a minimum recount of 1. */
        call_malicious_code_that_decrefs_by_one_this_argument(arg);
        /* arg potentially could have had a ref count of 0 and been deallocated. */
        /* ... */
        /* So now doing something with arg could be undefined. */
    }

A solution would be, since ``arg`` is a 'borrowed' reference and borrowed references should always be incremented
whilst in use and decremented when done with.
This would suggest the following:

.. code-block:: c

    static PyObject *foo(PyObject *module, PyObject *arg) {
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

But now we have just pushed the burden onto our caller.
They created ``arg`` and passed it to us in good faith and whilst we have protected ourselves have not protected the
caller and they can fail unexpectedly.
So it is best to fail fast, an near the error site, that dastardly
``call_malicious_code_that_decrefs_by_one_this_argument()``.

Side note: Of course this does not protect you from malicious/badly written code that decrements by more than one :-)

Variable Number of Arguments
----------------------------------------------------

The function will be called with two arguments, the module and a ``PyTupleObject`` that contains a tuple of arguments.
You can either parse this list yourself or use a helper method to parse it into Python and C types.

In the following code we are expecting a bytes object, an integer and an optional string whose default value is
'default_string'.
For demonstration purposes, this returns the same three arguments.
In Python the equivalent function declaration would be::

    def parse_args(a: bytes, b: int, c: str = 'default_string') -> typing.Tuple[bytes, int, str]:

Here is the C code, note the string that describes the argument types passed to ``PyArg_ParseTuple``, if these types
are not present a ``ValueError`` will be set.

.. code-block:: c

    static PyObject *parse_args(PyObject *Py_UNUSED(module), PyObject *args) {
        PyObject *arg_0 = NULL;
        int arg_1;
        char *arg_2 = "default_string";

        if (!PyArg_ParseTuple(args, "Si|s", &arg_0, &arg_1, &arg_2)) {
            return NULL;
        }

        /* Your code here...*/

        return Py_BuildValue("Ois", arg_0, arg_1, arg_2);
    }

This function can be added to the module with the ``METH_VARARGS`` flag:

.. code-block:: c

    static PyMethodDef cParseArgs_methods[] = {
        /* Other functions here... */
        {
            "parse_args",
            (PyCFunction) parse_args,
            METH_VARARGS,
            "parse_args() documentation"
        },
        /* Other functions here... */
        {NULL, NULL, 0, NULL}  /* Sentinel */
    };

This code can be seen in ``src/cpy/cParseArgs.c``.
It is tested in ``tests.unit.test_c_parse_args.test_parse_args``.
Failure modes, when the wrong arguments are passed are tested in ``tests.unit.test_c_parse_args.test_parse_args_raises``.
Note the wide variety of error messages that are obtained.

Variable Number of Arguments and Keyword Arguments
--------------------------------------------------------------------------

The function will be called with three arguments, the module, a ``PyTupleObject`` that contains a tuple of arguments
and a ``PyDictObject`` that contains a dictionary of keyword arguments.
You can either parse these yourself or use a helper method to parse it into Python and C types.

In the following code we are expecting a sequence and an optional integer defaulting to 1.
It returns the `sequence` repeated `count` times.
In Python the equivalent function declaration would be::

    def parse_args_kwargs(sequence=typing.Sequence[typing.Any], count: = 1) -> typing.Sequence[typing.Any]:
        return sequence * count

Here is the C code, note the string that describes the argument types passed to ``PyArg_ParseTupleAndKeywords``,
if these types are not present a ``ValueError`` will be set.

.. code-block:: c

    static PyObject *
    parse_args_kwargs(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwargs) {
        PyObject *ret = NULL;
        PyObject *py_sequence = NULL;
        int count = 1;          /* Default. */
        static char *kwlist[] = {
                "sequence", /* A sequence object, str, list, tuple etc. */
                "count", /* Python int converted to a C int. */
                NULL,
        };

        if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|i", kwlist, &py_sequence, &count)) {
            goto except;
        }

        /* Your code here...*/

        ret = PySequence_Repeat(py_sequence, count);
        if (ret == NULL) {
            goto except;
        }
        assert(!PyErr_Occurred());
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        return ret;
    }

This function can be added to the module with both the ``METH_VARARGS`` and ``METH_KEYWORDS`` flags set:

.. code-block:: c

    static PyMethodDef cParseArgs_methods[] = {
        /* Other functions here... */
        {
            "parse_args_kwargs",
            (PyCFunction) parse_args_kwargs,
            METH_VARARGS | METH_KEYWORDS,
            "parse_args_kwargs() documentation"
        },
        /* Other functions here... */
        {NULL, NULL, 0, NULL}  /* Sentinel */
    };

This code can be seen in ``src/cpy/cParseArgs.c``.
It is tested in ``tests.unit.test_c_parse_args.test_parse_args_kwargs`` which shows the variety of ways this can be
called.
Failure modes, when the wrong arguments are passed are tested in
``tests.unit.test_c_parse_args.test_parse_args_kwargs_raises``.

All arguments are keyword arguments so this function can be called in a number of ways, all of the following are equivalent:

.. code-block:: python

    cParseArgs.parse_args_kwargs([1, 2, 3], 2)
    cParseArgs.parse_args_kwargs([1, 2, 3], count=2)
    cParseArgs.parse_args_kwargs(sequence=[1, 2, 3], count=2)

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


Default String and Bytes Arguments
------------------------------------------

The recommended way to accept binary data is to parse the argument using the ``"y*"`` formatting string and supply
a `Py_Buffer <https://docs.python.org/3/c-api/buffer.html#c.Py_buffer>`_ argument.
This also applies to strings, using the ``"s*"`` formatting string, where they might contain ``'\0'`` characters.

Typically this would be done with C code such as this:

.. code-block:: c

    Py_buffer arg;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*", kwlist, &arg)) {
        assert(PyErr_Occurred());
        return NULL;
    }
    /* arg.buf has the byte data of length arg.len */

However this will likely segfault if it is used as a default argument using ``"|y*"`` formatting string as the
Py_Buffer is uninitialized.
Here is the fix for using a default value of ``b''``:

.. code-block:: c

    Py_buffer arg;
    arg.buf = NULL;
    arg.len = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|y*", kwlist, &arg)) {
        assert(PyErr_Occurred());
        return NULL;
    }


For a different default value, say ``b'default'``, then this will work.
The Python signature is:

.. code-block:: python

    def parse_default_bytes_object(b: bytes = b"default") -> bytes:
        pass

The complete C code is:

.. code-block:: c

    static PyObject *
    parse_default_bytes_object(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwargs) {
        static const char *arg_default = "default";
        Py_buffer arg;
        arg.buf = (void *)arg_default;
        arg.len = strlen(arg_default);
        static char *kwlist[] = {
                "b",
                NULL,
        };
        if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|y*", kwlist, &arg)) {
            assert(PyErr_Occurred());
            return NULL;
        }
        return Py_BuildValue("y#", arg.buf, arg.len);
    }

.. _cpython_default_mutable_arguments:

Being Pythonic with Default Mutable Arguments
--------------------------------------------------------------------------

If the arguments default to some C fundamental type the code above is fine.
However if the arguments default to Python objects then a little more work is needed.
Here is a function that has a tuple and a dict as default arguments, in other words the Python signature:

.. code-block:: python

    def function(arg_0=(42, "this"), arg_1={}):

The first argument is immutable, the second is mutable and so we need to mimic the well known behaviour of Python
with mutable arguments.
Mutable default arguments are evaluated once only at function definition time and then becomes a (mutable) property of
the function.
For example:

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

In C we can get this behaviour by treating the mutable argument as ``static``, the immutable argument does not need to
be ``static`` but it will do no harm if it is (if non-``static`` it will have to be initialised on every function call).

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


Positional Only and Keyword Only Arguments
-----------------------------------------------

This section shows how to achieve
`positional only <https://docs.python.org/3/glossary.html#positional-only-parameter>`_
and `keyword only <https://docs.python.org/3/glossary.html#keyword-only-parameter>`_ arguments in a C extension.
These are described in the Python documentation for
`Special parameters <https://docs.python.org/3/tutorial/controlflow.html#special-parameters>`_
Specifically `positional only parameters <https://docs.python.org/3/tutorial/controlflow.html#positional-only-parameters>`_
and `keyword only arguments <https://docs.python.org/3/tutorial/controlflow.html#keyword-only-arguments>`_

Suppose we want the functional equivalent of the Python function signature
(reproducing https://docs.python.org/3/tutorial/controlflow.html#special-parameters )::

    def parse_pos_only_kwd_only(pos1: str, pos2: int, /, pos_or_kwd: bytes, *, kwd1: float, kwd2: int):
        return None

This is achieved by combining two techniques:

- Positional only: The strings in the ``*kwlist`` passed to ``PyArg_ParseTupleAndKeywords`` are empty.
- Keyword only: The formatting string passed to ``PyArg_ParseTupleAndKeywords`` uses the ``'$'`` character.

A function using either positional only or keyword only arguments must use the flags ``METH_VARARGS | METH_KEYWORDS``
and uses``PyArg_ParseTupleAndKeywords``. Currently, all keyword-only arguments must also be optional arguments, so ``'|'`` must always be
specified before ``'$'`` in the format string.

Here is the C code:

.. code-block:: c

    static PyObject *
    parse_pos_only_kwd_only(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwargs) {
        /* Arguments, first three are required. */
        Py_buffer pos1;
        int pos2;
        Py_buffer pos_or_kwd;
        /* Last two are optional. */
        double kwd1 = 256.0;
        int kwd2 = -421;
        static char *kwlist[] = {
                "",                 /* pos1 is positional only. */
                "",                 /* pos2 is positional only. */
                "pos_or_kwd",       /* pos_or_kwd can be positional or keyword argument. */
                "kwd1",             /* kwd1 is keyword only argument by use of '$' in format string. */
                "kwd2",             /* kwd2 is keyword only argument by use of '$' in format string. */
                NULL,
        };

        if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s*iy*|$di", kwlist, &pos1, &pos2, &pos_or_kwd, &kwd1, &kwd2)) {
            assert(PyErr_Occurred());
            return NULL;
        }
        /* Return the parsed arguments. */
        return Py_BuildValue("s#iy#di", pos1.buf, pos1.len, pos2, pos_or_kwd.buf, pos_or_kwd.len, kwd1, kwd2);
    }



Parsing Arguments With a Functional Conversion to C
---------------------------------------------------------

Often you want to convert a Python argument to a C value(s) in a way that is not covered by the format strings
provided by the Python C API. To do this you can provide a special conversion function in C and give it to
``PyArg_ParseTuple`` or ``PyArg_ParseTupleAndKeywords``.

In this example we are expecting the Python argument to be a list of integers and we want the sum of them as
a C ``long``. First create a C function that takes a Python list, checks it and sums the values.
The function returns 1 on success or 0 on error and, in that case, and exception is expected to be set.
On success the result will be written into the opaque pointer, here called ``address``:

.. code-block:: c

    int sum_list_of_longs(PyObject *list_longs, void *address) {
        PyObject *item = NULL;

        if (!list_longs || !PyList_Check(list_longs)) { /* Note: PyList_Check allows sub-types. */
            PyErr_Format(PyExc_TypeError, "check_list_of_longs(): First argument is not a list");
            return 0;
        }
        long result = 0L;
        for (Py_ssize_t i = 0; i < PyList_GET_SIZE(list_longs); ++i) {
            item = PyList_GetItem(list_longs, i);
            if (!PyLong_CheckExact(item)) {
                PyErr_Format(PyExc_TypeError, "check_list_of_longs(): Item %d is not a Python integer.", i);
                return 0;
            }
            /* PyLong_AsLong() must always succeed because of check above. */
            result += PyLong_AsLong(item);
        }
        long *p_long = (long *) address;
        *p_long = result;
        return 1; /* Success. */
    }

Now we can pass this function to ``PyArg_ParseTuple`` with the ``"O&"`` formatting string that takes two arguments,
the Python list and the C conversion function. On success ``PyArg_ParseTuple`` writes the value to the target,
``result``.

In this case the function just returns the sum of the integers in the list.
Here is the C code.

.. code-block:: c

    static PyObject *
    parse_args_with_function_conversion_to_c(PyObject *Py_UNUSED(module), PyObject *args) {
        PyObject *ret = NULL;
        long result;

        if (!PyArg_ParseTuple(args, "O&", sum_list_of_longs, &result)) {
            /* NOTE: If check_list_of_numbers() returns 0 an error should be set. */
            assert(PyErr_Occurred());
            goto except;
        }

        /* Your code here...*/
        ret = PyLong_FromLong(result);
        if (ret == NULL) {
            goto except;
        }
        assert(!PyErr_Occurred());
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        return ret;
    }

Parsing File Paths as Arguments
----------------------------------------

This technique is very useful, and common, when converting Python file paths (a ``str`` or path-like object)
to C file paths (``char *``).
For this the Python C API provides some useful functions
`PyUnicode_FSConverter <https://docs.python.org/3/c-api/unicode.html#c.PyUnicode_FSConverter>`_
and
`PyUnicode_DecodeFSDefaultAndSize <https://docs.python.org/3/c-api/unicode.html#c.PyUnicode_DecodeFSDefaultAndSize>`_

Here is an example of taking a Python Unicode string representing a file path, converting it to C and then back
to Python. The stages are:

- Use ``PyArg_ParseTupleAndKeywords`` and ``PyUnicode_FSConverter`` to convert the path-like Python object to
  a Python ``bytes`` object.
- Extract the raws bytes to use as a C path.

.. code-block:: c

    static PyObject *
    parse_filesystem_argument(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwargs) {
        assert(!PyErr_Occurred());
        assert(args || kwargs);

        PyBytesObject *py_path = NULL;
        char *c_path = NULL;
        Py_ssize_t path_size;
        PyObject *ret = NULL;

        /* Parse arguments */
        static char *kwlist[] = {"path", NULL};
        /* Can be optional output path with "|O&". */
        if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&", kwlist, PyUnicode_FSConverter, &py_path)) {
            goto except;
        }
        /* Check arguments. */
        assert(py_path);
        /* Grab a reference to the internal bytes buffer. */
        if (PyBytes_AsStringAndSize((PyObject *) py_path, &c_path, &path_size)) {
            /* Should have a TypeError or ValueError. */
            assert(PyErr_Occurred());
            assert(PyErr_ExceptionMatches(PyExc_TypeError) || PyErr_ExceptionMatches(PyExc_ValueError));
            goto except;
        }
        assert(c_path);

        /* Use the C path. */

        /* Now convert the C path to a Python object, a string. */
        ret = PyUnicode_DecodeFSDefaultAndSize(c_path, path_size);
        if (!ret) {
            goto except;
        }
        assert(!PyErr_Occurred());
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        // Assert all temporary locals are NULL and thus have been transferred if used.
        Py_XDECREF(py_path);
        return ret;
    }
