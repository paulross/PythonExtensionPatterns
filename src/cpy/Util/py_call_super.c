//
//  py_call_super.c
//  PythonSubclassList
//
// Provides C functions to call the Python super() class.
//
//  Created by Paul Ross on 03/05/2016.
//  Copyright (c) 2016-2024 Paul Ross. All rights reserved.
//

#include "py_call_super.h"

/* Call func_name on the super classes of self with the arguments and
 * keyword arguments.
 *
 * Equivalent to getattr(super(type(self), self), func_name)(*args, **kwargs)
 *
 * func_name is a Python string.
 * The implementation creates a new super object on each call.
 */
PyObject *
call_super_pyname(PyObject *self, PyObject *func_name,
                  PyObject *args, PyObject *kwargs) {
    PyObject *super_func = NULL;
    PyObject *super_args = NULL;
    PyObject *func = NULL;
    PyObject *result = NULL;

    // Error check input
    if (!PyUnicode_Check(func_name)) {
        PyErr_Format(PyExc_TypeError,
                     "super() must be called with unicode attribute not %s",
                     Py_TYPE(func_name)->tp_name);
    }
    // Will be decremented when super_args is decremented if Py_BuildValue succeeds.
    Py_INCREF(self->ob_type);
    Py_INCREF(self);
    super_args = Py_BuildValue("OO", (PyObject *) self->ob_type, self);
    if (!super_args) {
        Py_DECREF(self->ob_type);
        Py_DECREF(self);
        PyErr_SetString(PyExc_RuntimeError, "Could not create arguments for super().");
        goto except;
    }
    super_func = PyType_GenericNew(&PySuper_Type, super_args, NULL);
    if (!super_func) {
        PyErr_SetString(PyExc_RuntimeError, "Could not create super().");
        goto except;
    }
    // Use tuple as first arg, super() second arg (i.e. kwargs) should be NULL
    super_func->ob_type->tp_init(super_func, super_args, NULL);
    if (PyErr_Occurred()) {
        goto except;
    }
    func = PyObject_GetAttr(super_func, func_name);
    if (!func) {
        assert(PyErr_Occurred());
        goto except;
    }
    if (!PyCallable_Check(func)) {
        PyErr_Format(PyExc_AttributeError,
                     "super() attribute \"%S\" is not callable.", func_name);
        goto except;
    }
    result = PyObject_Call(func, args, kwargs);
    if (!result) {
        assert(PyErr_Occurred());
        goto except;
    }
    assert(!PyErr_Occurred());
    goto finally;
except:
    assert(PyErr_Occurred());
    Py_XDECREF(result);
    result = NULL;
finally:
    Py_XDECREF(super_func);
    Py_XDECREF(super_args);
    Py_XDECREF(func);
    return result;
}

/* Call func_name on the super classes of self with the arguments and
 * keyword arguments.
 *
 * Equivalent to getattr(super(type(self), self), func_name)(*args, **kwargs)
 *
 * func_name is a C string.
 * The implementation uses the builtin super().
 */
PyObject *
call_super_name(PyObject *self, const char *func_cname,
                PyObject *args, PyObject *kwargs) {
    PyObject *result = NULL;
    PyObject *func_name = PyUnicode_FromFormat(func_cname);
    if (!func_name) {
        PyErr_SetString(PyExc_RuntimeError,
                        "call_super_name(): Could not create string.");
        return NULL;
    }
    result = call_super_pyname(self, func_name, args, kwargs);
    Py_DECREF(func_name);
    return result;
}


/* Call func_name on the super classes of self with the arguments and
 * keyword arguments.
 *
 * Equivalent to getattr(super(type(self), self), func_name)(*args, **kwargs)
 *
 * func_name is a Python string.
 * The implementation uses the builtin super().
 */
extern PyObject *
call_super_pyname_lookup(PyObject *self, PyObject *func_name,
                         PyObject *args, PyObject *kwargs) {
    PyObject *result = NULL;
    PyObject *builtins = NULL;
    PyObject *super_type = NULL;
    PyObject *super = NULL;
    PyObject *super_args = NULL;
    PyObject *func = NULL;

    builtins = PyImport_AddModule("builtins");
    if (!builtins) {
        assert(PyErr_Occurred());
        goto except;
    }
    // Borrowed reference
    Py_INCREF(builtins);
    super_type = PyObject_GetAttrString(builtins, "super");
    if (!super_type) {
        assert(PyErr_Occurred());
        goto except;
    }
    // Will be decremented when super_args is decremented if Py_BuildValue succeeds.
    Py_INCREF(self->ob_type);
    Py_INCREF(self);
    super_args = Py_BuildValue("OO", (PyObject *) self->ob_type, self);
    if (!super_args) {
        Py_DECREF(self->ob_type);
        Py_DECREF(self);
        PyErr_SetString(PyExc_RuntimeError, "Could not create arguments for super().");
        goto except;
    }
    super = PyObject_Call(super_type, super_args, NULL);
    if (!super) {
        assert(PyErr_Occurred());
        goto except;
    }
    // The following code is the same as call_super_pyname()
    func = PyObject_GetAttr(super, func_name);
    if (!func) {
        assert(PyErr_Occurred());
        goto except;
    }
    if (!PyCallable_Check(func)) {
        PyErr_Format(PyExc_AttributeError,
                     "super() attribute \"%S\" is not callable.", func_name);
        goto except;
    }
    result = PyObject_Call(func, args, kwargs);
    if (!result) {
        assert(PyErr_Occurred());
        goto except;
    }
    assert(!PyErr_Occurred());
    goto finally;
except:
    assert(PyErr_Occurred());
    Py_XDECREF(result);
    result = NULL;
finally:
    Py_XDECREF(builtins);
    Py_XDECREF(super_args);
    Py_XDECREF(super_type);
    Py_XDECREF(super);
    Py_XDECREF(func);
    return result;
}

/* Call func_name on the super classes of self with the arguments and
 * keyword arguments.
 *
 * Equivalent to getattr(super(type(self), self), func_name)(*args, **kwargs)
 *
 * func_name is a C string.
 * The implementation uses the builtin super().
 */
extern PyObject *
call_super_name_lookup(PyObject *self, const char *func_cname,
                       PyObject *args, PyObject *kwargs) {

    PyObject *result = NULL;
    PyObject *func_name = PyUnicode_FromFormat(func_cname);
    if (!func_name) {
        PyErr_SetString(PyExc_RuntimeError,
                        "call_super_name_lookup(): Could not create string.");
        return NULL;
    }
    result = call_super_pyname_lookup(self, func_name, args, kwargs);
    Py_DECREF(func_name);
    return result;
}
