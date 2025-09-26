.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

..
    Links, mostly to the Python documentation.

.. _PyModuleDef: https://docs.python.org/3/c-api/module.html#c.PyModuleDef
.. _PyMethodDef: https://docs.python.org/3/c-api/structures.html#c.PyMethodDef
.. _PyMODINIT_FUNC: https://docs.python.org/3/c-api/intro.html#c.PyMODINIT_FUNC

=================
A Simple Example
=================

This somewhat artificial example illustrates some of the benefits and drawbacks of Python C Extensions.

Suppose you have some Python code such as this that is performing slowly:

.. code-block:: python

    def fibonacci(index: int) -> int:
        if index < 2:
            return index
        return fibonacci(index - 2) + fibonacci(index - 1)

And that code is in ``pFibA.py``.
In the repl we can measure its performance with ``timeit``:

.. code-block:: bash

    >>> import pFibA
    >>> pFibA.fibonacci(30)
    832040
    >>> import timeit
    >>> ti_py = timeit.timeit(f'pFibA.fibonacci(30)', setup='import pFibA', number=10)
    >>> print(f'Python timeit: {ti_py:8.6f}')
    Python timeit: 1.459842
    >>>

That is pretty slow.

-----------------------
Faster Please
-----------------------

Now we want something faster so we turn to creating a C extension.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The C Equivalent Function
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Firstly we can write the C equivalent to ``fibonacci()`` in the file ``cFibA.c``:

.. code-block:: c

    long fibonacci(long index) {
        if (index < 2) {
            return index;
        }
        return fibonacci(index - 2) + fibonacci(index - 1);
    }

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The Python Interface to C
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

So far we have a pure C function, we now write a C function that:

- Takes Python objects as arguments, converts them to C objects with ``PyArg_ParseTuple()`` (so-called 'un-boxing').
- Calls our C function ``fibonacci()``
- Then converts the C result to a Python object with ``Py_BuildValue()`` (so-called 'boxing').

Note the inclusion of ``"Python.h"`` which will give us access to the whole Python C API.

.. code-block:: c

    #define PY_SSIZE_T_CLEAN
    #include "Python.h"

    static PyObject *
    py_fibonacci(PyObject *Py_UNUSED(module), PyObject *args) {
        long index;

        if (!PyArg_ParseTuple(args, "l", &index)) {
            return NULL;
        }
        long result = fibonacci(index);
        return Py_BuildValue("l", result);
    }

.. index::
    single: PyModuleDef
    single: PyMethodDef
    single: PyMODINIT_FUNC

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The Python Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Then we need to write some C code that defines the Python module that contains this function.
The first is a `PyMethodDef`_ structure to define the Python functions in the module, the comments explain each field.:

.. code-block:: c

    static PyMethodDef module_methods[] = {
        {
            /* The name of the function as seen by Python */
            "fibonacci",
            /* The function in the C code. */
            (PyCFunction) py_fibonacci,
            /* Flag that specifies that the function takes a variable
             * number of arguments, although only one is parsed. */
            METH_VARARGS,
            /* The documentation string. */
            "Returns the Fibonacci value."
        },
        {NULL, NULL, 0, NULL} /* Sentinel */
    };

Then we create a `PyModuleDef`_ structure that defines the module itself, its name and so on.
Note that this references the ``module_methods`` structure above:

.. code-block:: c

    static PyModuleDef cFibA = {
        /* The module structure. */
        PyModuleDef_HEAD_INIT,
        /* The name of the modules as seen by Python. */
        .m_name = "cFibA",
        /* The module documentation. */
        .m_doc = "Fibonacci in C.",
        /* The module state. -1 means it has global state. */
        .m_size = -1,
        /* The module method table (above). */
        .m_methods = module_methods,
    };

Lastly a function to to initialise the module, this uses the `PyMODINIT_FUNC`_ macro.
This is the only non-static function in the module.
Note that the name must match; when you go ``import cFibA`` Python will want to call a C function ``PyInit_cFibA()``:

.. code-block:: c

    PyMODINIT_FUNC PyInit_cFibA(void) {
        /* Create the module according to the module definition above. */
        PyObject *m = PyModule_Create(&cFibA);
        return m;
    }

.. index::
    single: setup.py

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The ``setup.py`` File
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Finally we add a ``setup.py`` that specifies how to compile this code:

.. code-block::

    from setuptools import setup, Extension

    setup(
        name='cPyExtPatt',
        version='0.1.0',
        author='AUTHOR',
        description='Python Extension example.',
        ext_modules=[
            Extension(
                "cPyExtPatt.SimpleExample.cFibA",
                sources=['src/cpy/SimpleExample/cFibA.c', ],
                include_dirs=[],
                library_dirs=[],
                libraries=[],
                extra_compile_args=[
                    '-Wall', '-Wextra', '-Werror', '-Wfatal-errors', '-Wpedantic',
                    '-Wno-unused-function', '-Wno-unused-parameter',
                    '-Qunused-arguments', '-std=c99',
                    '-UDEBUG', '-DNDEBUG', '-Ofast', '-g',
                ],
            )
        ]
    )

Running ``python setup.py develop`` will compile and build the module which can be used thus:


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Trying it Out
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: python

    >>> from cPyExtPatt.SimpleExample import cFibA
    >>> cFibA.fibonacci(8)
    21

Great, it works!

----------------------------
How Did We Do?
----------------------------

There is a test file that uses ``timeit`` to check the performance of the Python and C code at
``src/cpy/SimpleExample/timeit_test.py``.


.. code-block:: bash

    (PythonExtPatt3.11_A) SimpleExample git:(develop) $ python timeit_test.py
    Index: 32 number of times: 20
    Version A, no cacheing:
    Python timeit: 7.321638
         C timeit: 0.131115
    C is 55.8 times FASTER.

So with a small bit of work we have got a performance improvement of 55x.

----------------------------
It's Not Over Yet
----------------------------

Suppose we change the Python code by adding a couple of lines that uses a local cache for the results.
We put this in the file ``pFibB.py``:

.. code-block:: python

    import functools # Added

    @functools.cache # Added
    def fibonacci(index: int) -> int:
        if index < 2:
            return index
        return fibonacci(index - 2) + fibonacci(index - 1)

Now what does our timing code say?

.. code-block:: bash

    Version A with Python cache, no C cache:
    Python timeit: 0.000012
         C timeit: 0.130394
    C is 11058.7 times SLOWER.

So our Python code is now vastly faster than our C code.

This emphasises that performance can also come from a good choice of libraries, data structures, algorithms,
cacheing and other techniques as well as the choice of the language of the implementation.

-------------------------------
C Fights Back
-------------------------------

Well whatever we can do in Python we can do in C.
So what if we write ``cFibB.c`` to change the ``fibonacci()`` function to have a cache as well?

.. code-block:: c

    long fibonacci(long index) {
        static long *cache = NULL;
        if (!cache) {
            /* FIXME */
            cache = calloc(1000, sizeof(long));
        }
        if (index < 2) {
            return index;
        }
        if (!cache[index]) {
            cache[index] = fibonacci(index - 2) + fibonacci(index - 1);
        }
        return cache[index];
    }

Now what does our timeing code say?

.. code-block:: bash

    Version B, both are cached:
    Python timeit: 0.000004
         C timeit: 0.000007
    C is 1.9 times SLOWER.

So our C code is back in the game but still slower.
What is more the C code has added significant complexity to our codebase.
And this codebase has to be maintained, and at what cost?
The C code has also added significant risk as well as identified by the ``/* FIXME */`` comment above.

So while the options are available there are tradeoffs to be made.

--------------------------
Summary
--------------------------

- C Extensions can give vastly improved performance.
- A good choice of Python libraries, algorithms, code architecture and design can improve performance less
  expensively than going to C.
- This exposes the possible tradeoffs between the techniques.
- It is very useful in software engineering to have these tradeoffs that are explicit and visible.

Next up, understanding reference counts and Python's terminology.
