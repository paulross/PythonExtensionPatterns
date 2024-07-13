.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

***************
Capsules
***************

Usually C extension code is used solely by the extension itself, for that reason all functions are declared ``static``
however there are cases where the C extension code is useful to another C extension.
When modules are used as shared libraries the symbols in one extension might not be visible to another extension.

`Capsules <https://docs.python.org/3/extending/extending.html#providing-a-c-api-for-an-extension-module>`_
are a means by which this can be achieved by passing C pointers from one extension module to another.


================================
A Simple Example
================================

This takes the example given in the Python documentation and makes it complete.
The code is in ``src/cpy/Capsules/spam*``.

---------------------------
Basic Extension
---------------------------

Here is the basic example of an Extension that can make a system call.
The code is in ``src/cpy/Capsules/spam.c``.

.. code-block:: c

    #define PY_SSIZE_T_CLEAN
    #include <Python.h>

    static PyObject *
    spam_system(PyObject *Py_UNUSED(self), PyObject *args) {
        const char *command;
        int sts;

        if (!PyArg_ParseTuple(args, "s", &command))
            return NULL;
        sts = system(command);
        return PyLong_FromLong(sts);
    }

    static PyMethodDef SpamMethods[] = {
            /* ... */
            {"system",  spam_system, METH_VARARGS,
                        "Execute a shell command."},
            /* ... */
            {NULL, NULL, 0, NULL}        /* Sentinel */
    };

    static struct PyModuleDef spammodule = {
            PyModuleDef_HEAD_INIT,
            "spam",   /* name of module */
            PyDoc_STR("Documentation for the spam module"), /* module documentation, may be NULL */
            -1,       /* size of per-interpreter state of the module,
                     or -1 if the module keeps state in global variables. */
            SpamMethods,
            NULL,
            NULL,
            NULL,
            NULL,
    };

    PyMODINIT_FUNC
    PyInit_spam(void) {
        return PyModule_Create(&spammodule);
    }

This would be built wit this entry in the ``setup.py``:

.. code-block:: python

    Extension(f"{PACKAGE_NAME}.Capsules.spam", sources=['src/cpy/Capsules/spam.c',],
              include_dirs=['/usr/local/include', ],  # os.path.join(os.getcwd(), 'include'),],
              library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
              extra_compile_args=extra_compile_args_c,
              language='c',
                  ),


