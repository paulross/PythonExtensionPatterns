//
// Created by Paul Ross on 01/08/2025.
//
#define PPY_SSIZE_T_CLEAN

#include "Python.h"

#include "DebugSet.h"
/* For access to new_unique_string().*/
#include "pyextpatt_util.h"

#ifndef __FILE_NAME__
#define __FILE_NAME__ "DebugSet.c"
#endif

#pragma mark - Sets

void dbg_PySet_Add(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PySet_New(NULL);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    assert(PySet_GET_SIZE(container) == 0);

    if (PySet_Add(container, value)) {
        assert(0);
    }
    assert(PySet_GET_SIZE(container) == 1);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    assert(PySet_Contains(container, value) == 1);

    // Now insert the same object again, dupe of the code above.
    if (PySet_Add(container, value)) {
        assert(0);
    }
    assert(PySet_GET_SIZE(container) == 1);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    assert(PySet_Contains(container, value) == 1);

    Py_DECREF(container);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    /* Clean up. */
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PySet_Discard(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PySet_New(NULL);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    assert(PySet_GET_SIZE(container) == 0);

    if (PySet_Add(container, value)) {
        assert(0);
    }
    assert(PySet_GET_SIZE(container) == 1);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    assert(PySet_Contains(container, value) == 1);

    if (PySet_Discard(container, value) != 1) {
        assert(0);
    }
    assert(PySet_GET_SIZE(container) == 0);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    assert(PySet_Contains(container, value) == 0);

    Py_DECREF(container);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    /* Clean up. */
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PySet_Pop(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PySet_New(NULL);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    assert(PySet_GET_SIZE(container) == 0);

    if (PySet_Add(container, value)) {
        assert(0);
    }
    assert(PySet_GET_SIZE(container) == 1);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    assert(PySet_Contains(container, value) == 1);

    // Now pop()
    PyObject *popped_value = PySet_Pop(container);

    assert(popped_value == value);

    assert(PySet_GET_SIZE(container) == 0);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    assert(PySet_Contains(container, value) == 0);

    Py_DECREF(container);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    /* Clean up. */
    Py_DECREF(value);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}
