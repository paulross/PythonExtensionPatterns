//
// Created by Paul Ross on 15/12/2024.
//
#define PPY_SSIZE_T_CLEAN

#include "Python.h"

#include "DebugContainers.h"

/* This is used to guarantee that Python is not caching a string value when we want to check the
 * reference counts after each string creation.
 * */
static long debug_test_count = 0;

PyObject *
new_unique_string(const char *function_name, const char *suffix) {
    PyObject *value =  NULL;
    if (suffix){
        value = PyUnicode_FromFormat("%s-%^s-%ld", function_name, suffix, debug_test_count);
    } else {
        value = PyUnicode_FromFormat("%s-%ld", function_name, debug_test_count);
    }
    /* To view in the debugger. */
    Py_UCS1 *buffer = PyUnicode_1BYTE_DATA(value);
    assert(buffer);
    ++debug_test_count;
    return value;
}

/**
 * TODO:
 * Check int PyTuple_SetItem with new tuple.
 * Check int PyTuple_SetItem with existing item in tuple.
 * Check PyTuple_SET_ITEM with new tuple.
 * Check PyTuple_SET_ITEM with existing item in tuple.
 * Tuple Py_BuildValue.
 *
 * Lists:
 * As above plus append.
 *
 * We should cover named tuples/dataclasses etc.:
 * file:///Users/engun/dev/Python/python-3.12.1-docs-html/c-api/tuple.html#struct-sequence-objects
 *
 */

/**
 * A function that checks whether a tuple steals a reference when using PyTuple_SetItem.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 */
void dbg_PyTuple_SetItem_steals(void) {
    if (PyErr_Occurred()) {
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyTuple_New(1);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyTuple_SetItem(container, 0, value)) {
        assert(0);
    }
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyObject *get_item = PyTuple_GET_ITEM(container, 0);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    Py_DECREF(container);
    /* NO as container deals with this. */
    /* Py_DECREF(value); */

    assert(!PyErr_Occurred());
}

/**
 * A function that checks whether a tuple steals a reference when using PyTuple_SET_ITEM.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 */
void dbg_PyTuple_SET_ITEM_steals(void) {
    if (PyErr_Occurred()) {
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyTuple_New(1);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyTuple_SET_ITEM(container, 0, value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyObject *get_item = PyTuple_GET_ITEM(container, 0);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    Py_DECREF(container);
    /* NO as container deals with this. */
    /* Py_DECREF(value); */

    assert(!PyErr_Occurred());
}

/**
 * A function that checks whether a tuple steals a reference when using PyTuple_SetItem on an existing item.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 *
 * This DOES leak an existing value contrary to the Python documentation.
 */
void dbg_PyTuple_SetItem_steals_replace(void) {
    if (PyErr_Occurred()) {
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyTuple_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);
    PyTuple_SetItem(container, 0, value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);

    PyObject *value_1 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_1);
    assert(ref_count == 1);

    /* Preserve the value_0 as this reference count is about to be decremented. */
    Py_INCREF(value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 2);

    /* Preserve the value_1 so that we can see Py_DECREF(container) decrements it. */
    Py_INCREF(value_1);
    ref_count = Py_REFCNT(value_1);
    assert(ref_count == 2);

    /* This will decrement the ref count of value_0 leaving it with a reference count of 1.*/
    PyTuple_SetItem(container, 0, value_1);
    ref_count = Py_REFCNT(value_1);
    assert(ref_count == 2);

    /* Check that value_0 has a ref count of 1. */
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);

    PyObject *get_item = PyTuple_GET_ITEM(container, 0);
    assert(get_item == value_1);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    /* Clean up. */
    Py_DECREF(container);
    ref_count = Py_REFCNT(value_1);
    assert(ref_count == 1);
    Py_DECREF(value_0);
    Py_DECREF(value_1);

    assert(!PyErr_Occurred());
}

/**
 * A function that checks whether a tuple steals a reference when using PyTuple_SET_ITEM on an existing item.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 */
void dbg_PyTuple_SET_ITEM_steals_replace(void) {
    if (PyErr_Occurred()) {
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyTuple_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);
    PyTuple_SET_ITEM(container, 0, value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);

    PyObject *value_1 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_1);
    assert(ref_count == 1);

    /* This will overwrite value_0 leaving it with a reference count of 1.*/
    PyTuple_SET_ITEM(container, 0, value_1);
    ref_count = Py_REFCNT(value_1);
    assert(ref_count == 1);
    PyObject *get_item = PyTuple_GET_ITEM(container, 0);
    assert(get_item == value_1);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    Py_DECREF(container);

    /* This is now leaked. */
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);

    Py_DECREF(value_0);

    assert(!PyErr_Occurred());
}
