.. highlight:: python
    :linenothreshold: 20

.. toctree::
    :maxdepth: 3

.. Links to the Python documentation

.. _PyArg_ParseTuple(): https://docs.python.org/3/c-api/arg.html#c.PyArg_ParseTuple
.. _PyArg_ParseTupleAndKeywords(): https://docs.python.org/3/c-api/arg.html#c.PyArg_ParseTupleAndKeywords

.. index::
    single: Parsing Arguments

***************************
Parsing Python Arguments
***************************

This section describes how you write functions that accept Python ``*args`` and ``**kwargs``
arguments and how to extract ``PyObject`` or C fundamental types from them.


====================================
Specifying the Function Arguments
====================================

Python has a myriad of ways of function arguments and the C API reflects that.
Two important features of CPython C functions are:

- Declaring them correctly with the right signature and flags.
- Parsing the arguments and checking their types.

These are described below.

-------------------------------
C Function Declaration
-------------------------------

The C function signature must be declared correctly depending on the arguments it is expected to work with.
Here is a small summary of the required declaration.

In all cases the function has a *context* which is the first argument.

- For free functions the first argument is the module within which the function is declared.
  This allows you to access other attributes or functions in the module.
- For member functions (methods) the first argument is the object within which the function is declared ( ``self`` ).
  This allows you to access other properties or functions in the object.

If the argument is unused then the `Py_UNUSED <https://docs.python.org/3/c-api/intro.html#c.Py_UNUSED>`_ can be used to
supress a compiler warning or error thus:

.. code-block:: c

    static PyObject *
    parse_args_kwargs(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwargs);

.. index::
    single: Parsing Arguments; ml_flags

Setting the ``ml_flags`` Field
------------------------------

The `ml_flags <https://docs.python.org/3.13/c-api/structures.html#c.PyMethodDef.ml_flags>`_ field in
`PyMethodDef <https://docs.python.org/3.13/c-api/structures.html#c.PyMethodDef>`_ specifies the form of the arguments.

.. index::
    single: Parsing Arguments; No Arguments
    single: Parsing Arguments; METH_NOARGS

No Arguments
^^^^^^^^^^^^^^^^^^

- The flags will be `METH_NOARGS <https://docs.python.org/3/c-api/structures.html#c.METH_NOARGS>`_
- The C Function Signature will be ``PyObject *PyCFunction(PyObject *self, PyObject *args);``
- The second argument will be ``NULL``.

.. index::
    single: Parsing Arguments; One Argument
    single: Parsing Arguments; METH_O

One Argument
^^^^^^^^^^^^^^^^^^

- The flags will be `METH_O <https://docs.python.org/3/c-api/structures.html#c.METH_O>`_
- The C Function Signature will be ``PyObject *PyCFunction(PyObject *self, PyObject *args);``
- The second argument will be the single argument.

.. index::
    single: Parsing Arguments; Multiple Arguments
    single: Parsing Arguments; METH_VARARGS

Multiple Positional Arguments
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- The flags will be `METH_VARARGS <https://docs.python.org/3/c-api/structures.html#c.METH_VARARGS>`_
- The C Function Signature will be ``PyObject *PyCFunction(PyObject *self, PyObject *args);``
- Second value will be a tuple of arguments.
- `PyArg_ParseTuple()`_ is used to unpack the arguments.

.. index::
    single: Parsing Arguments; Positional and Keyword Arguments
    single: Parsing Arguments; METH_KEYWORDS

Positional and Keyword Arguments
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- The flags will be `METH_NOARGS | METH_KEYWORDS <https://docs.python.org/3/c-api/structures.html#c.METH_KEYWORDS>`_
- The C Function Signature will be ``PyObject *PyCFunctionWithKeywords(PyObject *self, PyObject *args, PyObject *kwargs);``
- Second value will be a sequence of arguments, the third the dictionary of arguments.
- `PyArg_ParseTupleAndKeywords()`_ is used to unpack the arguments.

Documentation:

- No arguments, single or multiple arguments:
  `PyCFunction <https://docs.python.org/3/c-api/structures.html#c.PyCFunction>`_
- Multiple argument and keywords `PyCFunctionWithKeywords <https://docs.python.org/3/c-api/structures.html#c.PyCFunctionWithKeywords>`_

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

Once we have the C function correctly declared then the arguments have to parsed according to their types and,
if required, converted to C types (so-called "unboxing").
This is done using the `PyArg_ParseTuple()`_ and `PyArg_ParseTupleAndKeywords()`_
(ignoring “old-style” functions which use `PyArg_Parse <https://docs.python.org/3/c-api/arg.html#c.PyArg_Parse>`_).

These use formatting strings that can become bewilderingly complex so this tutorial uses examples as an introduction.
The reference documentation is excellent: `argument parsing and building values <https://docs.python.org/3/c-api/arg.html>`_.

.. warning::

    Error messages can be slightly misleading as the argument index is the *C* index
    not the *Python* index.

    For example a C function using ``METH_VARARGS`` declared as:

    ``static PyObject *parse_args(PyObject *module, PyObject *args);``

    Which expects the Python args[0] to be a bytes object and the Python args[1]
    to be an integer by using:

    ``PyArg_ParseTuple(args, "Si", &arg0, &arg1)``

    Calling this from Python with ``parse_args(21, 22)`` will give:

    ``TypeError: argument 1 must be bytes, not int``.

    The "1" here refers to the C index not the Python index.
    The correct call would be to change the 0th Python argument to:

    ``cParseArgs.parse_args(b'21', 22)``

    Also consider the signature:

    ``def parse_args(a: bytes, b: int, c: str = '') -> int:``

    Called with ``parse_args(b'bytes', '456')`` gives the error:

    ``"'str' object cannot be interpreted as an integer"``

    without specifying which argument it is referring to.


Examples
====================================

These examples are in ``src/cpy/cParseArgs.c`` and their tests are in ``tests/unit/test_c_parse_args.py``.

.. index::
    single: Parsing Arguments Example; No Arguments
    single: Parsing Arguments; METH_NOARGS

No Arguments
------------------------------------

The simplest form is a global function in a module that takes no arguments at all:

.. code-block:: c

    static PyObject *parse_no_args(PyObject *Py_UNUSED(module)) {

        /* Your code here...*/

        Py_RETURN_NONE;
    }

This function is added to the module methods with the ``METH_NOARGS`` value.
The Python interpreter will raise a ``TypeError`` on any arguments are offered to the function.

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

.. index::
    single: Parsing Arguments Example; One Argument
    single: Parsing Arguments; METH_O

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
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

.. index::
    single: Parsing Arguments Example; Variable Number of Arguments
    single: Parsing Arguments; METH_VARARGS

Variable Number of Arguments
----------------------------------------------------

The function will be called with two arguments, the module and a ``PyTupleObject`` that contains a tuple of arguments.
You can either parse this list yourself or use a helper method to parse it into Python and C types.

In the following code we are expecting a bytes object, an integer and an optional string whose default value is
'default_string'.
For demonstration purposes, this returns the same three arguments.
In Python the equivalent function signature would be::

    def parse_args(a: bytes, b: int, c: str = 'default_string') \
        -> typing.Tuple[bytes, int, str]:

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

.. index::
    single: Parsing Arguments Example; Variable Number of Arguments
    single: Parsing Arguments Example; Keyword Arguments
    single: Parsing Arguments; METH_KEYWORDS

Variable Number of Arguments and Keyword Arguments
--------------------------------------------------------------------------

The function will be called with three arguments, the module, a ``PyTupleObject`` that contains a tuple of arguments
and a ``PyDictObject`` that contains a dictionary of keyword arguments.
The keyword arguments can be NULL if there are no keyword arguments.
You can either parse these yourself or use a helper method to parse it into Python and C types.

In the following code we are expecting a sequence and an optional integer count defaulting to 1.
It returns the `sequence` repeated `count` times.
In Python the equivalent function declaration would be::

    def parse_args_kwargs(sequence=typing.Sequence[typing.Any], count: = 1) \
        -> typing.Sequence[typing.Any]:
        return sequence * count

Here is the C code, note the string ``"O|i"`` that describes the argument types passed to
``PyArg_ParseTupleAndKeywords``, if these types are not present a ``TypeError`` will be set.

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

        if (!PyArg_ParseTupleAndKeywords(
                args, kwargs, "O|i", kwlist, &py_sequence, &count
            )) {
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

.. index::
    single: Parsing Arguments Example; Keyword Arguments and C++11

Keyword Arguments and C++11
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

C++11 compilers warn when creating non-const ``char*`` from string literals as we have done with the keyword array
above.
The solution is to declare these ``const char*`` however `PyArg_ParseTupleAndKeywords()`_ expects a ``char **``.
The solution is to cast away const in the call:

.. code-block:: c

    /* ... */
    static const char *kwlist[] = { "foo", "bar", "baz", NULL };
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "OOO",
                                      const_cast<char**>(kwlist),
                                      &foo, &bar, &baz)) {
        return NULL;
    }
    /* ... */


.. index::
    single: Parsing Arguments Example; Default String Arguments
    single: Parsing Arguments Example; Default Bytes Arguments
    single: Default Arguments; C
    single: Py_buffer

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
``Py_Buffer`` is uninitialized.
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

The complete C code is:

.. code-block:: c

    static PyObject *
    parse_default_bytes_object(PyObject *Py_UNUSED(module), PyObject *args,
                               PyObject *kwargs) {
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

See ``tests/unit/test_c_parse_args.py`` for some Python uses of this code.

.. index::
    single: Positional Only Arguments
    single: Keyword Only Arguments
    single: Parsing Arguments Example; Positional Only Arguments
    single: Parsing Arguments Example; Keyword Only Arguments

Positional Only and Keyword Only Arguments
-----------------------------------------------

This section shows how to achieve
`positional only <https://docs.python.org/3/glossary.html#positional-only-parameter>`_
and `keyword only <https://docs.python.org/3/glossary.html#keyword-only-parameter>`_ arguments in a C extension.
These are described in the Python documentation for
`Special parameters <https://docs.python.org/3/tutorial/controlflow.html#special-parameters>`_
Specifically `positional only parameters <https://docs.python.org/3/tutorial/controlflow.html#positional-only-parameters>`_
and `keyword only arguments <https://docs.python.org/3/tutorial/controlflow.html#keyword-only-arguments>`_.

Suppose we want the functional equivalent of the Python function signature
(reproducing https://docs.python.org/3/tutorial/controlflow.html#special-parameters ):

.. code-block:: python

    def parse_pos_only_kwd_only(pos1: str, pos2: int, /, pos_or_kwd: bytes, *,
                                kwd1: float = 256.0, kwd2: int = -421):
        return None

This is achieved by combining two techniques:

- Positional only: The strings in the ``*kwlist`` passed to `PyArg_ParseTupleAndKeywords()`_ are empty.
- Keyword only: The formatting string passed to `PyArg_ParseTupleAndKeywords()`_ uses the ``'$'`` character.

A function using either positional only or keyword only arguments must use the flags ``METH_VARARGS | METH_KEYWORDS``
and uses `PyArg_ParseTupleAndKeywords()`_. Currently, all keyword-only arguments must also be optional arguments, so ``'|'`` must always be
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
                        /* NOTE: As '$' is in format string the next to are keyword only. */
                "kwd1",             /* kwd1 is keyword only argument. */
                "kwd2",             /* kwd2 is keyword only argument. */
                NULL,
        };

        if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s*iy*|$di", kwlist, &pos1, &pos2,
                                         &pos_or_kwd, &kwd1, &kwd2)) {
            assert(PyErr_Occurred());
            return NULL;
        }
        /* Return the parsed arguments.
         * NOTE the asymmetry between "s*iy*|$di" above and "s#iy#di" here. */
        return Py_BuildValue("s#iy#di", pos1.buf, pos1.len, pos2, pos_or_kwd.buf,
                             pos_or_kwd.len, kwd1, kwd2);
    }


.. index::
    single: Parsing Arguments Example; Functional Conversion to C

Parsing Arguments With Functional Conversion to C
---------------------------------------------------------

Often you want to convert a Python argument to a C value(s) in a way that is not covered by the format strings
provided by the Python C API. To do this you can provide a special conversion function in C and give it to
`PyArg_ParseTuple()`_ or `PyArg_ParseTupleAndKeywords()`_.

In this example we are expecting the Python argument to be a list of integers and we want the sum of them as
a C ``long``. First create a C function that takes a Python list, checks it and sums the values.
The function returns 1 on success or 0 on error and, in that case, and exception is expected to be set.
On success the result will be written into the opaque pointer, here called ``address``:

.. code-block:: c

    int sum_list_of_longs(PyObject *list_longs, void *address) {
        PyObject *item = NULL;

        /* Note: PyList_Check allows sub-types. */
        if (! list_longs || ! PyList_Check(list_longs)) {
            PyErr_Format(PyExc_TypeError,
                         "check_list_of_longs(): First argument is not a list"
            );
            return 0;
        }
        long result = 0L;
        for (Py_ssize_t i = 0; i < PyList_GET_SIZE(list_longs); ++i) {
            item = PyList_GetItem(list_longs, i);
            if (!PyLong_CheckExact(item)) {
                PyErr_Format(PyExc_TypeError,
                             "check_list_of_longs(): Item %d is not a Python integer.", i
                );
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

.. index::
    single: Parsing Arguments Example; Default Mutable Arguments
    single: Default Arguments, Mutable; C

.. _cpython_default_mutable_arguments:

Being Pythonic with Default Mutable Arguments
=============================================

If the arguments default to some C fundamental type the code above is fine.
However if the mutable arguments default to Python objects then a little more work is needed.

.. note::

    See :ref:`cpp_and_cpython.handling_default_arguments` for a way of simplifying this with C++.

Here is a function that has an object and as default argument as a list, appends the object to the list and
returns the list.
The Python signature is:

.. code-block:: python

    def function(arg_0, arg_1=[]):
        arg_1.append(arg_0)
        return arg_1

The first argument is immutable, the second is mutable.
We need to mimic the well known behaviour of Python with mutable arguments where default arguments are evaluated once
only at function definition time and then becomes a (mutable) property of the function.

For example:

.. code-block:: python

    >>> function(1)
    [1]
    >>> function(2)
    [1, 2]
    >>> my_list = []
    >>> f(10, my_list)
    [10]
    >>> f(10)
    [1, 2, 10]

In C we can get this behaviour by treating the mutable argument as ``static``, the immutable argument does not need to
be ``static`` but it will do no harm if it is (if non-``static`` it will have to be initialised on every function call).

My advice: Always make all ``PyObject*`` references to default arguments ``static``.

So first we declare a ``static PyObject*`` for each default argument:

.. code-block:: c

    /** Parse the args where we are simulating mutable default of an empty list.
     *
     * This is equivalent to:
     *
     *  def parse_args_with_mutable_defaults_macro_helper(obj, default_list=[]):
     *      default_list.append(obj)
     *      return default_list
     *
     * See also parse_args_with_mutable_defaults_macro_helper() in cParseArgsHelper.cpp
     * This adds the object to the list and returns None.
     *
     * This imitates the Python way of handling defaults.
     */
    static PyObject *parse_args_with_mutable_defaults(PyObject *Py_UNUSED(module),
                                                      PyObject *args) {
        PyObject *ret = NULL;
        /* Pointers to the non-default argument, initialised by PyArg_ParseTuple below. */
        PyObject *arg_0 = NULL;
        /* Pointers to the default argument, initialised below. */
        static PyObject *arg_1_default = NULL;
        /* Set defaults for argument 1. */
        if (!arg_1_default) {
            arg_1_default = PyList_New(0);
        }

Then we declare a ``PyObject*`` for each argument that will either reference the default or the passed in argument.
It is important that these ``pyObjArg_...`` pointers are NULL so that we can subsequently detect if
``PyArg_ParseTuple`` has set them non-``NULL``.

.. code-block:: c

    /* This pointer is the one we use in the body of the function, it
     * either points at the supplied argument or the default (static) argument.
     */
    PyObject *arg_1 = NULL;


Now parse the given arguments to see what, if anything, is there.
``PyArg_ParseTuple`` will set each working pointer non-``NULL`` if the argument is present.
As we set the working pointers ``NULL`` prior to this call we can now tell if any argument is present.

.. code-block:: c

    if (!PyArg_ParseTuple(args, "O|O", &arg_0, &arg_1)) {
        goto except;
    }


Now switch our working pointers to the default argument if no argument is given.
We also treat these as "borrowed" references regardless of whether they are default or supplied so increment the
refcount (we must decrement the refcount when done).

.. code-block:: c

    /* If optional argument absent then switch to defaults. */
    if (!arg_1) {
        arg_1 = arg_1_default;
    }

Now write the main body of your function and that must be followed by this clean up code:

.. code-block:: c

    /* Your code here...*/

    /* Append the first argument to the second.
     * PyList_Append increments the reference count of arg_0. */
    if (PyList_Append(arg_1, arg_0)) {
        PyErr_SetString(PyExc_RuntimeError, "Can not append to list!");
        goto except;
    }
    /* Success. */
    assert(!PyErr_Occurred());
    /* Increments the default or the given argument. */
    Py_INCREF(arg_1);
    ret = arg_1;
    goto finally;

Now the two blocks ``except`` and ``finally``.

.. code-block:: c

    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        return ret;
    }

Here is the complete C code:

.. code-block:: c
    :linenos:

    /** Parse the args where we are simulating mutable default of an empty list.
     *
     * This is equivalent to:
     *
     *  def parse_args_with_mutable_defaults_macro_helper(obj, default_list=[]):
     *      default_list.append(obj)
     *      return default_list
     *
     * See also parse_args_with_mutable_defaults_macro_helper() in cParseArgsHelper.cpp
     * This adds the object to the list and returns None.
     *
     * This imitates the Python way of handling defaults.
     */
    static PyObject *parse_args_with_mutable_defaults(PyObject *Py_UNUSED(module),
                                                      PyObject *args) {
        PyObject *ret = NULL;
        /* Pointers to the non-default argument, initialised by PyArg_ParseTuple below. */
        PyObject *arg_0 = NULL;
        /* Pointers to the default argument, initialised below. */
        static PyObject *arg_1_default = NULL;
        /* Set defaults for argument 1. */
        if (!arg_1_default) {
            arg_1_default = PyList_New(0);
        }
        /* This pointer is the one we use in the body of the function, it
         * either points at the supplied argument or the default (static) argument.
         */
        PyObject *arg_1 = NULL;

        if (!PyArg_ParseTuple(args, "O|O", &arg_0, &arg_1)) {
            goto except;
        }
        /* If optional argument absent then switch to defaults. */
        if (!arg_1) {
            arg_1 = arg_1_default;
        }

        /* Your code here...*/

        /* Append the first argument to the second.
         * PyList_Append increments the reference count of arg_0. */
        if (PyList_Append(arg_1, arg_0)) {
            PyErr_SetString(PyExc_RuntimeError, "Can not append to list!");
            goto except;
        }

        /* Success. */
        assert(!PyErr_Occurred());
        /* Increments the default or the given argument. */
        Py_INCREF(arg_1);
        ret = arg_1;
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        return ret;
    }

The code can be found in ``src/cpy/ParseArgs/cParseArgs.c``.

Tests are in ``test_parse_args_with_mutable_defaults()`` in ``tests/unit/test_c_parse_args.py``

.. index::
    single: Parsing Arguments Example; Helper Macros

Helper Macros
=============

Some macros can make this easier.
These are in ``src/cpy/ParseArgs/cParseArgsHelper.cpp``.
Firstly a macro to declare the static default object:

.. code-block:: c

    #define PY_DEFAULT_ARGUMENT_INIT(name, value, ret)          \
        PyObject *name = NULL;                                  \
        static PyObject *default_##name = NULL;                 \
        if (! default_##name) {                                 \
            default_##name = value;                             \
            if (! default_##name) {                             \
                PyErr_SetString(                                \
                    PyExc_RuntimeError,                         \
                    "Can not create default value for " #name   \
                );                                              \
                return ret;                                     \
            }                                                   \
        }

.. warning::

    When  using this macro in a source file then make sure each given "name" argument is unique within the
    translation unit.

And a macro to set it:

.. code-block:: c

    #define PY_DEFAULT_ARGUMENT_SET(name) \
        if (! name) {                     \
            name = default_##name;        \
        }

And a macro to check the type of the argument:

.. code-block:: c

    #define PY_DEFAULT_CHECK(name, check_function, type)    \
        if (!check_function(name)) {                        \
            PyErr_Format(                                   \
                PyExc_TypeError,                            \
                #name " must be " #type ", not \"%s\"",     \
                Py_TYPE(name)->tp_name                      \
            );                                              \
            return NULL;                                    \
        }

.. index::
    single: Default Arguments, Immutable; C

Immutable Arguments
-------------------

These can be used thus.
This is equivalent to the Python function:

.. code-block:: python

    def parse_defaults_with_helper_macro(
        encoding_m: str = "utf-8",
        the_id_m: int = 1024,
        log_interval_m: float = 8.0):
        return encoding_m, the_id_m, log_interval_m

Here it is in C:

.. code-block:: c

    static PyObject *
    parse_defaults_with_helper_macro(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
        PyObject *ret = NULL;
        /* Initialise default arguments. Note: these might cause an early return. */
        PY_DEFAULT_ARGUMENT_INIT(encoding_m, PyUnicode_FromString("utf-8"), NULL);
        PY_DEFAULT_ARGUMENT_INIT(the_id_m, PyLong_FromLong(DEFAULT_ID), NULL);
        PY_DEFAULT_ARGUMENT_INIT(log_interval_m, PyFloat_FromDouble(DEFAULT_FLOAT), NULL);

        static const char *kwlist[] = {"encoding", "the_id", "log_interval", NULL};
        if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOO",
                                         const_cast<char **>(kwlist),
                                         &encoding_m, &the_id_m, &log_interval_m)) {
            goto except;
        }
        /*
         * Assign absent arguments to defaults and increment the reference count.
         * Don't forget to decrement the reference count before returning!
         */
        PY_DEFAULT_ARGUMENT_SET(encoding_m);
        PY_DEFAULT_ARGUMENT_SET(the_id_m);
        PY_DEFAULT_ARGUMENT_SET(log_interval_m);

        /* Check the types of the given or default arguments. */
        PY_DEFAULT_CHECK(encoding_m, PyUnicode_Check, "str");
        PY_DEFAULT_CHECK(the_id_m, PyLong_Check, "int");
        PY_DEFAULT_CHECK(log_interval_m, PyFloat_Check, "float");

        /*
         * Use 'encoding': Python str, 'the_id': C long, 'must_log': C long from here on...
         */

        /* Py_BuildValue("O") increments the reference count. */
        ret = Py_BuildValue("OOO", encoding_m, the_id_m, log_interval_m);
        assert(!PyErr_Occurred());
        assert(ret);
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        return ret;
    }

The code is:

- Function: ``parse_defaults_with_helper_macro()``
- Location: ``src/cpy/ParseArgs/cParseArgsHelper.cpp``

The tests are in ``tests/unit/test_c_parse_args_helper.py``.

If you are in a C++ environment then the section on :ref:`cpp_and_cpython.handling_default_arguments` can help.

.. index::
    single: Default Arguments, Mutable; C

Mutable Arguments
-------------------

These macros also work with mutable arguments.

The following C code emulates this Python code:

.. code-block:: python

    def parse_mutable_defaults_with_helper_macro(obj, default_list=[]):
        default_list.append(obj)
        return default_list

Here is the C code:

.. code-block:: c

    /** Parse the args where we are simulating mutable default of an empty list.
     * This uses the helper macros.
     *
     * This is equivalent to:
     *
     *  def parse_mutable_defaults_with_helper_macro(obj, default_list=[]):
     *      default_list.append(obj)
     *      return default_list
     *
     * This adds the object to the list and returns None.
     *
     * This imitates the Python way of handling defaults.
     */
    static PyObject *parse_mutable_defaults_with_helper_macro(PyObject *Py_UNUSED(module),
                                                              PyObject *args) {
        PyObject *ret = NULL;
        /* Pointers to the non-default argument, initialised by PyArg_ParseTuple below. */
        PyObject *arg_0 = NULL;
        /* Pointers to the default argument, initialised below. */
        /* Initialise default arguments. Note: these might cause an early return. */
        PY_DEFAULT_ARGUMENT_INIT(list_argument_m, PyList_New(0), NULL);

        if (!PyArg_ParseTuple(args, "O|O", &arg_0, &list_argument_m)) {
            goto except;
        }
        /* If optional argument absent then switch to defaults. */
        PY_DEFAULT_ARGUMENT_SET(list_argument_m);
        PY_DEFAULT_CHECK(list_argument_m, PyList_Check, "list");

        /* Your code here...*/

        /* Append the first argument to the second.
         * PyList_Append() increments the refcount of arg_0. */
        if (PyList_Append(list_argument_m, arg_0)) {
            PyErr_SetString(PyExc_RuntimeError, "Can not append to list!");
            goto except;
        }

        /* Success. */
        assert(!PyErr_Occurred());
        /* This increments the default or the given argument. */
        Py_INCREF(list_argument_m);
        ret = list_argument_m;
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        return ret;
    }

Here is some test code from ``tests/unit/test_c_parse_args_helper.py``.

First a test to establish the Python behaviour:

.. code-block:: python

    def test_parse_mutable_defaults_with_helper_macro_python():
        """A local Python equivalent of cParseArgsHelper.parse_mutable_defaults_with_helper_macro()."""

        def parse_mutable_defaults_with_helper_macro(obj, default_list=[]):
            default_list.append(obj)
            return default_list

        result = parse_mutable_defaults_with_helper_macro(1)
        assert sys.getrefcount(result) == 3
        assert result == [1, ]
        result = parse_mutable_defaults_with_helper_macro(2)
        assert sys.getrefcount(result) == 3
        assert result == [1, 2]
        result = parse_mutable_defaults_with_helper_macro(3)
        assert sys.getrefcount(result) == 3
        assert result == [1, 2, 3]

        local_list = []
        assert sys.getrefcount(local_list) == 2
        assert parse_mutable_defaults_with_helper_macro(10, local_list) == [10]
        assert sys.getrefcount(local_list) == 2
        assert parse_mutable_defaults_with_helper_macro(11, local_list) == [10, 11]
        assert sys.getrefcount(local_list) == 2

        result = parse_mutable_defaults_with_helper_macro(4)
        assert result == [1, 2, 3, 4]
        assert sys.getrefcount(result) == 3

Now a similar test to establish the C behaviour:

.. code-block:: python

    from cPyExtPatt import cParseArgsHelper

    def test_parse_mutable_defaults_with_helper_macro_c():
        result = cParseArgsHelper.parse_mutable_defaults_with_helper_macro(1)
        assert sys.getrefcount(result) == 3
        assert result == [1, ]
        result = cParseArgsHelper.parse_mutable_defaults_with_helper_macro(2)
        assert sys.getrefcount(result) == 3
        assert result == [1, 2]
        result = cParseArgsHelper.parse_mutable_defaults_with_helper_macro(3)
        assert sys.getrefcount(result) == 3
        assert result == [1, 2, 3]

        local_list = []
        assert sys.getrefcount(local_list) == 2
        assert cParseArgsHelper.parse_mutable_defaults_with_helper_macro(10, local_list) == [10]
        assert sys.getrefcount(local_list) == 2
        assert cParseArgsHelper.parse_mutable_defaults_with_helper_macro(11, local_list) == [10, 11]
        assert sys.getrefcount(local_list) == 2

        result = cParseArgsHelper.parse_mutable_defaults_with_helper_macro(4)
        assert result == [1, 2, 3, 4]
        assert sys.getrefcount(result) == 3
