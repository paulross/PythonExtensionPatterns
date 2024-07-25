// Example of using Python's logging mechanism from C.
// Based on, and thanks to, an initial submission from https://github.com/nnathan

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
