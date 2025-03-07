.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3


.. _chapter_logging_and_frames:
.. _chapter_logging_and_frames.logging:

.. index::
    single: Logging

=================================
Logging and Frames
=================================

This chapter describes how to use the Python logging interface for C and how to access Python frames from C.

----------------------
Logging From C
----------------------

This presents a recipe for using the Python logging module in C.

Many thanks to `nnathan <https://github.com/nnathan>`_ for this.

We import the module and define C equivalent logging functions that are
compatible with the `*printf` family.

Logging C Declarations
----------------------

First define the log levels, ``#define`` is used so the that switch case does not issue a compile time non-const error:

.. code-block:: c

    #define PPY_SSIZE_T_CLEAN
    #include <Python.h>
    /* For va_start, va_end */
    #include <stdarg.h>

    /* logging levels defined by logging module
     * From: https://docs.python.org/3/library/logging.html#logging-levels */
    #define LOGGING_DEBUG 10
    #define LOGGING_INFO 20
    #define LOGGING_WARNING 30
    #define LOGGING_ERROR 40
    #define LOGGING_CRITICAL 50
    #define LOGGING_EXCEPTION 60

Logging C Globals
----------------------

Then two globals, the first is the imported logging module, the next the current logger:

.. code-block:: c

    /* This modules globals */
    static PyObject *g_logging_module = NULL; /* Initialise by PyInit_cLogging() below. */
    static PyObject *g_logger = NULL;

Logging C Functions
----------------------

Now a function to get a logger object from the logging module:

.. code-block:: c

    static PyObject *py_get_logger(char *logger_name) {
        assert(g_logging_module);
        PyObject *logger = NULL;

        logger = PyObject_CallMethod(g_logging_module, "getLogger", "s", logger_name);
        if (logger == NULL) {
            const char *err_msg = "failed to call logging.getLogger";
            PyErr_SetString(PyExc_RuntimeError, err_msg);
        }
        /*
        fprintf(stdout, "%s()#%d logger=0x%p\n", __FUNCTION__, __LINE__, (void *)logger);
         */
        return logger;
    }

Now the main interface to logging function:

.. code-block:: c

    static PyObject *
    py_log_msg(int log_level, char *printf_fmt, ...) {
        assert(g_logger);
        assert(!PyErr_Occurred());
        PyObject *log_msg = NULL;
        PyObject *ret = NULL;
        va_list fmt_args;

        va_start(fmt_args, printf_fmt);
        log_msg = PyUnicode_FromFormatV(printf_fmt, fmt_args);
        va_end(fmt_args);

        if (log_msg == NULL) {
            /* fail. */
            ret = PyObject_CallMethod(
                    g_logger,
                    "critical",
                    "O", "Unable to create log message."
            );
        } else {
            /* call function depending on loglevel */
            switch (log_level) {
                case LOGGING_DEBUG:
                    ret = PyObject_CallMethod(g_logger, "debug", "O", log_msg);
                    break;
                case LOGGING_INFO:
                    ret = PyObject_CallMethod(g_logger, "info", "O", log_msg);
                    break;
                case LOGGING_WARNING:
                    ret = PyObject_CallMethod(g_logger, "warning", "O", log_msg);
                    break;
                case LOGGING_ERROR:
                    ret = PyObject_CallMethod(g_logger, "error", "O", log_msg);
                    break;
                case LOGGING_CRITICAL:
                    ret = PyObject_CallMethod(g_logger, "critical", "O", log_msg);
                    break;
                default:
                    ret = PyObject_CallMethod(g_logger, "critical", "O", log_msg);
                    break;
            }
            assert(!PyErr_Occurred());
        }
        Py_DECREF(log_msg);
        return ret;
    }

A function to set a log level:

.. code-block:: c

    static PyObject *
    py_log_set_level(PyObject *Py_UNUSED(module), PyObject *args) {
        assert(g_logger);
        PyObject *py_log_level;

        if (!PyArg_ParseTuple(args, "O", &py_log_level)) {
            return NULL;
        }
        return PyObject_CallMethod(g_logger, "setLevel", "O", py_log_level);
    }

And the main function to log a message:

.. code-block:: c

    static PyObject *
    py_log_message(PyObject *Py_UNUSED(module), PyObject *args) {
        int log_level;
        char *message;

        if (!PyArg_ParseTuple(args, "iz", &log_level, &message)) {
            return NULL;
        }
        return py_log_msg(log_level, "%s", message);
    }

cLogging Module
----------------------

Setup the module functions:

.. code-block:: c

    static PyMethodDef logging_methods[] = {
        /* ... */
        {
            "py_log_set_level",
            (PyCFunction) py_log_set_level,
            METH_VARARGS,
            "Set the logging level."
        },
        {
            "log",
            (PyCFunction) py_log_message,
            METH_VARARGS,
            "Log a message."
        },
        /* ... */
        {NULL, NULL, 0, NULL} /* Sentinel */
    };

The module definition:

.. code-block:: c

    static PyModuleDef cLogging = {
            PyModuleDef_HEAD_INIT,
            .m_name = "cLogging",
            .m_doc = "Logging mmodule.",
            .m_size = -1,
            .m_methods = logging_methods,
    };

The module initialisation, this is where the logging module is imported with ``PyImport_ImportModule()``:

.. code-block:: c

    PyMODINIT_FUNC PyInit_cLogging(void) {
        PyObject *m = PyModule_Create(&cLogging);
        if (! m) {
            goto except;
        }
        g_logging_import = PyImport_ImportModule("logging");
        if (!g_logging_import) {
            const char *err_msg = "failed to import 'logging'";
            PyErr_SetString(PyExc_ImportError, err_msg);
            goto except;
        }
        g_logger = py_get_logger("cLogging");
        if (!g_logger) {
            goto except;
        }
        /* Adding module globals */
        /* logging levels defined by logging module Note: In Python logging FATAL = CRITICAL */
        if (PyModule_AddIntConstant(m, "INFO", LOGGING_INFO)) {
            goto except;
        }
        if (PyModule_AddIntConstant(m, "WARNING", LOGGING_WARNING)) {
            goto except;
        }
        if (PyModule_AddIntConstant(m, "ERROR", LOGGING_ERROR)) {
            goto except;
        }
        if (PyModule_AddIntConstant(m, "FATAL", LOGGING_FATAL)) {
            goto except;
        }
        if (PyModule_AddIntConstant(m, "CRITICAL", LOGGING_FATAL)) {
            goto except;
        }
        if (PyModule_AddIntConstant(m, "DEBUG", LOGGING_DEBUG)) {
            goto except;
        }
        if (PyModule_AddIntConstant(m, "EXCEPTION", LOGGING_EXCEPTION)) {
            goto except;
        }

        goto finally;
    except:
        /* abnormal cleanup */
        /* cleanup logger references */
        Py_XDECREF(g_logging_import);
        Py_XDECREF(g_logger);
    finally:
        return m;
    }

Create in the ``setup.py``:

.. code-block:: python

    Extension(name=f"{PACKAGE_NAME}.Logging.cLogging",
        include_dirs=[],
        sources=["src/cpy/Logging/cLogging.c", ],
        extra_compile_args=extra_compile_args_c,
        language='c',
    ),

And run ``python setup.py develop``.

Using and Testing
-----------------

Using From C
^^^^^^^^^^^^^

To simply use the interface defined in the above function, use it like the `printf` family of functions:

.. code-block:: c

    py_log_msg(WARNING, "error code: %d", 10);

Using From Python
^^^^^^^^^^^^^^^^^

.. code-block:: python

    from cPyExtPatt.Logging import cLogging

    cLogging.log(cLogging.ERROR, "Test log message")

There are various tests in ``tests/unit/test_c_logging.py``.
As pytest swallows logging messages there is a ``main()`` function in that script so you can run it from the command
line:

.. code-block:: python

    def main():
        logger.setLevel(logging.DEBUG)
        logger.info('main')
        logger.warning('Test warning message XXXX')
        logger.debug('Test debug message XXXX')
        logger.info('_test_logging')
        test_logging()
        print()
        print(cLogging)
        print(dir(cLogging))
        print()
        logger.info('cLogging.log():')
        cLogging.py_log_set_level(10)
        cLogging.log(cLogging.ERROR, "cLogging.log(): Test log message")

        return 0


    if __name__ == '__main__':
        exit(main())

Here is an example output:

.. code-block:: bash

    $python tests/unit/test_c_logging.py
    2025-03-07 11:49:23,994 7064 INFO     main
    2025-03-07 11:49:23,994 7064 WARNING  Test warning message XXXX
    2025-03-07 11:49:23,994 7064 DEBUG    Test debug message XXXX
    2025-03-07 11:49:23,994 7064 INFO     _test_logging
    2025-03-07 11:49:23,994 7064 WARNING  Test warning message XXXX
    2025-03-07 11:49:23,994 7064 DEBUG    Test debug message XXXX

    <module 'cPyExtPatt.Logging.cLogging' from 'PythonExtensionPatterns/cPyExtPatt/Logging/cLogging.cpython-313-darwin.so'>
    ['CRITICAL', 'DEBUG', 'ERROR', 'EXCEPTION', 'INFO', 'WARNING', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__', 'c_file_line_function', 'log', 'py_file_line_function', 'py_log_set_level']

    2025-03-07 11:49:23,994 7064 INFO     cLogging.log():
    2025-03-07 11:49:23,994 7064 ERROR    cLogging.log(): Test log message


.. _PyEval_GetFrame(): https://docs.python.org/3/c-api/reflection.html#c.PyEval_GetFrame
.. _PyFrameObject: https://docs.python.org/3/c-api/frame.html#c.PyFrameObject
.. _Frame API: https://docs.python.org/3/c-api/frame.html

.. _chapter_logging_and_frames.frames:

.. index::
    single: Frames

----------------------
Frames
----------------------

This describes how to extract the currently executing *frame* (or call stack if you wish).

.. index::
    single: Frames; Python

The Python Frame
----------------------

The crucial call is `PyEval_GetFrame()`_ that gets the current executing frame as a `PyFrameObject`_.
From Python 3.11 onwards the `PyFrameObject`_ has no public members instead there are many functions in the
`Frame API`_ to extract the data we need. Prior to that we could extract tha data directly.

The example code is in ``src/cpy/Logging/cLogging.c``.

.. code-block:: c

    /**
     * Returns a tuple of the file, line and function of the current Python frame.
     * Returns (None, 0, None) on failure.
     * @param _unused_module
     * @return PyObject *, a tuple of three values.
     */
    static PyObject *
    py_file_line_function(PyObject *Py_UNUSED(module)) {
        const unsigned char *file_name = NULL;
        const char *func_name = NULL;
        int line_number = 0;

        PyFrameObject *frame = PyEval_GetFrame();
        if (frame) {
            /* Get the file name. */
    #if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
            /* See:
             * https://docs.python.org/3.11/whatsnew/3.11.html#pyframeobject-3-11-hiding
             */
            file_name = PyUnicode_1BYTE_DATA(PyFrame_GetCode(frame)->co_filename);
    #else
            file_name = PyUnicode_1BYTE_DATA(frame->f_code->co_filename);
    #endif // PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
            /* Get the line number. */
            line_number = PyFrame_GetLineNumber(frame);
            /* Get the function name. */
    #if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
            /* See:
             * https://docs.python.org/3.11/whatsnew/3.11.html#pyframeobject-3-11-hiding
             */
            func_name = (const char *) PyUnicode_1BYTE_DATA(PyFrame_GetCode(frame)->co_name);
    #else
            func_name = (const char *) PyUnicode_1BYTE_DATA(frame->f_code->co_name);
    #endif // PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
        }
        /* Build the return value. */
        /* Use 'z' that makes Python None if the string is NULL. */
        return Py_BuildValue("ziz", file_name, line_number, func_name);
    }

And this function is registered thus:

.. code-block:: c

    static PyMethodDef logging_methods[] = {
        /* ... */
        {
            "py_file_line_function",
            (PyCFunction) py_file_line_function,
            METH_NOARGS,
            "Return the file, line and function name from the current Python frame."
        },
        /* ... */
        {NULL, NULL, 0, NULL} /* Sentinel */
    };

And can be used thus (test code is in ``tests/unit/test_c_logging.py``):

.. code-block:: python

    from cPyExtPatt.Logging import cLogging

    def test_py_file_line_function_file():
        file, _line, _function = cLogging.py_file_line_function()
        assert file == __file__


    def test_py_file_line_function_line():
        _file, line, _function = cLogging.py_file_line_function()
        assert line == 50


    def test_py_file_line_function_function():
        _file, _line, function = cLogging.py_file_line_function()
        assert function == 'test_py_file_line_function_function'

.. index::
    single: Frames; C

The C "Frame"
-----------------

It is also useful to be able to extract the C "frame" (actually the code location at compile time)
and present that as the same kind of Python tuple.

A simple macro helps:

.. code-block:: c

    #define C_FILE_LINE_FUNCTION Py_BuildValue("sis", __FILE__, __LINE__, __FUNCTION__)

And is used thus:

.. code-block:: c

    static PyObject *
    c_file_line_function(PyObject *Py_UNUSED(module)) {
        return C_FILE_LINE_FUNCTION;
    }

Teh function is registered thus:

.. code-block:: c

    static PyMethodDef logging_methods[] = {
        /* ... */
        {
            "c_file_line_function",
            (PyCFunction) c_file_line_function,
            METH_NOARGS,
            "Return the file, line and function name from the current C code."
        },
        /* ... */
        {NULL, NULL, 0, NULL} /* Sentinel */
    };

And can be used thus (test code is in ``tests/unit/test_c_logging.py``):

.. code-block:: python

    def test_c_file_line_function_file():
        file, line, function = cLogging.c_file_line_function()
        assert file == 'src/cpy/Logging/cLogging.c'
        assert line == 148
        assert function == 'c_file_line_function'
