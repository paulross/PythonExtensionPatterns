//
//  cParseArgsHelper.cpp
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 07/07/2024.
//  Copyright (c) 2024 Paul Ross. All rights reserved.
//
// NOTE: For some reason when reformatting this file as a *.cpp file the formatting goes horribly wrong.
// The solution is to comment out #include "Python.h" before reformatting.

// NOTE: This is legacy code.
// In version 0.1.0 of this project the documentation described some helper techniques for handling default arguments.
// The section titles were "Simplifying Macros" and "Simplifying C++11 class'.
// The code samples in the document were incorrect, this is the correct version(s) of that concept.
// However these helper techniques don't really help very much as the defaults have to be PyObjects and
// subsequently converted to C types.
// This just introduces another layer of abstraction for no real gain.
// For those reasons the documentation sections were removed from version 0.2.0.
// This code remains incase it is of interest.

#define PY_SSIZE_T_CLEAN

#include "Python.h"

/****************** Parsing arguments. ****************/


/* Helper macros. */
#define PY_DEFAULT_ARGUMENT_INIT(name, value, ret)          \
    PyObject *name = NULL;                                  \
    static PyObject *default_##name = NULL;                 \
    if (! default_##name) {                                 \
        default_##name = value;                             \
        if (! default_##name) {                             \
            PyErr_SetString(                                \
                PyExc_RuntimeError,                         \
                "Can not create default value for " #name   \
            );                                              \
            return ret;                                     \
        }                                                   \
    }

#define PY_DEFAULT_ARGUMENT_SET(name) \
    if (! name) {                     \
        name = default_##name;        \
    }                                 \
    Py_INCREF(name)

static PyObject*
parse_defaults_with_helper_macro(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    PyObject *ret = NULL;
    /* Initialise default arguments. Note: these might cause an early return. */
    PY_DEFAULT_ARGUMENT_INIT(encoding,  PyUnicode_FromString("utf-8"),  NULL);
    PY_DEFAULT_ARGUMENT_INIT(the_id,    PyLong_FromLong(0L),            NULL);
    PY_DEFAULT_ARGUMENT_INIT(must_log,  PyBool_FromLong(1L),            NULL);

    static const char *kwlist[] = { "encoding", "the_id", "must_log", NULL };
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|OOO",
                                      const_cast<char**>(kwlist),
                                      &encoding, &the_id, &must_log)) {
        goto except;
    }
    /*
     * Assign absent arguments to defaults and increment the reference count.
     * Don't forget to decrement the reference count before returning!
     */
    PY_DEFAULT_ARGUMENT_SET(encoding);
    PY_DEFAULT_ARGUMENT_SET(the_id);
    PY_DEFAULT_ARGUMENT_SET(must_log);

    /*
     * Use encoding: Python str, the_id: C long, must_log from here on...
     */

    Py_INCREF(encoding);
    Py_INCREF(the_id);
    Py_INCREF(must_log);
    ret = Py_BuildValue("OOO", encoding, the_id, must_log);
    assert(! PyErr_Occurred());
    assert(ret);
    goto finally;
except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    ret = NULL;
finally:
    Py_DECREF(encoding);
    Py_DECREF(the_id);
    Py_DECREF(must_log);
    return ret;
}

/* Helper classes. */

/** Class to simplify default arguments.
 *
 * Usage:
 *
 * static DefaultArg arg_0(PyLong_FromLong(1L));
 * static DefaultArg arg_1(PyUnicode_FromString("Default string."));
 * if (! arg_0 || ! arg_1) {
 *      return NULL;
 * }
 *
 * if (! PyArg_ParseTupleAndKeywords(args, kwargs, "...",
                                     const_cast<char**>(kwlist),
                                     &arg_0, &arg_1, ...)) {
        return NULL;
    }
 *
 * Then just use arg_0, arg_1 as if they were a PyObject* (possibly
 * might need to be cast to some specific PyObject*).
 *
 * WARN: This class is designed to be statically allocated. If allocated
 * on the heap or stack it will leak memory. That could be fixed by
 * implementing:
 *
 * ~DefaultArg() { Py_XDECREF(m_default); }
 *
 * But this will be highly dangerous when statically allocated as the
 * destructor will be invoked with the Python interpreter in an
 * uncertain state and will, most likely, segfault:
 * "Python(39158,0x7fff78b66310) malloc: *** error for object 0x100511300: pointer being freed was not allocated"
 */
class DefaultArg {
public:
    DefaultArg(PyObject *new_ref) : m_arg(NULL), m_default(new_ref) {}
    /// Allow setting of the (optional) argument with
    /// PyArg_ParseTupleAndKeywords
    PyObject **operator&() { m_arg = NULL; return &m_arg; }
    /// Access the argument or the default if default.
    operator PyObject *() const {
        return m_arg ? m_arg : m_default;
    }
    PyObject *obj() const {
        return m_arg ? m_arg : m_default;
    }
    /// Test if constructed successfully from the new reference.
    explicit operator bool() { return m_default != NULL; }
protected:
    PyObject *m_arg;
    PyObject *m_default;
};

static PyObject*
parse_defaults_with_helper_class(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    PyObject *ret = NULL;
    /* Initialise default arguments. */
    static DefaultArg encoding(PyUnicode_FromString("utf-8"));
    static DefaultArg the_id(PyLong_FromLong(0L));
    static DefaultArg must_log(PyBool_FromLong(1L));

    /* Check that the defaults are non-NULL i.e. succesful. */
    if (!encoding || !the_id || !must_log) {
        return NULL;
    }

    static const char *kwlist[] = { "encoding", "the_id", "must_log", NULL };
    /* &encoding etc. accesses &m_arg in DefaultArg because of PyObject **operator&() */
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|OOO",
                                      const_cast<char**>(kwlist),
                                      &encoding, &the_id, &must_log)) {
        return NULL;
    }
    /*
     * Use encoding, the_id, must_log from here on as PyObject* since we have
     * operator PyObject*() const ...
     *
     * So if we have a function:
     * set_encoding(PyObject *obj) { ... }
     */
//    set_encoding(encoding);
    /* ... */
    Py_INCREF(encoding.obj());
    Py_INCREF(the_id.obj());
    Py_INCREF(must_log.obj());
    ret = Py_BuildValue("OOO", encoding.obj(), the_id.obj(), must_log.obj());
    return ret;
}


static PyMethodDef cParseArgsHelper_methods[] = {
        {
                "parse_defaults_with_helper_macro",
                      (PyCFunction) parse_defaults_with_helper_macro,
                            METH_VARARGS,
                               "A function with immutable defaults."
        },
        {
                "parse_defaults_with_helper_class",
                      (PyCFunction) parse_defaults_with_helper_class,
                            METH_VARARGS,
                               "A function with mutable defaults."
        },
        {       NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef cParseArgsHelper_module = {
        PyModuleDef_HEAD_INIT,
        "cParseArgsHelper",
        "Examples of helper macros and classes when parsing arguments in a Python C/C++ extension.",
        -1,
        cParseArgsHelper_methods,
        NULL, /* inquiry m_reload */
        NULL, /* traverseproc m_traverse */
        NULL, /* inquiry m_clear */
        NULL, /* freefunc m_free */
};

PyMODINIT_FUNC PyInit_cParseArgsHelper(void) {
    return PyModule_Create(&cParseArgsHelper_module);
}
/****************** END: Parsing arguments. ****************/
