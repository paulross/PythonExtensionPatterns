.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

=================================
Logging
=================================

This presents a recipe for using the Python logging module in C.

We import the module and define C equivalent logging functions that are
compatible with the `*printf` family.

.. code-block:: c

    #include <Python.h>
    #include <stdarg.h>

    /* logging levels defined by logging module */
    enum { INFO, WARNING, ERROR, DEBUG, EXCEPTION };

    /* module globals */
    static PyObject *logging_import = NULL;
    static PyObject *logger = NULL;

    /* Get a logger object from the logging module. */
    static PyObject *py_get_logger(char *logger_name)
    {
        PyObject *logger = NULL;
        PyObject *ret = NULL;

        logger = PyObject_CallMethod(logging_import, "getLogger", "s", logger_name);
        if (logger == NULL)
        {
            const char *err_msg = "failed to call logging.getLogger";
            PyErr_SetString(PyExc_RuntimeError, err_msg);
        }

        return logger;
    }

    /* main interface to logging function */
    static void py_log_msg(int log_level, char *printf_fmt, ...)
    {
        PyObject *log_msg = NULL;
        va_list fmt_args;

        va_start(fmt_args, printf_fmt);
        log_msg = PyUnicode_FromFormatV(printf_fmt, fmt_args);
        va_end(fmt_args);

        if (log_msg == NULL)
        {
            /* fail silently. */
            return;
        }

        /* call function depending on loglevel */
        switch (log_level)
        {
            case INFO:
                PyObject_CallMethod(PyLogger, "info", "O", log_msg);
                break;

            case WARNING:
                PyObject_CallMethod(PyLogger, "warn", "O", log_msg);
                break;

            case ERROR:
                PyObject_CallMethod(PyLogger, "error", "O", log_msg);
                break;

            case DEBUG:
                PyObject_CallMethod(PyLogger, "debug", "O", log_msg);
                break;

            case EXCEPTION:
                PyObject_CallMethod(PyLogger, "exception", "O", log_msg);
                break;

            default:
                break;
        }

        Py_DECREF(log_msg);
    }

    /* module initialization function. */
    PyMODINIT_FUNC PyInit_interface(void)
    {
        /* ... define local variables ... */

    try:

        /* ... code to initialise module ... */

        logging_import = PyImport_ImportModule("logging");

        if (!logging_import)
        {
            const char *err_msg = "failed to import 'logging'";
            PyErr_SetString(PyExc_ImportError, err_msg);
            goto except;
        }

        logger = py_get_logger("my.module.name");

        if (!logger)
        {
            goto except;
        }

        /* ... more fabulous things ... */

    except:

        /* abnormal cleanup */

        /* cleanup logger references */
        Py_XDECREF(logging_import);
        Py_XDECREF(logger);
        ret = NULL;

    finally:

        /* ... clean up under normal conditions ... */
    }

To simply use the interface defined in the above function, use it like the `printf` family of functions:

.. code-block:: c

    py_log_msg(WARNING, "error code: %d", 10);



