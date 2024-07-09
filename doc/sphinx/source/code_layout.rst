.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

=================================
Source Code Layout 
=================================

I find it useful to physically separate out the source code into different categories:

.. list-table:: Recommended Code Directories
   :widths: 10 10 10 10 10 30
   :header-rows: 1

   * - Category
     - Language
     - ``#include <Python.h>``?
     - Testable?
     - Where?
     - Description
   * - Pure Python
     - Python
     - No
     - Yes
     - ``py/``
     - Regular Python code tested by pytest or similar.
   * - CPython interface
     - Mostly C
     - Yes
     - No
     - ``cpy/``
     - C code that defines Python modules and classes. Functions that are exposed directly to Python.
   * - CPython utilities
     - C, C++
     - Yes
     - Yes
     - ``cpy/``
     - Utility C/C++ code that works with Python objects but these functions that are *not* exposed directly to Python.
       This code can be tested in a C/C++ environment with a specialised test framework.
       See :ref:`cpp_and_cpython` for some examples.
   * - C/C++ core
     - C, C++
     - No
     - Yes
     - ``cpp/``
     - C/C++ code that knows nothing about Python. This code can be tested in a C/C++ environment with a standard C/C++
       test framework.

--------------------------------------
Testing CPython Utility Code
--------------------------------------

When making Python C API calls from a C/C++ environment it is important to initialise the Python interpreter.
For example, this small program segfaults:

.. code-block:: c
    :linenos:
    :emphasize-lines: 4, 5, 6

    #include <Python.h>

    int main(int /* argc */, const char *[] /* argv[] */) {
        /* Forgot this:
        Py_Initialize();
        */
        PyErr_Format(PyExc_TypeError, "Stuff",);
        return 0;
    }

The reason is that ``PyErr_Format`` calls ``PyThreadState *thread_state = PyThreadState_Get();`` theen ``thread_state``
will be NULL unless the Python interpreter is initialised.

So you need to call ``Py_Initialize()`` to set up statically allocated interpreter data.
Alternatively put ``if (! Py_IsInitialized()) Py_Initialize();`` in every test. See: `https://docs.python.org/3/c-api/init.html <https://docs.python.org/3/c-api/init.html>`_

Here are a couple of useful C++ functions that assert all is well that can be used at the beginning of any function:

.. code-block:: c

    /* Returns non zero if Python is initialised and there is no Python error set.
     * The second version also checks that the given pointer is non-NULL
     * Use this thus, it will do nothing if NDEBUG is defined:
     *
     * assert(cpython_asserts());
     * assert(cpython_asserts(p));
     */
    int cpython_asserts() {
        return Py_IsInitialized() && PyErr_Occurred() == NULL;
    }

    int cpython_asserts(PyObject *pobj) {
        return cpython_asserts() && pobj != NULL;
    }
