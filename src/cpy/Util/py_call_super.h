//
//  py_call_super.h
//  PythonSubclassList
//
// Provides C functions to call the Python super() class.
//
//  Created by Paul Ross on 03/05/2016.
//  Copyright (c) 2016 Paul Ross. All rights reserved.
//

#ifndef __UTIL_PY_CALL_SUPER__
#define __UTIL_PY_CALL_SUPER__

#include <Python.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Call func_name on the super classes of self with the arguments and keyword arguments.
 * Equivalent to getattr(super(type(self), self), func_name)(*args, **kwargs)
 * func_name is a Python string.
 * The implementation creates a new super object on each call.
 */
extern PyObject *
call_super_pyname(PyObject *self, PyObject *func_name, PyObject *args, PyObject *kwargs);

/* Call func_name on the super classes of self with the arguments and keyword arguments.
 * Equivalent to getattr(super(type(self), self), func_name)(*args, **kwargs)
 * func_name is a C string.
 * The implementation creates a new super object on each call.
 */
extern PyObject *
call_super_name(PyObject *self, const char *func_cname, PyObject *args, PyObject *kwargs);

/* Call func_name on the super classes of self with the arguments and keyword arguments.
 * Equivalent to getattr(super(type(self), self), func_name)(*args, **kwargs)
 * func_name is a Python string.
 * The implementation uses the builtin super().
 */
extern PyObject *
call_super_pyname_lookup(PyObject *self, PyObject *func_name, PyObject *args, PyObject *kwargs);

/* Call func_name on the super classes of self with the arguments and keyword arguments.
 * Equivalent to getattr(super(type(self), self), func_name)(*args, **kwargs)
 * func_name is a C string.
 * The implementation uses the builtin super().
 */
extern PyObject *
call_super_name_lookup(PyObject *self, const char *func_cname, PyObject *args, PyObject *kwargs);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* #ifndef __UTIL_PY_CALL_SUPER__ */
