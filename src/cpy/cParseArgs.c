//
//  cParseArgs.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 08/05/2014.
//  Copyright (c) 2014 Paul Ross. All rights reserved.
//

#include "Python.h"

#include "time.h"

/****************** Parsing arguments. ****************/
static PyObject *parse_no_args(PyObject *module) {
    PyObject_Print(module, stdout, 0);
    fprintf(stdout, "\nparse_no_args()\n");
    Py_RETURN_NONE;
}

static PyObject *parse_one_arg(PyObject *module, PyObject *arg) {
    PyObject_Print(module, stdout, 0);
    fprintf(stdout, "\nparse_one_arg(): ");
    PyObject_Print(arg, stdout, 0);
    fprintf(stdout, "\n");
    /* Your code here...*/
    Py_RETURN_NONE;
}

// TODO
static PyObject *parse_args(PyObject *module, PyObject *args) {
    PyObject *pyStr = NULL;
    int arg1, arg2;

    PyObject_Print(module, stdout, 0);
    fprintf(stdout, "\n");
    fprintf(stdout, "\nparse_args(): ");
    PyObject_Print(args, stdout, 0);
    fprintf(stdout, "\n");

    if (!PyArg_ParseTuple(args, "Si|i", &pyStr, &arg1, &arg2)) {
        return NULL;
    }
    /* Your code here...*/
    Py_RETURN_NONE;
}

static PyObject *parse_args_kwargs(PyObject *module, PyObject *args,
                                   PyObject *kwargs) {
    PyObject *ret = NULL;
    PyObject *pyStr = NULL;
    int arg2;
    static char *kwlist[] = {"argOne", /* bytes object. */
                             "argTwo", NULL};

    PyObject_Print(module, stdout, 0);
    fprintf(stdout, "\n");
    PyObject_Print(args, stdout, 0);
    fprintf(stdout, "\n");
    PyObject_Print(kwargs, stdout, 0);
    fprintf(stdout, "\n");

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "S|i", kwlist, &pyStr,
                                     &arg2)) {
        goto except;
    }

    /* Your code here...*/

    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
    except:
    Py_XDECREF(ret);
    ret = NULL;
    finally:
    return ret;
}

/** Checks that a list is full of numbers. */
int check_list_of_numbers(PyObject *lst, void *Py_UNUSED(address)) {
    PyObject *item = NULL;

    if (!lst || !PyList_Check(lst)) { /* Note: PyList_Check allows sub-types. */
        return 0;
    }
    for (Py_ssize_t i = 0; i < PyList_GET_SIZE(lst); ++i) {
        item = PyList_GetItem(lst, i);
        if (!(PyLong_CheckExact(item) || PyFloat_CheckExact(item) ||
              PyComplex_CheckExact(item))) {
            PyErr_Format(PyExc_ValueError, "Item %d is not a number.", i);
            return 0;
        }
    }
    return 1; /* Success. */
}

/** Parse the args where we are expecting a single argument that must be a
 * list of numbers.
 *
 * This illustrates the use of "O&" in parsing.
 */
static PyObject *parse_args_with_checking(PyObject *module, PyObject *args) {
    PyObject *ret = NULL;
    PyObject *pyObj = NULL;

    PyObject_Print(module, stdout, 0);
    fprintf(stdout, "\n");
    PyObject_Print(args, stdout, 0);
    fprintf(stdout, "\n");

    if (!PyArg_ParseTuple(args, "O&", check_list_of_numbers, &pyObj)) {
        goto except;
    }

    /* Your code here...*/

    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
    except:
    Py_XDECREF(ret);
    ret = NULL;
    finally:
    return ret;
}

/* Parse the args where we are simulating immutable defaults of a string and a
 * tuple.
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
        if (PyTuple_SetItem(pyObjDefaultArg_1, 0, PyLong_FromLong(42))) {
            PyErr_SetString(PyExc_RuntimeError, "Can not set tuple[0]!");
            goto except;
        }
        if (PyTuple_SetItem(pyObjDefaultArg_1, 1, PyUnicode_FromString("This"))) {
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
    Py_INCREF(pyObjArg_0);
    if (!pyObjArg_1) {
        pyObjArg_1 = pyObjDefaultArg_1;
    }
    Py_INCREF(pyObjArg_1);

    fprintf(stdout, "pyObjArg0 was: ");
    PyObject_Print(pyObjArg_0, stdout, 0);
    fprintf(stdout, "\n");
    fprintf(stdout, "pyObjArg1 was: ");
    PyObject_Print(pyObjArg_1, stdout, 0);
    fprintf(stdout, "\n");

    /* Your code here...*/

    /* Mutate the arguments. */

    fprintf(stdout, "pyObjArg0 now: ");
    PyObject_Print(pyObjArg_0, stdout, 0);
    fprintf(stdout, "\n");
    fprintf(stdout, "pyObjArg1 now: ");
    PyObject_Print(pyObjArg_1, stdout, 0);
    fprintf(stdout, "\n");

    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
    except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    ret = NULL;
    finally:
    Py_XDECREF(pyObjArg_0);
    Py_XDECREF(pyObjArg_1);
    return ret;
}

/* Parse the args where we are simulating mutable defaults of a list and a dict.
 * This imitates the Python way of handling defaults.
 */
static PyObject *parse_args_with_mutable_defaults(PyObject *Py_UNUSED(module),
                                                  PyObject *args) {
    PyObject *ret = NULL;
    /* Pointers to the default arguments, initialised below. */
    static PyObject *pyObjDefaultArg_0;
    static PyObject *pyObjDefaultArg_1;
    /* These pointers are the ones we use in the body of the function, they
     * either point at the supplied argument or the default (static) argument.
     * We treat these as "borrowed" references and so indref and decref them
     * appropriatly.
     */
    PyObject *pyObjArg_0 = NULL;
    PyObject *pyObjArg_1 = NULL;

    /* Set defaults for arguments. */
    if (!pyObjDefaultArg_0) {
        pyObjDefaultArg_0 = PyList_New(0);
    }
    if (!pyObjDefaultArg_1) {
        pyObjDefaultArg_1 = PyDict_New();
    }

    if (!PyArg_ParseTuple(args, "|OO", &pyObjArg_0, &pyObjArg_1)) {
        goto except;
    }
    /* If optional arguments absent then switch to defaults. */
    if (!pyObjArg_0) {
        pyObjArg_0 = pyObjDefaultArg_0;
    }
    Py_INCREF(pyObjArg_0);
    if (!pyObjArg_1) {
        pyObjArg_1 = pyObjDefaultArg_1;
    }
    Py_INCREF(pyObjArg_1);

    fprintf(stdout, "pyObjArg0 was: ");
    PyObject_Print(pyObjArg_0, stdout, 0);
    fprintf(stdout, "\n");
    fprintf(stdout, "pyObjArg1 was: ");
    PyObject_Print(pyObjArg_1, stdout, 0);
    fprintf(stdout, "\n");

    /* Your code here...*/

    /* Mutate the arguments. */
    if (PyList_Append(pyObjArg_0, PyLong_FromLong(9))) {
        PyErr_SetString(PyExc_RuntimeError, "Can not append to list!");
        goto except;
    }
    if (PyDict_SetItem(pyObjDefaultArg_1,
                       PyLong_FromLong(PyList_Size(pyObjArg_0)),
                       PyLong_FromLong(time(NULL)))) {
        PyErr_SetString(PyExc_RuntimeError, "Can not append to dict!");
        goto except;
    }

    fprintf(stdout, "pyObjArg0 now: ");
    PyObject_Print(pyObjArg_0, stdout, 0);
    fprintf(stdout, "\n");
    fprintf(stdout, "pyObjArg1 now: ");
    PyObject_Print(pyObjArg_1, stdout, 0);
    fprintf(stdout, "\n");

    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
    except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    ret = NULL;
    finally:
    Py_XDECREF(pyObjArg_0);
    Py_XDECREF(pyObjArg_1);
    return ret;
}

static char parse_args_kwargs_docstring[] =
        "Some documentation for this function.";

static PyMethodDef cParseArgs_methods[] = {
        {"parse_no_args",                      (PyCFunction) parse_no_args,            METH_NOARGS,  "No arguments."},
        {"parse_one_arg",                      (PyCFunction) parse_one_arg,            METH_O,       "One argument."},
        {"parse_args",                         (PyCFunction) parse_args,               METH_VARARGS, "Reads args only."},
        {"parse_args_kwargs",                  (PyCFunction) parse_args_kwargs,
                                                                                       METH_VARARGS |
                                                                                       METH_KEYWORDS, parse_args_kwargs_docstring},
        {"parse_args_with_checking",           (PyCFunction) parse_args_with_checking, METH_VARARGS,
                                                                                                     "Parsing an argument that must be a list of numbers."},
        {"parse_args_with_immutable_defaults", (PyCFunction) parse_args_with_immutable_defaults,
                                                                                       METH_VARARGS, "A function with mutable defaults."},
        {"parse_args_with_mutable_defaults",   (PyCFunction) parse_args_with_mutable_defaults,
                                                                                       METH_VARARGS, "A function with mutable defaults."},
        {NULL, NULL, 0,                                                                               NULL} /* Sentinel */
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
