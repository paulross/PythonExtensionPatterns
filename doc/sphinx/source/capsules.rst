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
            PyDoc_STR("Documentation for the spam module"),
            -1,       /* size of per-interpreter state of the module,
                     or -1 if the module keeps state in global variables. */
            SpamMethods,
            NULL, NULL, NULL, NULL,
    };

    PyMODINIT_FUNC
    PyInit_spam(void) {
        return PyModule_Create(&spammodule);
    }

This would be built with this entry in ``setup.py``:

.. code-block:: python

    Extension(f"{PACKAGE_NAME}.Capsules.spam", sources=['src/cpy/Capsules/spam.c',],
              include_dirs=['/usr/local/include', ],
              library_dirs=[os.getcwd(), ],
              extra_compile_args=extra_compile_args_c,
              language='c',
              ),

This can be tested with the code in ``tests/unit/test_c_capsules.py``:

.. code-block:: python

    from cPyExtPatt.Capsules import spam

    def test_spam():
        result = spam.system("ls -l")
        assert result == 0

---------------------------
An Exportable Extension
---------------------------

To make a version that exports its API in a Capsule we will make some changes:

- Introduce a function ``PySpam_System`` that actually does the system call. We will export this.
- Introduce a header file ``src/cpy/Capsules/spam_capsule.h`` that contains the exports.
- Introduce a macro ``SPAM_CAPSULE`` that controls whether we are exporting or importing the API.
- Change the module initialisation to initialise the API.

The header file in ``src/cpy/Capsules/spam_capsule.h`` looks like this:

.. code-block:: c

    #ifndef Py_SPAM_CAPSULE_H
    #define Py_SPAM_CAPSULE_H
    #ifdef __cplusplus
    extern "C" {
    #endif

    #include <Python.h>

    /* Header file for spammodule */

    /* C API functions */
    #define PySpam_System_NUM 0
    #define PySpam_System_RETURN int
    #define PySpam_System_PROTO (const char *command)

    /* Total number of C API pointers */
    #define PySpam_API_pointers 1

    #ifdef SPAM_CAPSULE

    /* This section is used when compiling spam_capsule.c */
    static PySpam_System_RETURN PySpam_System PySpam_System_PROTO;

    #else
    /* This section is used in modules that use spam_capsule's API */
    static void **PySpam_API;

    #define PySpam_System \
     (*(PySpam_System_RETURN (*)PySpam_System_PROTO) PySpam_API[PySpam_System_NUM])

    /* Return -1 on error, 0 on success.
     * PyCapsule_Import will set an exception if there's an error.
     */
    static int
    import_spam_capsule(void) {
        PySpam_API = (void **)PyCapsule_Import("cPyExtPatt.Capsules.spam_capsule._C_API", 0);
        return (PySpam_API != NULL) ? 0 : -1;
    }
    #endif
    #ifdef __cplusplus
    }
    #endif
    #endif /* !defined(Py_SPAM_CAPSULE_H) */

The full code is in ``src/cpy/Capsules/spam_capsule.c``:

.. code-block:: c

    // Implements: https://docs.python.org/3/extending/extending.html#extending-simpleexample
    // as a capsule: https://docs.python.org/3/extending/extending.html#providing-a-c-api-for-an-extension-module
    // Includes specific exception.
    // Lightly edited.

    #define PY_SSIZE_T_CLEAN

    #include <Python.h>

    #define SPAM_CAPSULE

    #include "spam_capsule.h"

    static int
    PySpam_System(const char *command) {
        return system(command);
    }

    static PyObject *
    spam_system(PyObject *Py_UNUSED(self), PyObject *args) {
        const char *command;
        int sts;

        if (!PyArg_ParseTuple(args, "s", &command))
            return NULL;
        sts = PySpam_System(command);
        return PyLong_FromLong(sts);
    }

    static PyMethodDef SpamMethods[] = {
            /* ... */
            {"system", spam_system, METH_VARARGS,
                    "Execute a shell command."},
            /* ... */
            {NULL, NULL, 0, NULL}        /* Sentinel */
    };

    static struct PyModuleDef spammodule = {
            PyModuleDef_HEAD_INIT,
            "spam_capsule",   /* name of module */
            PyDoc_STR("Documentation for the spam module"),
            -1,       /* size of per-interpreter state of the module,
                     or -1 if the module keeps state in global variables. */
            SpamMethods,
            NULL, NULL, NULL, NULL,
    };

    PyMODINIT_FUNC
    PyInit_spam_capsule(void) {
        PyObject *m;
        static void *PySpam_API[PySpam_API_pointers];
        PyObject *c_api_object;

        m = PyModule_Create(&spammodule);
        if (m == NULL)
            return NULL;

        /* Initialize the C API pointer array */
        PySpam_API[PySpam_System_NUM] = (void *) PySpam_System;

        /* Create a Capsule containing the API pointer array's address */
        c_api_object = PyCapsule_New((void *) PySpam_API, "cPyExtPatt.Capsules.spam_capsule._C_API", NULL);

        if (PyModule_AddObject(m, "_C_API", c_api_object) < 0) {
            Py_XDECREF(c_api_object);
            Py_DECREF(m);
            return NULL;
        }
        return m;
    }

This can be built by adding this Extension to ``setup.py``:

.. code-block:: python

        Extension(f"{PACKAGE_NAME}.Capsules.spam_capsule",
                  sources=['src/cpy/Capsules/spam_capsule.c',],
                  include_dirs=['/usr/local/include', 'src/cpy/Capsules',],
                  library_dirs=[os.getcwd(), ],
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),

This can be tested with the code in ``tests/unit/test_c_capsules.py``:

.. code-block:: python

    from cPyExtPatt.Capsules import spam_capsule

    def test_spam_capsule():
        result = spam_capsule.system("ls -l")
        assert result == 0

---------------------------
A Client Extension
---------------------------

This can now be used by another extension, say ``spam_client`` in ``src/cpy/Capsules/spam_client.c``.
Note the use of:

- ``#include "spam_capsule.h"``
- The call to the imported function ``PySpam_System``.
- Calling ``import_spam_capsule()`` in the module initialisation.

Here is the complete C code:

.. code-block:: c

    // Implements: https://docs.python.org/3/extending/extending.html#extending-simpleexample
    // but using a capsule exported by spam_capsule.h/.c
    // Excludes specific exception.
    // Lightly edited.

    #define PY_SSIZE_T_CLEAN

    #include <Python.h>
    #include "spam_capsule.h"

    static PyObject *
    spam_system(PyObject *Py_UNUSED(self), PyObject *args) {
        const char *command;
        int sts;

        if (!PyArg_ParseTuple(args, "s", &command))
            return NULL;
        sts = PySpam_System(command);
        return PyLong_FromLong(sts);
    }

    static PyMethodDef SpamMethods[] = {
            /* ... */
            {"system", spam_system, METH_VARARGS,
                    "Execute a shell command."},
            /* ... */
            {NULL, NULL, 0, NULL}        /* Sentinel */
    };

    static struct PyModuleDef spam_clientmodule = {
            PyModuleDef_HEAD_INIT,
            "spam_client",   /* name of module */
            PyDoc_STR("Documentation for the spam module"),
            -1,       /* size of per-interpreter state of the module,
                     or -1 if the module keeps state in global variables. */
            SpamMethods,
            NULL, NULL, NULL, NULL,
    };

    PyMODINIT_FUNC
    PyInit_spam_client(void) {
        PyObject *m;

        m = PyModule_Create(&spam_clientmodule);
        if (m == NULL)
            return NULL;
        if (import_spam_capsule() < 0)
            return NULL;
        /* additional initialization can happen here */
        return m;
    }

This can be built by adding this Extension to ``setup.py``:

.. code-block:: python

        Extension(f"{PACKAGE_NAME}.Capsules.spam_client",
                  sources=['src/cpy/Capsules/spam_client.c',],
                  include_dirs=['/usr/local/include', 'src/cpy/Capsules',],
                  library_dirs=[os.getcwd(), ],
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),

This can be tested with the code in ``tests/unit/test_c_capsules.py``:

.. code-block:: python

    from cPyExtPatt.Capsules import spam_client

    def test_spam_client():
        result = spam_client.system("ls -l")
        assert result == 0


================================
Using an Existing Capsule
================================

Here is an example of using an existing Capsule, the ``datetime`` C API.
In this case we want to create a subclass of the ``datetime.datetime`` object that always has a time zone i.e. no
`naive` datetimes.

Here is the C Extension code to create a ``datetimetz`` module and a ``datetimetz.datetimetz`` object.
This code is lightly edited for clarity and works with Python 3.10+.
The actual code is in ``src/cpy/Capsules/datetimetz.c`` (which works with Python 3.9 as well)
and the tests are in ``tests/unit/test_c_capsules.py``.

Firstly the declaration of the timezone aware datetime:

.. code-block:: c

    #define PY_SSIZE_T_CLEAN
    #include <Python.h>
    #include "datetime.h"

    typedef struct {
        PyDateTime_DateTime datetime;
    } DateTimeTZ;

Then a function that sets an error if the ``DateTimeTZ`` lacks a tzinfo, this will be used in a couple of places.

.. code-block:: c

    /* This function sets an error if a tzinfo is not set and returns NULL.
     * In practice this would use Python version specific calls.
     * For simplicity this uses Python 3.10+ code.
     */
    static DateTimeTZ *
    raise_if_no_tzinfo(DateTimeTZ *self) {
        if (!_PyDateTime_HAS_TZINFO(&self->datetime)) {
            PyErr_SetString(PyExc_TypeError, "No time zone provided.");
            Py_DECREF(self);
            self = NULL;
        }
        return self;
    }

Now the code for creating a new instance:

.. code-block:: c

    static PyObject *
    DateTimeTZ_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
        DateTimeTZ *self = (DateTimeTZ *) PyDateTimeAPI->DateTimeType->tp_new(type, args, kwds);
        if (self) {
            self = raise_if_no_tzinfo(self);
        }
        return (PyObject *) self;
    }

So far a new ``datetimetz`` object must be created with a ``tzinfo`` but the ``datetime.datetime`` has an API
``replace`` that creates a new datetime with different properties, including ``tzinfo``.
We need to guard against the user trying to change the timezone.
To do this we call the super class function and then check, and raise, if a ``tzinfo`` is absent.
This uses the utility function that call Python's ``super()`` function.
That code is in ``src/cpy/Util/py_call_super.h`` and ``src/cpy/Util/py_call_super.c``:

.. code-block:: c

    static PyObject *
    DateTimeTZ_replace(PyObject *self, PyObject *args, PyObject *kwargs) {
        PyObject *result = call_super_name(self, "replace", args, kwargs);
        if (result) {
            result = (PyObject *) raise_if_no_tzinfo((DateTimeTZ *) result);
        }
        return result;
    }

Finally the module code:

.. code-block:: c

    static PyMethodDef DateTimeTZ_methods[] = {
            {
                "replace",
                (PyCFunction) DateTimeTZ_replace,
                METH_VARARGS | METH_KEYWORDS,
                PyDoc_STR("Return datetime with new specified fields.")
            },
            {NULL, NULL, 0, NULL}  /* Sentinel */
    };

    static PyTypeObject DatetimeTZType = {
            PyVarObject_HEAD_INIT(NULL, 0)
            .tp_name = "datetimetz.datetimetz",
            .tp_doc = "A datetime that requires a time zone.",
            .tp_basicsize = sizeof(DateTimeTZ),
            .tp_itemsize = 0,
            .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
            .tp_new = DateTimeTZ_new,
            .tp_methods = DateTimeTZ_methods,
    };

    static PyModuleDef datetimetzmodule = {
            PyModuleDef_HEAD_INIT,
            .m_name = "datetimetz",
            .m_doc = (
                "Module that contains a datetimetz,"
                "a datetime.datetime with a mandatory time zone."
            ),
            .m_size = -1,
    };

    PyMODINIT_FUNC
    PyInit_datetimetz(void) {
        PyObject *m = PyModule_Create(&datetimetzmodule);
        if (m == NULL) {
            return NULL;
        }
        // datetime.datetime_CAPI
        PyDateTime_IMPORT;
        if (!PyDateTimeAPI) {
            Py_DECREF(m);
            return NULL;
        }
        // Set inheritance.
        DatetimeTZType.tp_base = PyDateTimeAPI->DateTimeType;
        if (PyType_Ready(&DatetimeTZType) < 0) {
            Py_DECREF(m);
            return NULL;
        }
        Py_INCREF(&DatetimeTZType);
        PyModule_AddObject(m, "datetimetz", (PyObject *) &DatetimeTZType);
        /* additional initialization can happen here */
        return m;
    }

The extension is created with this in ``setup.py``:

.. code-block:: python

        Extension(f"{PACKAGE_NAME}.Capsules.datetimetz",
                  sources=[
                      'src/cpy/Capsules/datetimetz.c',
                      'src/cpy/Util/py_call_super.c',
                  ],
                  include_dirs=[
                      '/usr/local/include',
                      'src/cpy/Capsules',
                      'src/cpy/Util',
                  ],
                  library_dirs=[os.getcwd(), ],
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),

Extensive tests are in ``tests/unit/test_c_capsules.py``.
