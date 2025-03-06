// Example of using Python's logging mechanism from C.
// Based on, and thanks to, an initial submission from https://github.com/nnathan
// See also https://docs.python.org/3/library/logging.html

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

/* This modules globals */
static PyObject *g_logging_module = NULL;
static PyObject *g_logger = NULL;

/* Get a logger object from the logging module. */
static PyObject *py_get_logger(char *logger_name) {
    assert(g_logging_module);
    PyObject *logger = NULL;

    logger = PyObject_CallMethod(g_logging_module, "getLogger", "s", logger_name);
    if (logger == NULL) {
        const char *err_msg = "failed to call logging.getLogger";
        PyErr_SetString(PyExc_RuntimeError, err_msg);
    }
    return logger;
}

/* main interface to logging function */
static PyObject *
py_log_msg(int log_level, char *printf_fmt, ...) {
    assert(g_logger);
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
        return ret;
    }
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
}

/**
 * Returns a tuple of the file, line and function of the current Python frame.
 * Returns (None, 0, None) on failure.
 * @param _unused_module
 * @return
 */
static PyObject *
py_file_line_function(PyObject *Py_UNUSED(module)) {
    const unsigned char *file_name = NULL;
    const char *func_name = NULL;
    int line_number = 0;

    PyFrameObject *frame = PyEval_GetFrame();
    if (frame) {
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
        /* See https://docs.python.org/3.11/whatsnew/3.11.html#pyframeobject-3-11-hiding */
        file_name = PyUnicode_1BYTE_DATA(PyFrame_GetCode(frame)->co_filename);
#else
        file_name = PyUnicode_1BYTE_DATA(frame->f_code->co_filename);
#endif // PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
        line_number = PyFrame_GetLineNumber(frame);
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
        /* See https://docs.python.org/3.11/whatsnew/3.11.html#pyframeobject-3-11-hiding */
        func_name = (const char *) PyUnicode_1BYTE_DATA(PyFrame_GetCode(frame)->co_name);
#else
        func_name = (const char *) PyUnicode_1BYTE_DATA(frame->f_code->co_name);
#endif // PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
    }
    /* Use 'z' that makes Python None if the string is NULL. */
    return Py_BuildValue("ziz", file_name, line_number, func_name);
}

#define C_FILE_LINE_FUNCTION Py_BuildValue("sis", __FILE__, __LINE__, __FUNCTION__)

static PyObject *
c_file_line_function(PyObject *Py_UNUSED(module)) {
    return C_FILE_LINE_FUNCTION;
}

static PyMethodDef logging_methods[] = {
        {
                "log",
                (PyCFunction) py_log_message,
                METH_VARARGS,
                "Log a message."
        },
        {
                "py_file_line_function",
                (PyCFunction) py_file_line_function,
                METH_NOARGS,
                "Return the file, line and function name from the current Python frame."
        },
        {
                "c_file_line_function",
                (PyCFunction) c_file_line_function,
                METH_NOARGS,
                "Return the file, line and function name from the current C code."
        },
        {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef cLogging = {
        PyModuleDef_HEAD_INIT,
        .m_name = "cLogging",
        .m_doc = "Logging module.",
        .m_size = -1,
        .m_methods = logging_methods,
};

PyMODINIT_FUNC PyInit_cLogging(void) {
    PyObject *m = PyModule_Create(&cLogging);
    if (!m) {
        goto except;
    }
    g_logging_module = PyImport_ImportModule("logging");
    if (!g_logging_module) {
        const char *err_msg = "failed to import 'logging'";
        PyErr_SetString(PyExc_ImportError, err_msg);
        goto except;
    }
    g_logger = py_get_logger("cLogging");
    if (!g_logger) {
        goto except;
    }
    /* Adding module globals */
    /* logging levels defined by logging module. */
    if (PyModule_AddIntConstant(m, "DEBUG", LOGGING_DEBUG)) {
        goto except;
    }
    if (PyModule_AddIntConstant(m, "INFO", LOGGING_INFO)) {
        goto except;
    }
    if (PyModule_AddIntConstant(m, "WARNING", LOGGING_WARNING)) {
        goto except;
    }
    if (PyModule_AddIntConstant(m, "ERROR", LOGGING_ERROR)) {
        goto except;
    }
    if (PyModule_AddIntConstant(m, "CRITICAL", LOGGING_CRITICAL)) {
        goto except;
    }
    if (PyModule_AddIntConstant(m, "EXCEPTION", LOGGING_EXCEPTION)) {
        goto except;
    }

    goto finally;
    except:
    /* abnormal cleanup */
    /* cleanup logger references */
    Py_XDECREF(g_logging_module);
    Py_XDECREF(g_logger);
    finally:
    return m;
}
