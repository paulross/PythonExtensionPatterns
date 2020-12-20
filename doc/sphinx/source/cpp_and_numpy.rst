.. toctree::
    :maxdepth: 2

.. _cpp_and_numpy:

====================================
C++ and the Numpy C API
====================================

`Numpy <http://www.numpy.org>`_ is a powerful arrary based data structure with fast vector and array operations. It has a fully featured `C API <https://docs.scipy.org/doc/numpy/reference/c-api.html>`_. This section describes some aspects of using Numpy with C++.

------------------------------------
Initialising Numpy
------------------------------------

The Numpy C API must be setup so that a number of static data structures are initialised correctly. The way to do this is to call ``import_array()`` which makes a number of Python import statements so the Python interpreter must be initialised first. This is described in detail in the `Numpy documentation <https://docs.scipy.org/doc/numpy/reference/c-api.array.html#miscellaneous>`_ so this document just presents a cookbook approach.


------------------------------------
Verifying Numpy is Initialised
------------------------------------

``import_array()`` always returns ``NUMPY_IMPORT_ARRAY_RETVAL`` regardless of success instead we have to check the Python error status:

.. code-block:: cpp

    #include <Python.h>    
    #include "numpy/arrayobject.h" // Include any other Numpy headers, UFuncs for example.
    
    // Initialise Numpy
    import_array();
    if (PyErr_Occurred()) {
        std::cerr << "Failed to import numpy Python module(s)." << std::endl;
        return NULL; // Or some suitable return value to indicate failure.
    }

In other running code where Numpy is expected to be initialised then ``PyArray_API`` should be non-NULL and this can be asserted:

.. code-block:: cpp

    assert(PyArray_API);

------------------------------------
Numpy Initialisation Techniques
------------------------------------


Initialising Numpy in a CPython Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Taking the simple example of a module from the `Python documentation <https://docs.python.org/3/extending/extending.html#a-simple-example>`_  we can add Numpy access just by including the correct Numpy header file and calling ``import_numpy()`` in the module initialisation code:

.. code-block:: cpp

    #include <Python.h>
    
    #include "numpy/arrayobject.h" // Include any other Numpy headers, UFuncs for example.
    
    static PyMethodDef SpamMethods[] = {
        ...
        {NULL, NULL, 0, NULL}        /* Sentinel */
    };    
    
    static struct PyModuleDef spammodule = {
       PyModuleDef_HEAD_INIT,
       "spam",   /* name of module */
       spam_doc, /* module documentation, may be NULL */
       -1,       /* size of per-interpreter state of the module,
                    or -1 if the module keeps state in global variables. */
       SpamMethods
    };

    PyMODINIT_FUNC
    PyInit_spam(void) {
        ...
        assert(! PyErr_Occurred());
        import_numpy(); // Initialise Numpy
        if (PyErr_Occurred()) {
            return NULL;
        }
        ...
        return PyModule_Create(&spammodule);
    }

That is fine for a singular translation unit but you have multiple translation units then each has to initialise the Numpy API which is a bit extravagant. The following sections describe how to manage this with multiple translation units.

Initialising Numpy in Pure C++ Code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This is mainly for development and testing of C++ code that uses Numpy. Your code layout might look something like this where ``main.cpp`` has a ``main()`` entry point and ``class.h`` has your class declarations and ``class.cpp`` has their implementations, like this::

    .
    └── src
        └── cpp
            ├── class.cpp
            ├── class.h
            └── main.cpp

The way of managing Numpy initialisation and access is as follows. In ``class.h`` choose a unique name such as ``awesome_project`` then include:

.. code-block:: cpp

    #define PY_ARRAY_UNIQUE_SYMBOL awesome_project_ARRAY_API
    #include "numpy/arrayobject.h"

In the implementation file ``class.cpp`` we do not want to import Numpy as that is going to be handled by ``main()`` in ``main.cpp`` so we put this at the top:

.. code-block:: cpp

    #define NO_IMPORT_ARRAY
    #include "class.h"

Finally in ``main.cpp`` we initialise Numpy:

.. code-block:: cpp

    #include "Python.h"
    #include "class.h"
    
    int main(int argc, const char * argv[]) {
        // ...
        // Initialise the Python interpreter
        wchar_t *program = Py_DecodeLocale(argv[0], NULL);
        if (program == NULL) {
            fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
            exit(1);
        }
        Py_SetProgramName(program);  /* optional but recommended */
        Py_Initialize();        
        // Initialise Numpy
        import_array();
        if (PyErr_Occurred()) {
            std::cerr << "Failed to import numpy Python module(s)." << std::endl;
            return -1;
        }
        assert(PyArray_API);
        // ...
    }    

If you have multiple .h, .cpp files then it might be worth having a single .h file, say ``numpy_init.h`` with just this in:

.. code-block:: cpp

    #define PY_ARRAY_UNIQUE_SYMBOL awesome_project_ARRAY_API
    #include "numpy/arrayobject.h"

Then each implementation .cpp file has:

.. code-block:: cpp

    #define NO_IMPORT_ARRAY
    #include "numpy_init.h"
    #include "class.h" // Class declarations

And ``main.cpp`` has:

.. code-block:: cpp

    #include "numpy_init.h"
    #include "class_1.h"
    #include "class_2.h"
    #include "class_3.h"
    
    int main(int argc, const char * argv[]) {
        // ...
        import_array();
        if (PyErr_Occurred()) {
            std::cerr << "Failed to import numpy Python module(s)." << std::endl;
            return -1;
        }
        assert(PyArray_API);
        // ...
    }    

Initialising Numpy in a CPython Module using C++ Code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Supposing you have laid out your source code in the following fashion::

    .
    └── src
        ├── cpp
        │   ├── class.cpp
        │   └── class.h
        └── cpython
            └── module.c
 
This is a hybrid of the above and typical for CPython C++ extensions where ``module.c`` contains the CPython code that allows Python to access the pure C++ code.

The code in ``class.h`` and ``class.cpp`` is unchanged and the code in ``module.c`` is essentially the same as that of a CPython module as described above where ``import_array()`` is called from within the ``PyInit_<module>`` function.


How These Macros Work Together
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The two macros ``PY_ARRAY_UNIQUE_SYMBOL`` and ``NO_IMPORT_ARRAY`` work together as follows:

+-----------------------------------+-------------------------------+-------------------------------+
|                                   |                               |                               |
+-----------------------------------+-------------------------------+-------------------------------+
|                                   |   ``PY_ARRAY_UNIQUE_SYMBOL``  |   ``PY_ARRAY_UNIQUE_SYMBOL``  |
|                                   |          NOT defined          |        defined as <NAME>      |
+-----------------------------------+-------------------------------+-------------------------------+
|                                   | C API is declared as:         | C API is declared as:         |
| ``NO_IMPORT_ARRAY`` not defined   | ``static void **PyArray_API`` | ``void **<NAME>``             |
|                                   | Which makes it only available | so can be seen by other       |
|                                   | to that translation unit.     | translation units.            |
+-----------------------------------+-------------------------------+-------------------------------+
|                                   | C API is declared as:         | C API is declared as:         |
| ``NO_IMPORT_ARRAY`` defined       | ``extern void **PyArray_API`` | ``extern void **<NAME>``      |
|                                   | so is available from another  | so is available from another  |
|                                   | translation unit.             | translation unit.             |
+-----------------------------------+-------------------------------+-------------------------------+


Adding a Search Path to a Virtual Environment
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you are linking to the system Python this may not have numpy installed, here is a way to cope with that. Create a virtual environment from the system python and install numpy:

.. code-block:: bash

    python -m venv <PATH_TO_VIRTUAL_ENVIRONMENT>
    source <PATH_TO_VIRTUAL_ENVIRONMENT>/bin/activate
    pip install numpy

Then in your C++ entry point add this function that manipulates ``sys.path``:

.. code-block:: cpp

    /** Takes a path and adds it to sys.paths by calling PyRun_SimpleString.
     * This does rather laborious C string concatenation so that it will work in
     * a primitive C environment.
     *
     * Returns 0 on success, non-zero on failure.
     */
    int add_path_to_sys_module(const char *path) {
        int ret = 0;
        const char *prefix = "import sys\nsys.path.append(\"";
        const char *suffix = "\")\n";
        char *command = (char*)malloc(strlen(prefix)
                                      + strlen(path)
                                      + strlen(suffix)
                                      + 1);
        if (! command) {
            return -1;
        }
        strcpy(command, prefix);
        strcat(command, path);
        strcat(command, suffix);
        ret = PyRun_SimpleString(command);
    #ifdef DEBUG
        printf("Calling PyRun_SimpleString() with:\n");
        printf("%s", command);
        printf("PyRun_SimpleString() returned: %d\n", ret);
        fflush(stdout);
    #endif
        free(command);
        return ret;
    }

``main()`` now calls this with the path to the virtual environment ``site-packages``:

.. code-block:: cpp

    int main(int argc, const char * argv[]) {
        wchar_t *program = Py_DecodeLocale(argv[0], NULL);
        if (program == NULL) {
            fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
            exit(1);
        }
        // Initialise the interpreter.
        Py_SetProgramName(program);  /* optional but recommended */
        Py_Initialize();
        const char *multiarray_path = "<PATH_TO_VIRTUAL_ENVIRONMENT_SITE_PACKAGES>";
        add_path_to_sys_module(multiarray_path);
        import_array();
        if (PyErr_Occurred()) {
            std::cerr << "Failed to import numpy Python module(s)." << std::endl;
            return -1;
        }
        assert(PyArray_API);
        // Your code here...
    }




