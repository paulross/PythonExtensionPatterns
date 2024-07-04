//
//  cParseArgs.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 08/05/2014.
//  Copyright (c) 2014 Paul Ross. All rights reserved.
//

#include "Python.h"

#include "time.h"

#define FPRINTF_DEBUG 0

/****************** Parsing arguments. ****************/
static PyObject *parse_no_args(PyObject *Py_UNUSED(module)) {
#if FPRINTF_DEBUG
    PyObject_Print(module, stdout, 0);
    fprintf(stdout, "\nparse_no_args()\n");
#endif
    Py_RETURN_NONE;
}

static PyObject *parse_one_arg(PyObject *Py_UNUSED(module), PyObject *Py_UNUSED(arg)) {
#if FPRINTF_DEBUG
    PyObject_Print(module, stdout, 0);
    fprintf(stdout, "\nparse_one_arg(): ");
    PyObject_Print(arg, stdout, 0);
    fprintf(stdout, "\n");
#endif
    /* Your code here...*/
    Py_RETURN_NONE;
}

/** Example of a METH_VARGS function that takes a bytes object and int and an optional string.
 * Returns the number of arguments parsed.
 *
 * Signature is:
 *
 * def parse_args(a: bytes, b: int, c: str = '') -> int:
 * */
static PyObject *parse_args(PyObject *Py_UNUSED(module), PyObject *args) {
    PyObject *arg0 = NULL;
    int arg1;
    char *str = "";

#if FPRINTF_DEBUG
    PyObject_Print(module, stdout, 0);
    fprintf(stdout, "\nparse_args(): ");
    PyObject_Print(args, stdout, 0);
    fprintf(stdout, "\n");
#endif

    if (!PyArg_ParseTuple(args, "Si|s", &arg0, &arg1, &str)) {
        return NULL;
    }
    /* Your code here...*/

    /* PyTuple_Size returns a Py_ssize_t */
    return Py_BuildValue("n", PyTuple_Size(args));
}


/** This takes a Python object, 'sequence', that supports the sequence protocol and, optionally, an integer, 'count'.
 * This returns a new sequence which is the old sequence multiplied by the count.
 *
 * NOTE: If count is absent entirely then an empty sequence of given type is returned as count is assumed zero as
 * optional.
 * */
static PyObject *
parse_args_kwargs(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwargs) {
    PyObject *ret = NULL;
    PyObject *py_sequence = NULL;
    int count;
    static char *kwlist[] = {
            "sequence", /* bytes object. */
            "count", /* Python int converted to a C int. */
            NULL,
    };

#if FPRINTF_DEBUG
    PyObject_Print(module, stdout, 0);
    fprintf(stdout, "\n");
    PyObject_Print(args, stdout, 0);
    fprintf(stdout, "\n");
    PyObject_Print(kwargs, stdout, 0);
    fprintf(stdout, "\n");
#endif

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|i", kwlist, &py_sequence, &count)) {
        goto except;
    }

    /* Your code here...*/
    ret = PySequence_Repeat(py_sequence, count);
    if (ret == NULL) {
        goto except;
    }
    assert(!PyErr_Occurred());
    goto finally;
    except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    ret = NULL;
    finally:
    return ret;
}

/** Checks that a list is full of Python integers. */
int sum_list_of_longs(PyObject *list_longs, void *address) {
    PyObject *item = NULL;

    if (!list_longs || !PyList_Check(list_longs)) { /* Note: PyList_Check allows sub-types. */
        PyErr_Format(PyExc_TypeError, "check_list_of_longs(): First argument is not a list");
        return 0;
    }
    long result = 0L;
    for (Py_ssize_t i = 0; i < PyList_GET_SIZE(list_longs); ++i) {
        item = PyList_GetItem(list_longs, i);
        if (!PyLong_CheckExact(item)) {
            PyErr_Format(PyExc_TypeError, "check_list_of_longs(): Item %d is not a Python integer.", i);
            return 0;
        }
        /* PyLong_AsLong() must always succeed because of check above. */
        result += PyLong_AsLong(item);
    }
    long *p_long = (long *) address;
    *p_long = result;
    return 1; /* Success. */
}

/** Parse the args where we are expecting a single argument that must be a
 * list of numbers.
 *
 * This returns the sum of the numbers as a Python integer.
 *
 * This illustrates the use of "O&" in parsing.
 */
static PyObject *parse_args_with_function_conversion_to_c(PyObject *Py_UNUSED(module), PyObject *args) {
    PyObject *ret = NULL;
    long result;

#if FPRINTF_DEBUG
    PyObject_Print(module, stdout, 0);
    fprintf(stdout, "\n");
    PyObject_Print(args, stdout, 0);
    fprintf(stdout, "\n");
#endif

    if (!PyArg_ParseTuple(args, "O&", sum_list_of_longs, &result)) {
        /* NOTE: If check_list_of_numbers() returns 0 an error should be set. */
        assert(PyErr_Occurred());
        goto except;
    }

    /* Your code here...*/
    ret = PyLong_FromLong(result);
    if (ret == NULL) {
        goto except;
    }
    assert(!PyErr_Occurred());
    goto finally;
    except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    ret = NULL;
    finally:
    return ret;
}

/** Parse the args where we are simulating immutable defaults of a string and a tuple.
 * The defaults are: "Hello world", ("Answer", 42)
 *
 * This returns both arguments as a tuple.
 *
 * This imitates the Python way of handling defaults.
 */
static PyObject *parse_args_with_immutable_defaults(PyObject *Py_UNUSED(module),
                                                    PyObject *args) {
    PyObject *ret = NULL;
    /* Pointers to the default arguments, initialised below. */
    static PyObject *pyObjDefaultArg_0;
    static PyObject *pyObjDefaultArg_1;
    /* These pointers are the ones we use in the body of the function, they
     * either point at the supplied argument or the default (static) argument.
     * We treat these as "borrowed" references and so incref and decref them
     * appropriately.
     */
    PyObject *pyObjArg_0 = NULL;
    PyObject *pyObjArg_1 = NULL;
    int have_inc_ref_arguments = 0;

    /* Set defaults for arguments. */
    if (!pyObjDefaultArg_0) {
        pyObjDefaultArg_0 = PyUnicode_FromString("Hello world");
        if (!pyObjDefaultArg_0) {
            PyErr_SetString(PyExc_RuntimeError, "Can not create string!");
            goto except;
        }
    }
    if (!pyObjDefaultArg_1) {
        pyObjDefaultArg_1 = PyTuple_New(2);
        if (!pyObjDefaultArg_1) {
            PyErr_SetString(PyExc_RuntimeError, "Can not create tuple!");
            goto except;
        }
        if (PyTuple_SetItem(pyObjDefaultArg_1, 0, PyUnicode_FromString("Answer"))) {
            PyErr_SetString(PyExc_RuntimeError, "Can not set tuple[0]!");
            goto except;
        }
        if (PyTuple_SetItem(pyObjDefaultArg_1, 1, PyLong_FromLong(42))) {
            PyErr_SetString(PyExc_RuntimeError, "Can not set tuple[1]!");
            goto except;
        }
    }

    if (!PyArg_ParseTuple(args, "|OO", &pyObjArg_0, &pyObjArg_1)) {
        goto except;
    }
    /* If optional arguments absent then switch to defaults. */
    if (!pyObjArg_0) {
        pyObjArg_0 = pyObjDefaultArg_0;
    }
    /* Borrowed reference. */
    Py_INCREF(pyObjArg_0);
    if (!pyObjArg_1) {
        pyObjArg_1 = pyObjDefaultArg_1;
    }
    /* Borrowed reference. */
    Py_INCREF(pyObjArg_1);
    have_inc_ref_arguments = 1;

#if FPRINTF_DEBUG
    fprintf(stdout, "pyObjArg0 was: ");
    PyObject_Print(pyObjArg_0, stdout, 0);
    fprintf(stdout, "\n");
    fprintf(stdout, "pyObjArg1 was: ");
    PyObject_Print(pyObjArg_1, stdout, 0);
    fprintf(stdout, "\n");
#endif

    /* Your code here...*/

    /* In this case we just return the arguments so we keep the incremented (borrowed) references.
     * If the were not being returned then the borrowed references must be decremented:
     * Py_XDECREF(pyObjArg_0);
     * Py_XDECREF(pyObjArg_1);
     * Here we use have_inc_ref_arguments to decide if we are entering except with incremented borrowed references.
     */
    ret = Py_BuildValue("OO", pyObjArg_0, pyObjArg_1);
    if (ret == NULL) {
        goto except;
    }
    assert(!PyErr_Occurred());
    goto finally;
    except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    /* Error, so decrement borrowed references if have_inc_ref_arguments. */
    if (have_inc_ref_arguments) {
        Py_XDECREF(pyObjArg_0);
        Py_XDECREF(pyObjArg_1);
    }
    ret = NULL;
    finally:
    return ret;
}

/** Parse the args where we are simulating mutable default of an empty list.
 *
 * Signature is:
 *
 * parse_args_with_mutable_defaults(obj, default_list=[])
 *
 * This adds the object to the list and returns None.
 *
 * This imitates the Python way of handling defaults.
 */
static PyObject *parse_args_with_mutable_defaults(PyObject *Py_UNUSED(module),
                                                  PyObject *args) {
    PyObject *ret = NULL;
    /* Pointers to the non-default argument, initialised by PyArg_ParseTuple below. */
    PyObject *pyObjArg_0 = NULL;
    /* Pointers to the default argument, initialised below. */
    static PyObject *pyObjDefaultArg_1 = NULL;
    /* Set defaults for argument 1. */
    if (!pyObjDefaultArg_1) {
        pyObjDefaultArg_1 = PyList_New(0);
    }
    /* This pointer is the one we use in the body of the function, it
     * either points at the supplied argument or the default (static) argument.
     * We treat this as "borrowed" references and so incref and decref them
     * appropriately.
     * NOTE: We use a flag arg_1_ref_incremented to determine if we need to decrement the refcount of pyObjArg_1.
     */
    PyObject *pyObjArg_1 = NULL;
//    /* Flag to say that we have incremented the borrowed reference. Used during error handling. */
//    int ref_inc_arg_1 = 0;

    if (!PyArg_ParseTuple(args, "O|O", &pyObjArg_0, &pyObjArg_1)) {
        goto except;
    }
    /* If optional argument absent then switch to defaults. */
    if (!pyObjArg_1) {
        pyObjArg_1 = pyObjDefaultArg_1;
    }
//    /* This increments the default or the given argument. */
//    Py_INCREF(pyObjArg_1);
//    ref_inc_arg_1 = 1;

#if FPRINTF_DEBUG
    fprintf(stdout, "pyObjArg1 was: ");
    PyObject_Print(pyObjArg_1, stdout, 0);
    fprintf(stdout, "\n");
#endif

    /* Your code here...*/
    /* Append the first argument to the second. */
    if (PyList_Append(pyObjArg_1, pyObjArg_0)) {
        PyErr_SetString(PyExc_RuntimeError, "Can not append to list!");
        goto except;
    }

#if FPRINTF_DEBUG
    fprintf(stdout, "pyObjArg1 now: ");
    PyObject_Print(pyObjArg_1, stdout, 0);
    fprintf(stdout, "\n");
#endif

    /* Success. */
    assert(!PyErr_Occurred());
    /* This increments the default or the given argument. */
    Py_INCREF(pyObjArg_1);
    ret = pyObjArg_1;
    goto finally;
except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    ret = NULL;
finally:
//    if (ref_inc_arg_1) {
//        Py_XDECREF(pyObjArg_1);
//    }
    return ret;
}

static char parse_args_kwargs_docstring[] =
        "Some documentation for this function.";

static PyMethodDef cParseArgs_methods[] = {
        {"parse_no_args",                            (PyCFunction) parse_no_args,                            METH_NOARGS,  "No arguments."},
        {"parse_one_arg",                            (PyCFunction) parse_one_arg,                            METH_O,       "One argument."},
        {"parse_args",                               (PyCFunction) parse_args,                               METH_VARARGS, "Reads args only."},
        {"parse_args_kwargs",                        (PyCFunction) parse_args_kwargs,                        METH_VARARGS |
                                                                                                             METH_KEYWORDS, parse_args_kwargs_docstring},
        {"parse_args_with_function_conversion_to_c", (PyCFunction) parse_args_with_function_conversion_to_c, METH_VARARGS,
                                                                                                                           "Parsing an argument that must be a list of numbers."},
        {"parse_args_with_immutable_defaults",       (PyCFunction) parse_args_with_immutable_defaults,
                                                                                                             METH_VARARGS, "A function with mutable defaults."},
        {"parse_args_with_mutable_defaults",         (PyCFunction) parse_args_with_mutable_defaults,
                                                                                                             METH_VARARGS, "A function with mutable defaults."},
        {NULL, NULL, 0,                                                                                                     NULL} /* Sentinel */
};

static PyModuleDef cParseArgs_module = {
        PyModuleDef_HEAD_INIT,
        "cParseArgs",
        "Examples of parsing arguments in a Python 'C' extension.",
        -1,
        cParseArgs_methods,
        NULL, /* inquiry m_reload */
        NULL, /* traverseproc m_traverse */
        NULL, /* inquiry m_clear */
        NULL, /* freefunc m_free */
};

PyMODINIT_FUNC PyInit_cParseArgs(void) {
    return PyModule_Create(&cParseArgs_module);
}
/****************** END: Parsing arguments. ****************/
