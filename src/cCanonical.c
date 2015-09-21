//
//  cCanonical.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 09/05/2014.
//  Copyright (c) 2014 Paul Ross. All rights reserved.
//

#include "Python.h"


#define ASSERT_EXCEPTION_RETURN_VALUE(ret) assert ret && ! PyErr_Occurred() \
|| !ret && PyErr_Occurred()



/* So the canonical form is: */
static PyObject *_func(PyObject *arg1) {
	/* Create any local Python *objects as NULL. */
	PyObject *obj_a    = NULL;
	/* Create the Python object to be returned as NULL. */
	PyObject *ret      = NULL;
	
	goto try; /* Pythonic 'C' ;-) */
try:
    assert(! PyErr_Occurred());
    /* Increment the reference count of the arguments. */
    assert(arg1);
    Py_INCREF(arg1);    /* Alt: if (arg1) Py_INCREF(arg1); See Py_XDECREF below. */
    
    /* Your code here */
    
    /* Local object creation. */
	/* obj_a = ...; */
    /* If an error then set error condition and goto except; */
	if (! obj_a) {
		PyErr_SetString(PyExc_ValueError, "Ooops.");
	    goto except;
    }
    /* Only do this if f obj_a is a borrowed reference. */
    Py_INCREF(obj_a);
    
    /* More of your code to do stuff with obj_a. */
	
    /* Return object creation, ret must be a new reference. */
	/* ret = ...; */
	if (! ret) {
		PyErr_SetString(PyExc_ValueError, "Ooops again.");
	    goto except;
	}
    
    /* Any other error checking here. */
    
    /* If success then check exception is clear,
     * then goto finally; with non-NULL return value. */
    assert(! PyErr_Occurred());
    assert(ret);
	goto finally;
except:
    /* Failure so Py_XDECREF the return value here. */
	Py_XDECREF(ret);
    /* Check a Python error is set somewhere above. */
    assert(PyErr_Occurred());
    /* Signal failure. */
	ret = NULL;
finally:
    /* All _local_ PyObjects declared at the entry point are Py_XDECREF'd here.
     * For new references this will free them. For borrowed references this
     * will return them to their previous state.
     */
	Py_XDECREF(obj_a);
	/* Decrement the ref count of externally supplied the arguments here.
     * If you allow arg1 == NULL then Py_XDECREF(arg1). */
	Py_DECREF(arg1);
	/* And return...*/
	return ret;
}

