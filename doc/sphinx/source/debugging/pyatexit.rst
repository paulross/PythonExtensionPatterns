.. highlight:: python
    :linenothreshold: 10

.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

=======================================================
Instrumenting the Python Process for Your Structures
=======================================================

Some debugging problems can be solved by instrumenting your C extensions for the duration of the Python process and reporting what happened when the process terminates. The data could be: the number of times classes were instantiated, functions called, memory allocations/deallocations or anything else that you wish.

To take a simple case, suppose we have a class that implements a up/down counter and we want to count how often each ``inc()`` and ``dec()`` function is called during the entirety of the Python process. We will create a C extension that has a class that has a single member (an interger) and two functions that increment or decrement that number. If it was in Python it would look like this:

.. code-block:: python
    
    class Counter:
        def __init__(self, count=0):
            self.count = count
        
        def inc(self):
            self.count += 1

        def dec(self):
            self.count -= 1

What we would like to do is to count how many times ``inc()`` and ``dec()`` are called on *all* instances of these objects and summarise them when the Python process exits [#f1]_.

There is an interpreter hook ``Py_AtExit()`` that allows you to register C functions that will be executed as the Python interpreter exits. This allows you to dump information that you have gathered about your code execution.

-------------------------------------------
An Implementation of a Counter
-------------------------------------------

First here is the module ``pyatexit`` with the class ``pyatexit.Counter`` with no intrumentation (it is equivelent to the Python code above). We will add the instrumentation later:

.. code-block:: c
    
    #include <Python.h>
    #include "structmember.h"

    #include <stdio.h>

    typedef struct {
        PyObject_HEAD int number;
    } Py_Counter;

    static void Py_Counter_dealloc(Py_Counter* self) {
        Py_TYPE(self)->tp_free((PyObject*)self);
    }

    static PyObject* Py_Counter_new(PyTypeObject* type, PyObject* args,
                                    PyObject* kwds) {
        Py_Counter* self;
        self = (Py_Counter*)type->tp_alloc(type, 0);
        if (self != NULL) {
            self->number = 0;
        }
        return (PyObject*)self;
    }

    static int Py_Counter_init(Py_Counter* self, PyObject* args, PyObject* kwds) {
        static char* kwlist[] = { "number", NULL };
        if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &self->number)) {
            return -1;
        }
        return 0;
    }

    static PyMemberDef Py_Counter_members[] = {
        { "count", T_INT, offsetof(Py_Counter, number), 0, "count value" },
        { NULL, 0, 0, 0, NULL } /* Sentinel */
    };

    static PyObject* Py_Counter_inc(Py_Counter* self) {
        self->number++;
        Py_RETURN_NONE;
    }

    static PyObject* Py_Counter_dec(Py_Counter* self) {
        self->number--;
        Py_RETURN_NONE;
    }

    static PyMethodDef Py_Counter_methods[] = {
        { "inc", (PyCFunction)Py_Counter_inc, METH_NOARGS, "Increments the counter" },
        { "dec", (PyCFunction)Py_Counter_dec, METH_NOARGS, "Decrements the counter" },
        { NULL, NULL, 0, NULL } /* Sentinel */
    };

    static PyTypeObject Py_CounterType = {
        PyVarObject_HEAD_INIT(NULL, 0) "pyatexit.Counter", /* tp_name */
        sizeof(Py_Counter), /* tp_basicsize */
        0, /* tp_itemsize */
        (destructor)Py_Counter_dealloc, /* tp_dealloc */
        0, /* tp_print */
        0, /* tp_getattr */
        0, /* tp_setattr */
        0, /* tp_reserved */
        0, /* tp_repr */
        0, /* tp_as_number */
        0, /* tp_as_sequence */
        0, /* tp_as_mapping */
        0, /* tp_hash  */
        0, /* tp_call */
        0, /* tp_str */
        0, /* tp_getattro */
        0, /* tp_setattro */
        0, /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
        "Py_Counter objects", /* tp_doc */
        0, /* tp_traverse */
        0, /* tp_clear */
        0, /* tp_richcompare */
        0, /* tp_weaklistoffset */
        0, /* tp_iter */
        0, /* tp_iternext */
        Py_Counter_methods, /* tp_methods */
        Py_Counter_members, /* tp_members */
        0, /* tp_getset */
        0, /* tp_base */
        0, /* tp_dict */
        0, /* tp_descr_get */
        0, /* tp_descr_set */
        0, /* tp_dictoffset */
        (initproc)Py_Counter_init, /* tp_init */
        0, /* tp_alloc */
        Py_Counter_new, /* tp_new */
        0, /* tp_free */
    };

    static PyModuleDef pyexitmodule = {
        PyModuleDef_HEAD_INIT, "pyatexit",
        "Extension that demonstrates the use of Py_AtExit().",
        -1, NULL, NULL, NULL, NULL,
        NULL
    };

    PyMODINIT_FUNC PyInit_pyatexit(void) {
        PyObject* m;

        if (PyType_Ready(&Py_CounterType) < 0) {
            return NULL;
        }
        m = PyModule_Create(&pyexitmodule);
        if (m == NULL) {
            return NULL;
        }
        Py_INCREF(&Py_CounterType);
        PyModule_AddObject(m, "Counter", (PyObject*)&Py_CounterType);
        return m;
    }

If this was a file ``Py_AtExitDemo.c`` then a Python ``setup.py`` file might look like this:

.. code-block:: python

    from distutils.core import setup, Extension
    setup(
        ext_modules=[
            Extension("pyatexit", sources=['Py_AtExitDemo.c']),
        ]
    )

Building this with ``python3 setup.py build_ext --inplace`` we can check everything works as expected:

.. code-block:: python

    >>> import pyatexit
    >>> c = pyatexit.Counter(8)
    >>> c.inc()
    >>> c.inc()
    >>> c.dec()
    >>> c.count
    9
    >>> d = pyatexit.Counter()
    >>> d.dec()
    >>> d.dec()
    >>> d.count
    -2
    >>> ^D
 
-------------------------------------------
Instrumenting the Counter
-------------------------------------------

To add the instrumentation we will declare a macro ``COUNT_ALL_DEC_INC`` to control whether the compilation includes instrumentation.

.. code-block:: c

    #define COUNT_ALL_DEC_INC

In the global area of the file declare some global counters and a function to write them out on exit. This must be a ``void`` function taking no arguments:

.. code-block:: c

    #ifdef COUNT_ALL_DEC_INC
    /* Counters for operations and a function to dump them at Python process end. */
    static size_t count_inc = 0;
    static size_t count_dec = 0;

    static void dump_inc_dec_count(void) {
        fprintf(stdout, "==== dump_inc_dec_count() ====\n");
        fprintf(stdout, "Increments: %" PY_FORMAT_SIZE_T "d\n", count_inc);
        fprintf(stdout, "Decrements: %" PY_FORMAT_SIZE_T "d\n", count_dec);
        fprintf(stdout, "== dump_inc_dec_count() END ==\n");
    }
    #endif

In the ``Py_Counter_new`` function we add some code to register this function. This must be only done once so we use the static ``has_registered_exit_function`` to guard this:

.. code-block:: c

    static PyObject* Py_Counter_new(PyTypeObject* type, PyObject* args,
                                    PyObject* kwds) {
        Py_Counter* self;
    #ifdef COUNT_ALL_DEC_INC
        static int has_registered_exit_function = 0;
        if (! has_registered_exit_function) {
            if (Py_AtExit(dump_inc_dec_count)) {
                return NULL;
            }
            has_registered_exit_function = 1;
        }
    #endif
        self = (Py_Counter*)type->tp_alloc(type, 0);
        if (self != NULL) {
            self->number = 0;
        }
        return (PyObject*)self;
    }

.. note::
    ``Py_AtExit`` can take, at most, 32 functions. If the function can not be registered then ``Py_AtExit`` will return -1.
    
.. warning::
    Since Python’s internal finalization will have completed before the cleanup function, no Python APIs should be called by any registered function.


Now we modify the ``inc()`` and ``dec()`` functions thus:

.. code-block:: c
 
    static PyObject* Py_Counter_inc(Py_Counter* self) {
        self->number++;
    #ifdef COUNT_ALL_DEC_INC
        count_inc++;
    #endif
        Py_RETURN_NONE;
    }

    static PyObject* Py_Counter_dec(Py_Counter* self) {
        self->number--;
    #ifdef COUNT_ALL_DEC_INC
        count_dec++;
    #endif
        Py_RETURN_NONE;
    }

Now when we build this extension and run it we see the following:

.. code-block:: python
 
    >>> import pyatexit
    >>> c = pyatexit.Counter(8)
    >>> c.inc()
    >>> c.inc()
    >>> c.dec()
    >>> c.count
    9
    >>> d = pyatexit.Counter()
    >>> d.dec()
    >>> d.dec()
    >>> d.count
    -2
    >>> ^D
    ==== dump_inc_dec_count() ====
    Increments: 2
    Decrements: 3
    == dump_inc_dec_count() END ==

.. rubric:: Footnotes

.. [#f1] The ``atexit`` module in Python can be used to similar effect however registered functions are called at a different stage of interpreted teardown than ``Py_AtExit``.
