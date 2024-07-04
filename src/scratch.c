//
//  scratch.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 04/04/2015.
//  Copyright (c) 2015 Paul Ross. All rights reserved.
//

#include "Python.h"

#include <stdio.h>
#include <stdlib.h>


void leak(void) {
    char *p;
    
    p = malloc(1024);
    fprintf(stdout, "malloc(1024) returns %s", p);
}


void access_after_free(void) {
    char *p;
    
    p = malloc(1024);
    free(p);
    
    p[8] = 'A';
    printf("%c", p[8]);
}


#include "Python.h"

void py_leak(void) {
    PyObject *pObj = NULL;
    
    /* Object creation, ref count = 1. */
    pObj = PyBytes_FromString("Hello world\n");
    PyObject_Print(pObj, stdout, 0);
    /* Object still has ref count = 1. */
}

#include "Python.h"

void py_access_after_free(void) {
    PyObject *pObj = NULL;
    
    /* Object creation, ref count = 1. */
    pObj = PyBytes_FromString("Hello world\n");
    PyObject_Print(pObj, stdout, 0);
    /* ref count = 0 so object deallocated. */
    Py_DECREF(pObj);
    /* Now use pObj... */
    PyObject_Print(pObj, stdout, 0);
}

void py_caller_access_after_free(PyObject *pObj) {
    /* ... code here ... */
    Py_DECREF(pObj);
    /* ... more code here ... */
}




PyObject *bad_incref(PyObject *pObj) {
    Py_INCREF(pObj);
    int error = 0;
    /* ... a metric ton of code here ... */
    if (error) {
        /* No matching Py_DECREF, pObj is leaked. */
        return NULL;
    }
    /* ... more code here ... */
    Py_DECREF(pObj);
    Py_RETURN_NONE;
}


void bad_steal(void) {

PyObject *v, *r;

r = PyTuple_New(3);         /* New reference. */
v = PyLong_FromLong(1L);    /* New reference. */
PyTuple_SetItem(r, 0, v);   /* r takes ownership of the reference. */
Py_DECREF(v);               /* Now we are interfering with r's internals. */
    
    
/* Two common patterns to avoid this, either: */
v = PyLong_FromLong(1L);    /* New reference. */
PyTuple_SetItem(r, 0, v);   /* r takes ownership of the reference. */
v = NULL;
/* Or: */
PyTuple_SetItem(r, 0, PyLong_FromLong(1L));

}




static PyObject *pop_and_print_BAD(PyObject *pList) {
    PyObject *pLast;
    
    pLast = PyList_GetItem(pList, PyList_Size(pList) - 1);
    fprintf(stdout, "Ref count was: %zd\n", pLast->ob_refcnt);
    //do_something(pList);    /* Dragons ahoy me hearties! */
    fprintf(stdout, "Ref count now: %zd\n", pLast->ob_refcnt);
    PyObject_Print(pLast, stdout, 0);
    fprintf(stdout, "\n");
    Py_RETURN_NONE;
}


