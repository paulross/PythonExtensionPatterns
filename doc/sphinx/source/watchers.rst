.. moduleauthor:: Paul Ross <apaulross@gmail.com>
.. sectionauthor:: Paul Ross <apaulross@gmail.com>

.. highlight:: python
    :linenothreshold: 30

.. toctree::
    :maxdepth: 3

..
    Links, mostly to the Python documentation.
    Specific container links are just before the appropriate section.

.. index::
    single: Watchers

.. _chapter_watchers:

.. index::
    single: Watchers

======================================
Watchers
======================================

From Python 3.12 onwards *watchers* have been added [#]_.
This allows registering a callback function on specific ``dict``, ``type``, ``code`` or ``function`` object.
The callback is called with any event that occurs on the specific object.
This can be a powerful debugging technique.

Here is an example of a dictionary watcher.

.. index::
    pair: Watchers; Dictionary

..
    Links to the Python dictionary documentation

.. _PyDict_AddWatcher(): https://docs.python.org/3/c-api/dict.html#c.PyDict_AddWatcher
.. _PyDict_ClearWatcher(): https://docs.python.org/3/c-api/dict.html#c.PyDict_ClearWatcher
.. _PyDict_Watch(): https://docs.python.org/3/c-api/dict.html#c.PyDict_Watch
.. _PyDict_UnWatch(): https://docs.python.org/3/c-api/dict.html#c.PyDict_UnWatch
.. _PyDict_WatchEvent: https://docs.python.org/3/c-api/dict.html#c.PyDict_WatchEvent
.. _PyDict_WatchCallback(): https://docs.python.org/3/c-api/dict.html#c.PyDict_WatchCallback

.. _chapter_watchers_dictionary:

.. index::
    single: Watchers; Dictionary

---------------------------
Dictionary Watchers
---------------------------

Here is a context manager ``cWatchers.PyDictWatcher`` that wraps the low level CPython code with a watcher
that reports every dictionary operation to ``stdout``.
The code is in ``src/cpy/Watchers/watcher_example.py``.

.. code-block:: python
    :linenos:

    """Example of using watchers."""
    
    from cPyExtPatt import cWatchers
    
    
    def dict_watcher_demo() -> None:
        print('dict_watcher_demo():')
        d = {}                              # The dictionary we are going to watch.
        with cWatchers.PyDictWatcher(d):
            dd = {'age': 17, }
            d.update(dd)                    # Generates event: PyDict_EVENT_CLONED
            d['age'] = 42                   # Generates event: PyDict_EVENT_MODIFIED
            del d['age']                    # Generates event: PyDict_EVENT_DELETED
            d['name'] = 'Python'            # Generates event: PyDict_EVENT_ADDED
            d.clear()                       # Generates event: PyDict_EVENT_CLEARED
            del d
    
    
    if __name__ == '__main__':
        dict_watcher_demo()

And the output would be something like this, it reports the Python file, line number, function, event and
then detail about the arguments used to manipulate the dictionary:

.. raw:: latex

    [Continued on the next page]

    \pagebreak

..
    .. raw:: latex
    
        \begin{landscape}

.. code-block:: text

    dict_watcher_demo():
    Dict @ 0x0x10fb53c00: watcher_example.py 11 dict_watcher_demo PyDict_EVENT_CLONED
        Dict: {}
        Key (dict): {'age': 17}
        New value : NULL
    Dict @ 0x0x10fb53c00: watcher_example.py 12 dict_watcher_demo PyDict_EVENT_MODIFIED
        Dict: {'age': 17}
        Key (str): age
        New value (int): 42
    Dict @ 0x0x10fb53c00: watcher_example.py 13 dict_watcher_demo PyDict_EVENT_DELETED
        Dict: {'age': 42}
        Key (str): age
        New value : NULL
    Dict @ 0x0x10fb53c00: watcher_example.py 14 dict_watcher_demo PyDict_EVENT_ADDED
        Dict: {}
        Key (str): name
        New value (str): Python
    Dict @ 0x0x10fb53c00: watcher_example.py 15 dict_watcher_demo PyDict_EVENT_CLEARED
        Dict: {'name': 'Python'}
        Key : NULL
        New value : NULL

..
    .. raw:: latex
    
        \end{landscape}

There are some obvious variations here:

- Add some prefix to each watcher output line to discriminate it from the rest of stdout.
- Different outputs, such as JSON.

But how does this watcher work?

.. index::
    single: Watchers; Dictionary; Implementation

Low Level C Implementation
--------------------------

We need some low level C code that interacts with the CPython watcher API.
First a header file that provides the interface to our dictionary watcher code.
This declares two functions:

- ``dict_watcher_verbose_add()`` this adds a watcher to a dictionary and returns the watcher ID.
- ``dict_watcher_verbose_remove()`` this removes a watcher ID from a dictionary.

The actual code is in ``src/cpy/Watchers/DictWatcher.h``.
It looks like this, note the Python version guard to ensure this only works with Python 3.12+:

.. code-block:: c

    #define PPY_SSIZE_T_CLEAN
    #include "Python.h"

    #if PY_VERSION_HEX < 0x030C0000

    #error "Required version of Python is 3.12+ (PY_VERSION_HEX >= 0x030C0000)"

    #else

    int dict_watcher_verbose_add(PyObject *dict);
    int dict_watcher_verbose_remove(int watcher_id, PyObject *dict);

    #endif // #if PY_VERSION_HEX >= 0x030C0000

So there are several moving parts in the implementation in  ``src/cpy/Watchers/DictWatcher.c``.
First we have some general purpose functions that extract the file name, function name and line number from a Python
frame.
Note that the Python frame API changed in Python 3.11.

First up, getting the Python file name:

.. code-block:: c

    #include "DictWatcher.h"

    static const unsigned char *
    get_python_file_name(PyFrameObject *frame) {
        if (frame) {
    // Python 3.11+ specific code.
    #if PY_VERSION_HEX >= 0x030B0000
            /* See:
             * https://docs.python.org/3.11/whatsnew/3.11.html#pyframeobject-3-11-hiding
             */
            const unsigned char *file_name = PyUnicode_1BYTE_DATA(
                PyFrame_GetCode(frame)->co_filename
            );
    #else
            const unsigned char *file_name = PyUnicode_1BYTE_DATA(
                frame->f_code->co_filename
            );
    #endif // #if PY_VERSION_HEX >= 0x030B0000
            return file_name;
        }
        return "";
    }

Now, getting the Python function name:

.. code-block:: c

    static const char *
    get_python_function_name(PyFrameObject *frame) {
        const char *func_name = NULL;
        if (frame) {
    // Python 3.11+ specific code.
    #if PY_VERSION_HEX >= 0x030B0000
            /* See:
             * https://docs.python.org/3.11/whatsnew/3.11.html#pyframeobject-3-11-hiding
             */
            func_name = (const char *) PyUnicode_1BYTE_DATA(
                PyFrame_GetCode(frame)->co_name
            );
    #else
            func_name = (const char *) PyUnicode_1BYTE_DATA(frame->f_code->co_name);
    #endif // #if PY_VERSION_HEX >= 0x030B0000
            return func_name;
        }
        return "";
    }

.. raw:: latex

    [Continued on the next page]

    \pagebreak

Then, getting the Python line number:

.. code-block:: c

    int get_python_line_number(PyFrameObject *frame) {
        if (frame) {
            return PyFrame_GetLineNumber(frame);
        }
        return 0;
    }

We bring these together to print a summary of the frame state to a file, such as ``stdout``:

.. code-block:: c

    static void
    write_frame_data_to_outfile(FILE *outfile, PyFrameObject *frame) {
        if (frame) {
            fprintf(outfile,
                    "%-80s %6d %-24s",
                    get_python_file_name(frame),
                    get_python_line_number(frame),
                    get_python_function_name(frame)
                    );
        } else {
            fprintf(outfile, "No Python frame available.");
        }
    }

Then there is a simple little helper function that returns a string based on the event type:

.. code-block:: c

    static const char *watch_event_name(PyDict_WatchEvent event) {
        switch (event) {
            case PyDict_EVENT_ADDED:
                return "PyDict_EVENT_ADDED";
                break;
            case PyDict_EVENT_MODIFIED:
                return "PyDict_EVENT_MODIFIED";
                break;
            case PyDict_EVENT_DELETED:
                return "PyDict_EVENT_DELETED";
                break;
            case PyDict_EVENT_CLONED:
                return "PyDict_EVENT_CLONED";
                break;
            case PyDict_EVENT_CLEARED:
                return "PyDict_EVENT_CLEARED";
                break;
            case PyDict_EVENT_DEALLOCATED:
                return "PyDict_EVENT_DEALLOCATED";
                break;
            default:
                Py_UNREACHABLE();
                break;
        }
        return "PyDict_EVENT_UNKNOWN";
    }

Now we define the callback function that reports the dictionary event to ``stdout``.
This calles all the functionas above and uses the CPython API ``PyObject_Print`` to print the representation
of each object to ``stdout``.
This function has to respect NULL arguments:

.. code-block:: c

    static int dict_watcher_verbose(PyDict_WatchEvent event, PyObject *dict,
                                    PyObject *key, PyObject *new_value) {
        fprintf(stdout, "Dict @ 0x%p: ", (void *)dict);
        write_frame_data_to_outfile(stdout, PyEval_GetFrame());
        fprintf(stdout, " Event: %-24s", watch_event_name(event));
        fprintf(stdout, "\n");
        if (dict) {
            fprintf(stdout, "    Dict: ");
            PyObject_Print(dict, stdout, Py_PRINT_RAW);
        } else {
            fprintf(stdout, "    Dict: NULL");
        }
        fprintf(stdout, "\n");
        if (key) {
            fprintf(stdout, "    Key (%s): ", Py_TYPE(key)->tp_name);
            PyObject_Print(key, stdout, Py_PRINT_RAW);
        } else {
            fprintf(stdout, "    Key : NULL");
        }
        fprintf(stdout, "\n");
        if (new_value) {
            fprintf(stdout, "    New value (%s): ", Py_TYPE(new_value)->tp_name);
            PyObject_Print(new_value, stdout, Py_PRINT_RAW);
        } else {
            fprintf(stdout, "    New value : NULL");
        }
        fprintf(stdout, "\n");
        return 0;
    }

Finally we have the two implementations that register and unregister the callback using the low level Python C API.
This uses `PyDict_AddWatcher()`_ and `PyDict_Watch()`_.
The first registers the callback, returning the watcher ID (error handling code omitted):

.. code-block:: c

    // Set watcher.
    int dict_watcher_verbose_add(PyObject *dict) {
        int watcher_id = PyDict_AddWatcher(&dict_watcher_verbose);
        PyDict_Watch(watcher_id, dict);
        return watcher_id;
    }

The second de-registers the callback, with the watcher ID and the dictionary in question
(error handling code omitted).
This uses `PyDict_Unwatch()`_ and `PyDict_ClearWatcher()`_.

.. code-block:: c

    // Remove watcher.
    int dict_watcher_verbose_remove(int watcher_id, PyObject *dict) {
        PyDict_Unwatch(watcher_id, dict);
        PyDict_ClearWatcher(watcher_id);
        return 0;
    }

Exposing This to CPython
------------------------

Now we create a Python module ``cWatchers`` that exposes this low level C code to CPython.
This code is in ``src/cpy/Watchers/cWatchers.c``.

First some module level CPython wrappers around our underlying C code:

.. code-block:: c

    #define PPY_SSIZE_T_CLEAN
    #include "Python.h"

    #include "DictWatcher.h"

    static PyObject *
    py_dict_watcher_verbose_add(PyObject *Py_UNUSED(module), PyObject *arg) {
        if (!PyDict_Check(arg)) {
            PyErr_Format(
                PyExc_TypeError,
                "Argument must be a dict not type %s",
                Py_TYPE(arg)->tp_name
            );
            return NULL;
        }
        long watcher_id = dict_watcher_verbose_add(arg);
        return Py_BuildValue("l", watcher_id);
    }

    static PyObject *
    py_dict_watcher_verbose_remove(PyObject *Py_UNUSED(module), PyObject *args) {
        long watcher_id;
        PyObject *dict = NULL;

        if (!PyArg_ParseTuple(args, "lO", &watcher_id, &dict)) {
            return NULL;
        }

        if (!PyDict_Check(dict)) {
            PyErr_Format(
                PyExc_TypeError,
                "Argument must be a dict not type %s",
                Py_TYPE(dict)->tp_name
            );
            return NULL;
        }
        long result = dict_watcher_verbose_remove(watcher_id, dict);
        return Py_BuildValue("l", result);
    }

Now create the table of module methods:

.. code-block:: c

    static PyMethodDef module_methods[] = {
            {"py_dict_watcher_verbose_add",
                    (PyCFunction) py_dict_watcher_verbose_add,
                    METH_O,
                    "Adds watcher to a dictionary. Returns the watcher ID."
            },
            {"py_dict_watcher_verbose_remove",
                    (PyCFunction) py_dict_watcher_verbose_remove,
                    METH_VARARGS,
                    "Removes the watcher ID from the dictionary."
            },
            {NULL, NULL, 0, NULL} /* Sentinel */
    };

.. index::
    single: Watchers; Dictionary; Context Manager

Creating the Context Manager
-----------------------------

These are all fine but to be Pythonic it would be helpful to create a Context Manager
(see :ref:`chapter_context_manager`) in C.
The context manager holds a reference to the dictionary and the watcher ID.
Here is the definition which holds a watcher ID and a reference to the dictionary:

.. code-block:: c

    typedef struct {
        PyObject_HEAD
        int watcher_id;
        PyObject *dict;
    } PyDictWatcher;

Here is the creation code:

.. code-block:: c

    static PyDictWatcher *
    PyDictWatcher_new(PyObject *Py_UNUSED(arg)) {
        PyDictWatcher *self;
        self = PyObject_New(PyDictWatcher, &PyDictWatcher_Type);
        if (self == NULL) {
            return NULL;
        }
        self->watcher_id = -1;
        self->dict = NULL;
        return self;
    }

    static PyObject *
    PyDictWatcher_init(PyDictWatcher *self, PyObject *args) {
        if (!PyArg_ParseTuple(args, "O", &self->dict)) {
            return NULL;
        }
        if (!PyDict_Check(self->dict)) {
            PyErr_Format(
                PyExc_TypeError,
                "Argument must be a dictionary not a %s",
                Py_TYPE(self->dict)->tp_name
            );
            return NULL;
        }
        Py_INCREF(self->dict);
        return (PyObject *)self;
    }

The destruction code just decrements the reference count of the dictionary:

.. code-block:: c

    static void
    PyDictWatcher_dealloc(PyDictWatcher *self) {
        Py_DECREF(self->dict);
        PyObject_Del(self);
    }

Now the code that provides the context manager's ``__enter__`` and ``__exit__`` methods.
First the ``__enter__`` function, this uses the low level C function ``dict_watcher_verbose_add()`` to mark the
dictionary as watched and hold the watcher ID:

.. code-block:: c

    static PyObject *
    PyDictWatcher_enter(PyDictWatcher *self, PyObject *Py_UNUSED(args)) {
        self->watcher_id = dict_watcher_verbose_add(self->dict);
        Py_INCREF(self);
        return (PyObject *)self;
    }

Now the ``__exit__`` function, this uses the low level C function ``dict_watcher_verbose_remove()`` to remove the
watcher from the dictionary:

.. code-block:: c

    static PyObject *
    PyDictWatcher_exit(PyDictWatcher *self, PyObject *Py_UNUSED(args)) {
        int result = dict_watcher_verbose_remove(self->watcher_id, self->dict);
        if (result) {
            PyErr_Format(
                PyExc_RuntimeError,
                "dict_watcher_verbose_remove() returned %d",
                result
            );
            Py_RETURN_TRUE;
        }
        Py_RETURN_FALSE;
    }

Now we define the context manager methods and type:

.. code-block:: c

    static PyMethodDef PyDictWatcher_methods[] = {
            {"__enter__", (PyCFunction) PyDictWatcher_enter, METH_VARARGS,
                    PyDoc_STR("__enter__() -> PyDictWatcher")},
            {"__exit__", (PyCFunction) PyDictWatcher_exit, METH_VARARGS,
                    PyDoc_STR("__exit__(exc_type, exc_value, exc_tb) -> bool")},
            {NULL, NULL, 0, NULL} /* sentinel */
    };

    static PyTypeObject PyDictWatcher_Type = {
            PyVarObject_HEAD_INIT(NULL, 0)
            .tp_name = "cWatchers.PyDictWatcher",
            .tp_basicsize = sizeof(PyDictWatcher),
            .tp_dealloc = (destructor) PyDictWatcher_dealloc,
            .tp_flags = Py_TPFLAGS_DEFAULT,
            .tp_methods = PyDictWatcher_methods,
            .tp_new = (newfunc) PyDictWatcher_new,
            .tp_init = (initproc) PyDictWatcher_init
    };

.. index::
    single: Watchers; Dictionary; Module, Setup and Test

Module, Setup and Test
-----------------------------

Now we create the ``cWatchers`` module,

.. code-block:: c

    static PyModuleDef cWatchers = {
            PyModuleDef_HEAD_INIT,
            .m_name = "cWatchers",
            .m_doc = "Dictionary and type watchers.",
            .m_size = -1,
            .m_methods = module_methods,
    };

    PyMODINIT_FUNC PyInit_cWatchers(void) {
        PyObject *m = PyModule_Create(&cWatchers);
        if (!m) {
            goto fail;
        }
        if (PyType_Ready(&PyDictWatcher_Type) < 0) {
            goto fail;
        }
        if (PyModule_AddObject(m, "PyDictWatcher", (PyObject *) &PyDictWatcher_Type)) {
            goto fail;
        }
        return m;
    fail:
        Py_XDECREF(m);
        return NULL;
    }


And then in ``setup.py`` we add the extension:

.. code-block:: python

    Extension(
        name=f"{PACKAGE_NAME}.cWatchers",
        include_dirs=['src/cpy', ],
        sources=[
            "src/cpy/Watchers/cWatchers.c",
            "src/cpy/Watchers/DictWatcher.c",
            "src/cpy/pyextpatt_util.c",
        ],
        extra_compile_args=extra_compile_args_c,
        language='c',
    ),

And it can be used like this:

.. code-block:: python

    from cPyExtPatt import cWatchers

    d = {}
    with cWatchers.PyDictWatcher(d):
        d['age'] = 42

.. raw:: latex

    [Continued on the next page]

    \pagebreak

And the result on ``stdout`` is something like:

.. code-block:: text

    Dict @ 0x0x10fb53c00: watcher_example.py 12 dict_watcher_demo PyDict_EVENT_ADDED
        Dict: {}
        Key (str): age
        New value (int): 42

.. index::
    single: Watchers; Dictionary; No Context Manager

Without the Context Manager
---------------------------

If you are putting in some debugging code then a context manager might not be convenient.
``cWatchers`` provides two functions, ``py_dict_watcher_verbose_add()`` and
``py_dict_watcher_verbose_remove`` that achieve the same aim:

.. code-block:: python

    from cPyExtPatt import cWatchers

    d = {}
    watcher_id = cWatchers.py_dict_watcher_verbose_add(d)
    d['age'] = 42
    cWatchers.py_dict_watcher_verbose_remove(watcher_id, d)


.. _PyType_AddWatcher(): https://docs.python.org/3/c-api/type.html#c.PyType_AddWatcher
.. _PyType_ClearWatcher(): https://docs.python.org/3/c-api/type.html#c.PyType_ClearWatcher
.. _PyType_Watch(): https://docs.python.org/3/c-api/type.html#c.PyType_Watch
.. _PyType_UnWatch(): https://docs.python.org/3/c-api/type.html#c.PyType_UnWatch
.. _PyType_WatchEvent: https://docs.python.org/3/c-api/type.html#c.PyType_WatchEvent
.. _PyType_WatchCallback(): https://docs.python.org/3/c-api/type.html#c.PyType_WatchCallback

---------------------------
Type Watchers
---------------------------

These allow a callback when a type is modified.

This beyond the scope of this version of this document.

More information can be found in https://docs.python.org/3/c-api/type.html

---------------------------
Function Watchers
---------------------------

These allow a callback when a function is created and destroyed.

This beyond the scope of this version of this document.

More information can be found in https://docs.python.org/3/c-api/function.html

---------------------------
Code Watchers
---------------------------

These allow a callback when code is created and destroyed.

This beyond the scope of this version of this document.

More information can be found in https://docs.python.org/3/c-api/code.html

.. rubric:: Footnotes

.. [#] This change was not done with any PEP that I can find.
    Instead it was done during the ordinary pace of development.
    For example this change has example tests: https://github.com/python/cpython/pull/31787/files
    and is tracked with this issue: https://github.com/python/cpython/issues/91052

