//
//  PythonExtensionPatterns.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 07/05/2014.
//  Copyright (c) 2014 Paul Ross. All rights reserved.
//
#include "Python.h"

#include <stdio.h>
#if 0

/* Examples of design patterns in C that are helpful for Python extensions. */

static PyObject *python_arithmitic_trad(PyObject *str_a, PyObject *str_b, int op) {
	/* Local Python objects. */
	PyObject *num_a    = NULL;
	PyObject *num_b    = NULL;
	/* The Python object to be returned. */
	PyObject *ret      = NULL;
	
	if(op < 0 || op > 3) {
        PyErr_SetString(PyExc_RuntimeError, "Operator not in range.");
        return NULL;
	}
	num_a = PyLong_FromString(str_a, NULL, 0); /* New ref. */
	if (! num_a) {
		PyErr_SetString(PyExc_ValueError, "Can not read string a.");
	    return NULL;
	}
	num_b = PyLong_FromString(str_b, NULL, 0); /* New ref. */
	if (! num_b) {
		PyErr_SetString(PyExc_ValueError, "Can not read string a.");
	    return NULL; /* Ooops, forgot to free num_a. */
	}
	
	switch (op) {
		case 0:
			ret = PyNumber_Add(num_a, num_b);
			break;
		case 1:
			ret = PyNumber_Subtract(num_a, num_b);
			break;
		case 2:
			ret = PyNumber_Multiply(num_a, num_b);
			break;
		case 3:
			ret = PyNumber_FloorDivide(num_a, num_b);
			break;
		default:
			exit(1);
			break;
	}
	/* Oh dear, if ret is NULL both loc_a and loc_b are leaked. */
	return ret;
}

/* Does some arithmitic on two objects - Returns a new reference. */
static PyObject *_PyNumber_Operate(PyObject *o1, PyObject *o2, int op) {
	PyObject *ret= NULL;
	
	goto try;
try:
    /* Increment the reference count of the arguments. */
    if (o1) {
        Py_INCREF(o1);
    }
    if (o2) {
        Py_INCREF(o2);
    }
	switch (op) {
		case 0:
			ret = PyNumber_Add(o1, o2);
			break;
		case 1:
			ret = PyNumber_Subtract(o1, o2);
			break;
		case 2:
			ret = PyNumber_Multiply(o1, o2);
			break;
		case 3:
			ret = PyNumber_FloorDivide(o1, o2);
			break;
		default:
			exit(1);
			break;
	}
	if (!ret) {
		PyErr_SetString(PyExc_ValueError, "Can not do the math.");
		goto except;
	} else {
	    goto finally;
	}
except:
    /* Failure so Py_XDECREF the return value. */
	Py_XDECREF(ret);
	ret = NULL;
finally:
    /* All _local_ PyObjects declared at the entry point are Py_XDECREF'd. */
    /* (nothing to do here). */
	/* Decrement the ref count of the arguments. */
	Py_XDECREF(o2);
	Py_XDECREF(o1);
	return ret;
}

static PyObject *python_arithmitic(char *str_a, char *str_b, int op) {
	/* Local C variables. */
	int err;
	/* Local Python objects. */
	PyObject *loc_a    = NULL;
	PyObject *loc_b    = NULL;
	/* The Python object to be returned. */
	PyObject *ret      = NULL;
	
	goto try;
	
try:
	err = _check_operator(op);
	if(err) {
		if (err) {
			PyErr_SetString(PyExc_RuntimeError, "Operator not in range.");
			goto except;
		}
	}
	loc_a = _PyObject_str_to_int(str_a);
	if (!loc_a) {
		PyErr_SetString(PyExc_ValueError, "Can not read string a.");
	    goto except;
	}
	loc_b = _PyObject_str_to_int(str_b);
	if (!loc_b) {
		PyErr_SetString(PyExc_ValueError, "Can not read string a.");
	    goto except;
	}
	
	ret = _PyNumber_Operate(loc_a, loc_b, op);
	if (!ret) {
	    goto except;
	}
	goto finally;
	
except:
    /* Failure so Py_XDECREF the return value. */
	Py_XDECREF(ret);
	ret = NULL;
finally:
    /* All _local_ PyObjects declared at the entry point are Py_XDECREF'd. */
	Py_XDECREF(loc_a);
	Py_XDECREF(loc_b);
	return ret;
}

#endif



