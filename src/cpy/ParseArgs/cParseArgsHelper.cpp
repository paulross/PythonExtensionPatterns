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

const long DEFAULT_ID = 1024L;
const double DEFAULT_FLOAT = 8.0;

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
    }

#define PY_DEFAULT_CHECK(name, check_function, type)    \
    if (!check_function(name)) {                        \
        PyErr_Format(                                   \
            PyExc_TypeError,                            \
            #name " must be " #type ", not \"%s\"",     \
            Py_TYPE(name)->tp_name                      \
        );                                              \
        return NULL;                                    \
    }


static PyObject *
parse_defaults_with_helper_macro(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    PyObject *ret = NULL;
    /* Initialise default arguments. Note: these might cause an early return. */
    PY_DEFAULT_ARGUMENT_INIT(encoding_m, PyUnicode_FromString("utf-8"), NULL);
    PY_DEFAULT_ARGUMENT_INIT(the_id_m, PyLong_FromLong(DEFAULT_ID), NULL);
    PY_DEFAULT_ARGUMENT_INIT(log_interval_m, PyFloat_FromDouble(DEFAULT_FLOAT), NULL);

    fprintf(stdout, "%s(): %s#%d", __FUNCTION__, __FILE_NAME__, __LINE__);
    fprintf(stdout, " default_encoding_m %p", (void *)default_encoding_m);
    if (default_encoding_m) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(default_encoding_m));
    }
    fprintf(stdout, " encoding_m %p", (void *)encoding_m);
    if (encoding_m) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(encoding_m));
    }
    fprintf(stdout, "\n");

    static const char *kwlist[] = {"encoding", "the_id", "log_interval", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOO",
                                     const_cast<char **>(kwlist),
                                     &encoding_m, &the_id_m, &log_interval_m)) {
        goto except;
    }
    /*
     * Assign absent arguments to defaults and increment the reference count.
     * Don't forget to decrement the reference count before returning!
     */
    PY_DEFAULT_ARGUMENT_SET(encoding_m);
    PY_DEFAULT_ARGUMENT_SET(the_id_m);
    PY_DEFAULT_ARGUMENT_SET(log_interval_m);

    fprintf(stdout, "%s(): %s#%d", __FUNCTION__, __FILE_NAME__, __LINE__);
    fprintf(stdout, " default_encoding_m %p", (void *)default_encoding_m);
    if (default_encoding_m) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(default_encoding_m));
    }
    fprintf(stdout, " encoding_m %p", (void *)encoding_m);
    if (encoding_m) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(encoding_m));
    }
    fprintf(stdout, "\n");

    /* Check the types of the given or default arguments. */
    PY_DEFAULT_CHECK(encoding_m, PyUnicode_Check, "str");
    PY_DEFAULT_CHECK(the_id_m, PyLong_Check, "int");
    PY_DEFAULT_CHECK(log_interval_m, PyFloat_Check, "float");

    /*
     * Use 'encoding': Python str, 'the_id': C long, 'must_log': C long from here on...
     */

    /* Py_BuildValue("O") increments the reference count. */
    ret = Py_BuildValue("OOO", encoding_m, the_id_m, log_interval_m);

    fprintf(stdout, "%s(): %s#%d", __FUNCTION__, __FILE_NAME__, __LINE__);
    fprintf(stdout, " default_encoding_m %p", (void *)default_encoding_m);
    if (default_encoding_m) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(default_encoding_m));
    }
    fprintf(stdout, " encoding_m %p", (void *)encoding_m);
    if (encoding_m) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(encoding_m));
    }
    fprintf(stdout, "\n");

    assert(!PyErr_Occurred());
    assert(ret);
    goto finally;
except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    ret = NULL;
finally:
//    Py_DECREF(encoding_m);
//    Py_DECREF(the_id_m);
//    Py_DECREF(log_interval_m);
    return ret;
}

/** Parse the args where we are simulating mutable default of an empty list.
 * This uses the helper macros.
 *
 * This is equivalent to:
 *
 *  def parse_mutable_defaults_with_helper_macro(obj, default_list=[]):
 *      default_list.append(obj)
 *      return default_list
 *
 * This adds the object to the list and returns None.
 *
 * This imitates the Python way of handling defaults.
 */
static PyObject *parse_mutable_defaults_with_helper_macro(PyObject *Py_UNUSED(module),
                                                          PyObject *args) {
    PyObject *ret = NULL;
    /* Pointers to the non-default argument, initialised by PyArg_ParseTuple below. */
    PyObject *arg_0 = NULL;
    /* Pointers to the default argument, initialised below. */
    /* Initialise default arguments. Note: these might cause an early return. */
    PY_DEFAULT_ARGUMENT_INIT(list_argument_m, PyList_New(0), NULL);

    if (!PyArg_ParseTuple(args, "O|O", &arg_0, &list_argument_m)) {
        goto except;
    }
    /* If optional argument absent then switch to defaults. */
    PY_DEFAULT_ARGUMENT_SET(list_argument_m);
    PY_DEFAULT_CHECK(list_argument_m, PyList_Check, "list");

    /* Your code here...*/

    /* Append the first argument to the second.
     * PyList_Append() increments the refcount of arg_0. */
    if (PyList_Append(list_argument_m, arg_0)) {
        PyErr_SetString(PyExc_RuntimeError, "Can not append to list!");
        goto except;
    }

    /* Success. */
    assert(!PyErr_Occurred());
    /* This increments the default or the given argument. */
    Py_INCREF(list_argument_m);
    ret = list_argument_m;
    goto finally;
except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    ret = NULL;
finally:
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
    PyObject **operator&() {
        m_arg = NULL;
        return &m_arg;
    }

    /// Access the argument or the default if default.
    operator PyObject *() const {
        return m_arg ? m_arg : m_default;
    }

    PyObject *obj() const {
        return m_arg ? m_arg : m_default;
    }

    /// Test if constructed successfully from the new reference.
    explicit operator bool() { return m_default != NULL; }

    PyObject *arg() const {
        return m_arg;
    }

    PyObject *default_arg() const {
        return m_default;
    }
protected:
    PyObject *m_arg;
    PyObject *m_default;
};

static PyObject *
parse_defaults_with_helper_class(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    PyObject *ret = NULL;
    /* Initialise default arguments. */
    static DefaultArg encoding_c(PyUnicode_FromString("utf-8"));
    static DefaultArg the_id_c(PyLong_FromLong(DEFAULT_ID));
    static DefaultArg log_interval_c(PyFloat_FromDouble(DEFAULT_FLOAT));

    fprintf(stdout, "%s(): %s#%d", __FUNCTION__, __FILE_NAME__, __LINE__);
    fprintf(stdout, " default_encoding_c %p", (void *)encoding_c.default_arg());
    if (encoding_c.default_arg()) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(encoding_c.default_arg()));
    }
    fprintf(stdout, " encoding_c %p", (void *)encoding_c.arg());
    if (encoding_c.arg()) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(encoding_c.arg()));
    }
    fprintf(stdout, "\n");

    /* Check that the defaults are non-NULL i.e. succesful. */
    if (!encoding_c || !the_id_c || !log_interval_c) {
        return NULL;
    }

    static const char *kwlist[] = {"encoding", "the_id", "log_interval", NULL};
    /* &encoding etc. accesses &m_arg in DefaultArg because of PyObject **operator&() */
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOO",
                                     const_cast<char **>(kwlist),
                                     &encoding_c, &the_id_c, &log_interval_c)) {
        return NULL;
    }

    fprintf(stdout, "%s(): %s#%d", __FUNCTION__, __FILE_NAME__, __LINE__);
    fprintf(stdout, " default_encoding_c %p", (void *)encoding_c.default_arg());
    if (encoding_c.default_arg()) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(encoding_c.default_arg()));
    }
    fprintf(stdout, " encoding_c %p", (void *)encoding_c.arg());
    if (encoding_c.arg()) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(encoding_c.arg()));
    }
    fprintf(stdout, "\n");

    PY_DEFAULT_CHECK(encoding_c, PyUnicode_Check, "str");
    PY_DEFAULT_CHECK(the_id_c, PyLong_Check, "int");
    PY_DEFAULT_CHECK(log_interval_c, PyFloat_Check, "float");

    /*
     * Use encoding, the_id, must_log from here on as PyObject* since we have
     * operator PyObject*() const ...
     *
     * So if we have a function:
     * set_encoding(PyObject *obj) { ... }
     */
//    set_encoding(encoding);
    /* ... */

    /* Py_BuildValue("O") increments the reference count. */
    ret = Py_BuildValue("OOO", encoding_c.obj(), the_id_c.obj(), log_interval_c.obj());

    fprintf(stdout, "%s(): %s#%d", __FUNCTION__, __FILE_NAME__, __LINE__);
    fprintf(stdout, " default_encoding_c %p", (void *)encoding_c.default_arg());
    if (encoding_c.default_arg()) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(encoding_c.default_arg()));
    }
    fprintf(stdout, " encoding_c %p", (void *)encoding_c.arg());
    if (encoding_c.arg()) {
        fprintf(stdout, " refcount: %zd", Py_REFCNT(encoding_c.arg()));
    }
    fprintf(stdout, "\n");

    return ret;
}


/** Parse the args where we are simulating mutable default of an empty list.
 * This uses the helper class.
 *
 * This is equivalent to:
 *
 *  def parse_mutable_defaults_with_helper_class(obj, default_list=[]):
 *      default_list.append(obj)
 *      return default_list
 *
 * This adds the object to the list and returns None.
 *
 * This imitates the Python way of handling defaults.
 */
static PyObject *parse_mutable_defaults_with_helper_class(PyObject *Py_UNUSED(module),
                                                          PyObject *args) {
    PyObject *ret = NULL;
    /* Pointers to the non-default argument, initialised by PyArg_ParseTuple below. */
    PyObject *arg_0 = NULL;
    static DefaultArg list_argument_c(PyList_New(0));

    if (!PyArg_ParseTuple(args, "O|O", &arg_0, &list_argument_c)) {
        goto except;
    }
    PY_DEFAULT_CHECK(list_argument_c, PyList_Check, "list");

    /* Your code here...*/

    /* Append the first argument to the second.
     * PyList_Append() increments the refcount of arg_0. */
    if (PyList_Append(list_argument_c, arg_0)) {
        PyErr_SetString(PyExc_RuntimeError, "Can not append to list!");
        goto except;
    }

    /* Success. */
    assert(!PyErr_Occurred());
    /* This increments the default or the given argument. */
    Py_INCREF(list_argument_c);
    ret = list_argument_c;
    goto finally;
    except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    ret = NULL;
    finally:
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
                "parse_mutable_defaults_with_helper_macro",
                (PyCFunction) parse_mutable_defaults_with_helper_macro,
                METH_VARARGS,
                "A function with a mutable argument."
        },
        {
                "parse_defaults_with_helper_class",
                (PyCFunction) parse_defaults_with_helper_class,
                METH_VARARGS,
                "A function with immutable defaults."
        },
        {
                "parse_mutable_defaults_with_helper_class",
                (PyCFunction) parse_mutable_defaults_with_helper_class,
                METH_VARARGS,
                "A function with a mutable argument."
        },
        {NULL, NULL, 0, NULL} /* Sentinel */
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
