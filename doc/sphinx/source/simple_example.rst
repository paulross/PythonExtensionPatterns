.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

=================
A Simple Example
=================

This very artificial example illustrates some of the benefits and drawbacks of Python C Extensions.

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

Now we want something faster so we turn to creating a C extension.
Firstly we can write the C equivalent to ``fibonacci()`` in the file ``cFibA.c``, note the inclusion of ``"Python.h"``
which will give us access to the whole Python C API:

.. code-block:: c

    #define PPY_SSIZE_T_CLEAN
    #include "Python.h"

    long fibonacci(long index) {
        if (index < 2) {
            return index;
        }
        return fibonacci(index - 2) + fibonacci(index - 1);
    }

This is a pure C function, we now write a C function that takes Python objects as arguments, converts them to C objects
(so-called 'un-boxing'), calls ``fibonacci()`` then converts teh C result to a Python object (so-called 'boxing').

.. code-block:: c

    static PyObject *
    py_fibonacci(PyObject *Py_UNUSED(module), PyObject *args) {
        long index;

        if (!PyArg_ParseTuple(args, "l", &index)) {
            return NULL;
        }
        long result = fibonacci(index);
        return Py_BuildValue("l", result);
    }

Then we need to write some C code that defines the Python module that contains this function:

.. code-block:: c

    static PyMethodDef module_methods[] = {
            {"fibonacci",
                    (PyCFunction) py_fibonacci,
                    METH_VARARGS,
                    "Returns the Fibonacci value."
            },
            {NULL, NULL, 0, NULL} /* Sentinel */
    };

    static PyModuleDef cFibA = {
            PyModuleDef_HEAD_INIT,
            .m_name = "cFibA",
            .m_doc = "Fibonacci in C.",
            .m_size = -1,
            .m_methods = module_methods,
    };

    PyMODINIT_FUNC PyInit_cFibA(void) {
        PyObject *m = PyModule_Create(&cFibA);
        return m;
    }

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

.. code-block:: python

    >>> from cPyExtPatt.SimpleExample import cFibA
    >>> cFibA.fibonacci(8)
    21

There is a test file that uses ``timeit`` to check the performance of the Python and C code at
``src/cpy/SimpleExample/timeit_test.py``.


.. code-block:: bash

    (PythonExtPatt3.11_A) ➜  SimpleExample git:(develop) ✗ python timeit_test.py
    Index: 32 number of times: 20
    Version A, no cacheing:
    Python timeit: 7.321638
         C timeit: 0.131115
    C is 55.8 times FASTER.

    Version A with Python cache, no C cache:
    Python timeit: 0.000012
         C timeit: 0.130394
    C is 11058.7 times SLOWER.

    Version B, both are cached:
    Python timeit: 0.000004
         C timeit: 0.000007
    C is 1.9 times SLOWER.





Next up: understanding reference counts and Python's terminology.
