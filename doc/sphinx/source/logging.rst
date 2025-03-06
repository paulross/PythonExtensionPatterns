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

.. code-block:: c

    #include <Python.h>
    #include <stdarg.h>

    /* logging levels defined by logging module
     * NOTE: In Python logging FATAL = CRITICAL */
    enum { LOGGING_INFO, LOGGING_WARNING, LOGGING_ERROR, LOGGING_FATAL, LOGGING_DEBUG, LOGGING_EXCEPTION };

    /* module globals */
    static PyObject *g_logging_import = NULL;
    static PyObject *g_logger = NULL;

    /* Get a logger object from the logging module. */
    static PyObject *py_get_logger(char *logger_name) {
        PyObject *logger = NULL;

        logger = PyObject_CallMethod(g_logging_import, "getLogger", "s", logger_name);
        if (logger == NULL) {
            const char *err_msg = "failed to call logging.getLogger";
            PyErr_SetString(PyExc_RuntimeError, err_msg);
        }
        return logger;
    }

    /* main interface to logging function */
    static PyObject *
    py_log_msg(int log_level, char *printf_fmt, ...) {
        PyObject *log_msg = NULL;
        PyObject *ret = NULL;
        va_list fmt_args;

        va_start(fmt_args, printf_fmt);
        log_msg = PyUnicode_FromFormatV(printf_fmt, fmt_args);
        va_end(fmt_args);
        if (log_msg == NULL) {
            /* fail silently. */
            return ret;
        }
        /* call function depending on loglevel */
        switch (log_level) {
            case LOGGING_INFO:
                ret = PyObject_CallMethod(g_logger, "info", "O", log_msg);
                break;
            case LOGGING_WARNING:
                ret = PyObject_CallMethod(g_logger, "warning", "O", log_msg);
                break;
            case LOGGING_ERROR:
                ret = PyObject_CallMethod(g_logger, "error", "O", log_msg);
                break;
            case LOGGING_FATAL:
                ret = PyObject_CallMethod(g_logger, "fatal", "O", log_msg);
                break;
            case LOGGING_DEBUG:
                ret = PyObject_CallMethod(g_logger, "debug", "O", log_msg);
                break;
            case LOGGING_EXCEPTION:
                ret = PyObject_CallMethod(g_logger, "exception", "O", log_msg);
                break;
            default:
                break;
        }
        Py_DECREF(log_msg);
        return ret;
    }

    static PyObject *
    py_log_message(PyObject *Py_UNUSED(module), PyObject *args) {
        int log_level;
        char *message;

        if (!PyArg_ParseTuple(args, "iz", &log_level, &message)) {
            return NULL;
        }
        return py_log_msg(log_level, "%s", message);
    //    Py_RETURN_NONE;
    }

    static PyMethodDef logging_methods[] = {
            {
                    "log",
                    (PyCFunction) py_log_message,
                    METH_VARARGS,
                    "Log a message."
            },
            {NULL, NULL, 0, NULL} /* Sentinel */
    };

    static PyModuleDef cLogging = {
            PyModuleDef_HEAD_INIT,
            .m_name = "cLogging",
            .m_doc = "Logging mmodule.",
            .m_size = -1,
            .m_methods = logging_methods,
    };

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

To simply use the interface defined in the above function, use it like the `printf` family of functions:

.. code-block:: c

    py_log_msg(WARNING, "error code: %d", 10);

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
