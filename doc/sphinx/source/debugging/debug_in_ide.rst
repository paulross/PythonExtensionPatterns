.. highlight:: python
    :linenothreshold: 10

.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _debug-in-ide-label:

.. index::
    single: Debugging; IDEs

===============================================
Debuging Python C Extensions in an IDE
===============================================

``gdb`` and ``lldb`` work well with Python extensions but if you want to step through your C extension in an IDE here is one way to do it.

The basic idea is to compile/link your C extension in your IDE and get ``main()`` to call a function ``int import_call_execute(int argc, const char *argv[])`` that embeds the Python interpreter which then imports a Python module, say a unit test, that exercises your C extension code.

This ``import_call_execute()`` entry point is fairly generic and takes the standard arguments to ``main()`` so it can be in its own .h/.c file.

------------------------------------------------
Creating a Python Unit Test to Execute
------------------------------------------------

Suppose you have a Python extension ``ScList`` that sub-classes a list and counts the number of times ``.append(...)`` was called making this count available as a ``.appends`` property. You have a unit test called ``test_sclist.py`` that looks like this with a single function ``test()``:

.. code-block:: python

    import ScList

    def test():
        s = ScList.ScList()
        assert s.appends == 0
        s.append(8)
        assert s.appends == 1

-------------------------------------------------------
Writing a C Function to call any Python Unit Test
-------------------------------------------------------

We create the ``import_call_execute()`` function that takes that same arguments as ``main()`` which can forward its arguments. ``import_call_execute()`` expects 4 arguments:

* ``argc[0]`` - Name of the executable.
* ``argc[1]`` - Path to the directory that the Python module is in.
* ``argc[2]`` - Name of the Python module to be imported. This could be a unit test module for example.
* ``argc[3]`` - Name of the Python function in the Python module (no arguments will be supplied, the return value is ignored). This could be a particular unit test.

The ``import_call_execute()`` function does this:

#. Check the arguments and initialises the Python interpreter
#. Add the path to the ``argc[1]`` to ``sys.paths``.
#. Import ``argc[2]``.
#. Find the function ``argc[3]`` in module ``argc[2]`` and call it.
#. Clean up.


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Code Walk Through
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``import_call_execute()`` is quite generic and could be implemented in, say, ``py_import_call_execute.c``.

Here is a walk through of the implementation code:

Step 1: Check the arguments and initialise the Python interpreter

.. code-block:: c

    /** This imports a Python module and calls a specific function in it.
     * It's arguments are similar to main():
     * argc - Number of strings in argv
     * argv - Expected to be 4 strings:
     *      - Name of the executable.
     *      - Path to the directory that the Python module is in.
     *      - Name of the Python module.
     *      - Name of the function in the module.
     *
     * The Python interpreter will be initialised and the path to the Python module
     * will be added to sys.paths then the module will be imported.
     * The function will be called with no arguments and its return value will be
     * ignored.
     *
     * This returns 0 on success, non-zero on failure.
     */
    int import_call_execute(int argc, const char *argv[]) {
        int return_value = 0;
        PyObject *pModule   = NULL;
        PyObject *pFunc     = NULL;
        PyObject *pResult   = NULL;
    
        if (argc != 4) {
            fprintf(stderr,
                    "Wrong arguments!"
                    " Usage: %s package_path module function\n", argv[0]);
            return_value = -1;
            goto except;
        }
        Py_SetProgramName((wchar_t*)argv[0]);
        Py_Initialize();

Step 2: Add the path to the ``test_sclist.py`` to ``sys.paths``. For convenience we have a separate function ``add_path_to_sys_module()`` to do this.

.. code-block:: c

        if (add_path_to_sys_module(argv[1])) {
            return_value = -2;
            goto except;
        }

Here is the implementation of ``add_path_to_sys_module()``:

.. code-block:: c

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


Step 3: Import ``test_sclist``.

.. code-block:: c

        pModule = PyImport_ImportModule(argv[2]);
        if (! pModule) {
            fprintf(stderr,
                    EXECUTABLE_NAME ": Failed to load module \"%s\"\n", argv[2]);
            return_value = -3;
            goto except;
        }

Step 4: Find the function ``test()`` in ``test_sclist`` and call it.

.. code-block:: c

        pFunc = PyObject_GetAttrString(pModule, argv[3]);
        if (! pFunc) {
            fprintf(stderr,
                    EXECUTABLE_NAME ": Can not find function \"%s\"\n", argv[3]);
            return_value = -4;
            goto except;
        }
        if (! PyCallable_Check(pFunc)) {
            fprintf(stderr,
                    EXECUTABLE_NAME ": Function \"%s\" is not callable\n", argv[3]);
            return_value = -5;
            goto except;
        }
        pResult = PyObject_CallObject(pFunc, NULL);
        if (! pResult) {
            fprintf(stderr, EXECUTABLE_NAME ": Function call failed\n");
            return_value = -6;
            goto except;
        }
    #ifdef DEBUG
        printf(EXECUTABLE_NAME ": PyObject_CallObject() succeeded\n");
    #endif
        assert(! PyErr_Occurred());
        goto finally;

Step 5: Clean up.

.. code-block:: c

    except:
        assert(PyErr_Occurred());
        PyErr_Print();
    finally:
        Py_XDECREF(pFunc);
        Py_XDECREF(pModule);
        Py_XDECREF(pResult);
        Py_Finalize();
        return return_value;
    }

And then we need ``main()`` to call this, thus:

.. code-block:: c

    #include "py_import_call_execute.h"

    int main(int argc, const char *argv[]) {
        return import_call_execute(argc, argv);
    }


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Complete Code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The complete code for ``py_import_call_execute.c`` is here:

.. code-block:: c

    #include "py_import_call_execute.h"

    #include <Python.h>

    #ifdef __cplusplus
    extern "C" {
    #endif
    
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

    /** This imports a Python module and calls a specific function in it.
     * It's arguments are similar to main():
     * argc - Number of strings in argv
     * argv - Expected to be 4 strings:
     *      - Name of the executable.
     *      - Path to the directory that the Python module is in.
     *      - Name of the Python module.
     *      - Name of the function in the module.
     *
     * The Python interpreter will be initialised and the path to the Python module
     * will be added to sys.paths then the module will be imported.
     * The function will be called with no arguments and its return value will be
     * ignored.
     *
     * This returns 0 on success, non-zero on failure.
     */
    int import_call_execute(int argc, const char *argv[]) {
        int return_value = 0;
        PyObject *pModule   = NULL;
        PyObject *pFunc     = NULL;
        PyObject *pResult   = NULL;
    
        if (argc != 4) {
            fprintf(stderr,
                    "Wrong arguments!"
                    " Usage: %s package_path module function\n", argv[0]);
            return_value = -1;
            goto except;
        }
        Py_SetProgramName((wchar_t*)argv[0]);
        Py_Initialize();
        if (add_path_to_sys_module(argv[1])) {
            return_value = -2;
            goto except;
        }
        pModule = PyImport_ImportModule(argv[2]);
        if (! pModule) {
            fprintf(stderr,
                    "%s: Failed to load module \"%s\"\n", argv[0], argv[2]);
            return_value = -3;
            goto except;
        }
        pFunc = PyObject_GetAttrString(pModule, argv[3]);
        if (! pFunc) {
            fprintf(stderr,
                    "%s: Can not find function \"%s\"\n", argv[0], argv[3]);
            return_value = -4;
            goto except;
        }
        if (! PyCallable_Check(pFunc)) {
            fprintf(stderr,
                    "%s: Function \"%s\" is not callable\n", argv[0], argv[3]);
            return_value = -5;
            goto except;
        }
        pResult = PyObject_CallObject(pFunc, NULL);
        if (! pResult) {
            fprintf(stderr, "%s: Function call failed\n", argv[0]);
            return_value = -6;
            goto except;
        }
    #ifdef DEBUG
        printf("%s: PyObject_CallObject() succeeded\n", argv[0]);
    #endif
        assert(! PyErr_Occurred());
        goto finally;
    except:
        assert(PyErr_Occurred());
        PyErr_Print();
    finally:
        Py_XDECREF(pFunc);
        Py_XDECREF(pModule);
        Py_XDECREF(pResult);
        Py_Finalize();
        return return_value;
    }
    
    #ifdef __cplusplus
    //    extern "C" {
    #endif


.. index::
    single: Debugging; Xcode

--------------------------------------------
Debugging Python C Extensions in Xcode
--------------------------------------------

Build this code in Xcode, set break points in your extension, and run with the command line arguments:

``<path to test_sclist.py> test_sclist test``

And you should get something like this:

.. image:: ../images/DebugXcode.png
   :alt: Debugging in Xcode.
   :align: center

The full code for this is in *src/debugging/XcodeExample/PythonSubclassList/*.


--------------------------------------------
Using a Debug Version of Python C with Xcode
--------------------------------------------

To get Xcode to use a debug version of Python first build Python from source assumed here to be ``<SOURCE_DIR>`` with, as a minimum, ``--with-pydebug``. This example is using Python 3.6:

.. code-block:: console

    cd <SOURCE_DIR>
    mkdir debug-framework
    cd debug-framework/
    ../configure --with-pydebug --without-pymalloc --with-valgrind --enable-framework
    make

Then in Xcode select the project and "Add files to ..." and add:

* ``<SOURCE_DIR>/debug-framework/Python.framework/Versions/3.6/Python``
* ``<SOURCE_DIR>/debug-framework/libpython3.6d.a``

In "Build Settings":

* add ``/Library/Frameworks/Python.framework/Versions/3.6/include/python3.6m/`` to "Header Search Paths". Alternatively add both ``<SOURCE_DIR>/Include`` *and* ``<SOURCE_DIR>/debug-framework`` to "Header Search Paths", the latter is needed for ``pyconfig.h``.
* add ``<SOURCE_DIR>/debug-framework`` to "Library Search Paths".

Now you should be able to step into the CPython code.

--------------------------------------------
Debugging Python C Extensions in Eclipse
--------------------------------------------

The same ``main()`` can be used.

TODO: The intern can do this.
