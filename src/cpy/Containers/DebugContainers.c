//
// Created by Paul Ross on 15/12/2024.
//
#define PPY_SSIZE_T_CLEAN

#include "Python.h"

#include "DebugContainers.h"

#pragma mark - Tuples

/**
 * A function that checks whether a tuple steals a reference when using PyTuple_SetItem.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 */
void dbg_PyTuple_SetItem_steals(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
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
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
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
 */
void dbg_PyTuple_SetItem_steals_replace(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
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
    int result = PyTuple_SetItem(container, 0, value_0);
    assert(result == 0);
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

    /* This will decrement the ref count of value_0 leaving it with a reference count of 1.
     * Whilst preserving the reference count of value_1 of 2. */
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
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
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

    /* This is demonstrated as leaked. */
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);
    Py_DECREF(value_0);

    assert(!PyErr_Occurred());
}

void dbg_PyTuple_SetItem_replace_with_same(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyTuple_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    int result = PyTuple_SetItem(container, 0, value);
    assert(result == 0);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    /* Increment the reference count to track the bad behaviour. */
    Py_INCREF(value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    /* This will decrement the reference count of value as it is the previous value.
     * That will free the current value and set garbage in the tuple. */
    result = PyTuple_SetItem(container, 0, value);
    assert(result == 0);
    ref_count = Py_REFCNT(value);
    /* This is only alive because of Py_INCREF(value); above. */
    assert(ref_count == 1);

    PyObject *get_item = PyTuple_GET_ITEM(container, 0);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    /* Increment the reference count from 1 so we can see it go back to 1 on Py_DECREF(container);. */
    Py_INCREF(value);
    Py_DECREF(container);
    /* Clean up. */
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyTuple_SET_ITEM_replace_with_same(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
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

    /* Second PyTuple_SET_ITEM(). */
    /* This will NOT decrement the reference count of value as it is the previous value. */
    PyTuple_SET_ITEM(container, 0, value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyObject *get_item = PyTuple_GET_ITEM(container, 0);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    /* Increment the reference count from 1 so we can see it go back to 1 on Py_DECREF(container);. */
    Py_INCREF(value);
    Py_DECREF(container);
    /* Clean up. */
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

/**
 * Function that explores setting an item in a tuple to NULL with PyTuple_SetItem().
 */
void dbg_PyTuple_SetIem_NULL(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyTuple_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    PyTuple_SetItem(container, 0, NULL);
    assert(!PyErr_Occurred());

    Py_DECREF(container);

    assert(!PyErr_Occurred());
}

/**
 * Function that explores setting an item in a tuple to NULL with PyTuple_SET_ITEM().
 */
void dbg_PyTuple_SET_ITEM_NULL(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyTuple_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    PyTuple_SET_ITEM(container, 0, NULL);
    assert(!PyErr_Occurred());

    Py_DECREF(container);

    assert(!PyErr_Occurred());
}

/**
 * Function that explores setting an item in a tuple to NULL with PyTuple_SetItem() then setting it to a value.
 */
void dbg_PyTuple_SetIem_NULL_SetItem(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyTuple_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    PyTuple_SetItem(container, 0, NULL);
    assert(!PyErr_Occurred());

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);

    /* Preserve the value_0 as this reference count is about to be decremented. */
    Py_INCREF(value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 2);

    PyTuple_SetItem(container, 0, value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 2);

    Py_DECREF(container);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);
    Py_DECREF(value_0);

    assert(!PyErr_Occurred());
}

/**
 * Function that explores setting an item in a tuple to NULL with PyTuple_SET_ITEM() then setting it to a value.
 */
void dbg_PyTuple_SET_ITEM_NULL_SET_ITEM(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyTuple_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    PyTuple_SetItem(container, 0, NULL);
    assert(!PyErr_Occurred());

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);

    /* Preserve the value_0 as this reference count is about to be decremented. */
    Py_INCREF(value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 2);

    PyTuple_SET_ITEM(container, 0, value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 2);

    Py_DECREF(container);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);
    Py_DECREF(value_0);

    assert(!PyErr_Occurred());
}

/**
 * A function that checks PyTuple_SetItem when the container is not a tuple.
 * This decrements the value reference count.
 */
void dbg_PyTuple_SetItem_fails_not_a_tuple(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(1);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    /* We want to to hold onto this as PyTuple_SetItem() will decref it. */
    Py_INCREF(value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    int result = PyTuple_SetItem(container, 0, value);
    assert(result == -1);
    assert(PyErr_Occurred());
    fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    PyErr_Print();

    /* Yes, has been decremented on failure. */
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    Py_DECREF(container);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

/**
 * A function that checks PyTuple_SetItem when the container is not a tuple.
 * This decrements the value reference count.
 */
void dbg_PyTuple_SetItem_fails_out_of_range(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
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

    /* We want to to hold onto this as PyTuple_SetItem() will decref it. */
    Py_INCREF(value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    int result = PyTuple_SetItem(container, 1, value);
    assert(result == -1);
    assert(PyErr_Occurred());
    fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    PyErr_Print();

    /* Yes, has been decremented on failure. */
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    Py_DECREF(container);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

/**
 * Function that explores PyTuple_Pack(n, ...).
 */
void dbg_PyTuple_PyTuple_Pack(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());

    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL);

    PyObject *container = PyTuple_Pack(2, value_a, value_b);

    assert(Py_REFCNT(value_a) == 2);
    assert(Py_REFCNT(value_b) == 2);

    Py_DECREF(container);

    /* Leaks: */
    assert(Py_REFCNT(value_a) == 1);
    assert(Py_REFCNT(value_b) == 1);

    Py_DECREF(value_a);
    Py_DECREF(value_b);

    assert(!PyErr_Occurred());
}

/**
 * Function that explores Py_BuildValue("(O)", ...).
 */
void dbg_PyTuple_Py_BuildValue(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyObject *container = Py_BuildValue("(O)", value);

    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    Py_DECREF(container);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

#pragma mark - Lists

/**
 * A function that checks whether a tuple steals a reference when using PyList_SetItem.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 */
void dbg_PyList_SetItem_steals(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(1);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyList_SetItem(container, 0, value)) {
        assert(0);
    }
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyObject *get_item = PyList_GET_ITEM(container, 0);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    Py_DECREF(container);
    /* NO as container deals with this. */
    /* Py_DECREF(value); */

    assert(!PyErr_Occurred());
}

/**
 * A function that checks whether a tuple steals a reference when using PyList_SET_ITEM.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 */
void dbg_PyList_SET_ITEM_steals(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(1);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyList_SET_ITEM(container, 0, value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyObject *get_item = PyList_GET_ITEM(container, 0);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    Py_DECREF(container);
    /* NO as container deals with this. */
    /* Py_DECREF(value); */

    assert(!PyErr_Occurred());
}

/**
 * A function that checks whether a tuple steals a reference when using PyList_SetItem on an existing item.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 *
 * This DOES leak an existing value contrary to the Python documentation.
 */
void dbg_PyList_SetItem_steals_replace(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyList_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);
    int result = PyList_SetItem(container, 0, value_0);
    assert(result == 0);
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
    PyList_SetItem(container, 0, value_1);
    ref_count = Py_REFCNT(value_1);
    assert(ref_count == 2);

    /* Check that value_0 has a ref count of 1. */
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);

    PyObject *get_item = PyList_GET_ITEM(container, 0);
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
 * A function that checks whether a tuple steals a reference when using PyList_SET_ITEM on an existing item.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 */
void dbg_PyList_SET_ITEM_steals_replace(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyList_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);

    PyList_SET_ITEM(container, 0, value_0);

    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);

    PyObject *value_1 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_1);
    assert(ref_count == 1);

    /* This will overwrite value_0 leaving it with a reference count of 1.*/
    PyList_SET_ITEM(container, 0, value_1);
    ref_count = Py_REFCNT(value_1);
    assert(ref_count == 1);
    PyObject *get_item = PyList_GET_ITEM(container, 0);
    assert(get_item == value_1);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    Py_DECREF(container);

    /* This is demonstrated as leaked. */
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);
    Py_DECREF(value_0);

    assert(!PyErr_Occurred());
}

void dbg_PyList_SetItem_replace_with_same(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyList_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    int result = PyList_SetItem(container, 0, value);
    assert(result == 0);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    /* Increment the reference count to track the bad behaviour. */
    Py_INCREF(value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    /* This will decrement the reference count of value as it is the previous value.
     * That will free the current value and set garbage in the tuple. */
    result = PyList_SetItem(container, 0, value);
    assert(result == 0);
    ref_count = Py_REFCNT(value);
    /* This is only alive because of Py_INCREF(value); above. */
    assert(ref_count == 1);

    PyObject *get_item = PyList_GET_ITEM(container, 0);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    /* Increment the reference count from 1 so we can see it go back to 1 on Py_DECREF(container);. */
    Py_INCREF(value);
    Py_DECREF(container);
    /* Clean up. */
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyList_SET_ITEM_replace_with_same(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyList_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    PyList_SET_ITEM(container, 0, value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    /* Second PyList_SET_ITEM(). */
    /* This will NOT decrement the reference count of value as it is the previous value. */
    PyList_SET_ITEM(container, 0, value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyObject *get_item = PyList_GET_ITEM(container, 0);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    /* Increment the reference count from 1 so we can see it go back to 1 on Py_DECREF(container);. */
    Py_INCREF(value);
    Py_DECREF(container);
    /* Clean up. */
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

/**
 * Function that explores setting an item in a tuple to NULL with PyList_SetItem().
 */
void dbg_PyList_SetIem_NULL(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyList_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    PyList_SetItem(container, 0, NULL);
    assert(!PyErr_Occurred());

    Py_DECREF(container);

    assert(!PyErr_Occurred());
}

/**
 * Function that explores setting an item in a tuple to NULL with PyList_SET_ITEM().
 */
void dbg_PyList_SET_ITEM_NULL(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyList_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    PyList_SET_ITEM(container, 0, NULL);
    assert(!PyErr_Occurred());

    Py_DECREF(container);

    assert(!PyErr_Occurred());
}

/**
 * Function that explores setting an item in a tuple to NULL with PyList_SetItem() then setting it to a value.
 */
void dbg_PyList_SetIem_NULL_SetItem(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyList_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    PyList_SetItem(container, 0, NULL);
    assert(!PyErr_Occurred());

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);

    /* Preserve the value_0 as this reference count is about to be decremented. */
    Py_INCREF(value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 2);

    PyList_SetItem(container, 0, value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 2);

    Py_DECREF(container);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);
    Py_DECREF(value_0);

    assert(!PyErr_Occurred());
}

/**
 * Function that explores setting an item in a tuple to NULL with PyList_SET_ITEM() then setting it to a value.
 */
void dbg_PyList_SET_ITEM_NULL_SET_ITEM(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;
    PyObject *container = PyList_New(1);
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    PyList_SetItem(container, 0, NULL);
    assert(!PyErr_Occurred());

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);

    /* Preserve the value_0 as this reference count is about to be decremented. */
    Py_INCREF(value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 2);

    PyList_SET_ITEM(container, 0, value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 2);

    Py_DECREF(container);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 1);
    Py_DECREF(value_0);

    assert(!PyErr_Occurred());
}

/**
 * A function that checks PyList_SetItem when the container is not a tuple.
 * This decrements the value reference count.
 */
void dbg_PyList_SetItem_fails_not_a_tuple(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
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

    /* We want to to hold onto this as PyList_SetItem() will decref it. */
    Py_INCREF(value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    int result = PyList_SetItem(container, 0, value);
    assert(result == -1);
    assert(PyErr_Occurred());
    fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    PyErr_Print();

    /* Yes, has been decremented on failure. */
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    Py_DECREF(container);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

/**
 * A function that checks PyList_SetItem when the container is not a tuple.
 * This decrements the value reference count.
 */
void dbg_PyList_SetItem_fails_out_of_range(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(1);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    /* We want to to hold onto this as PyList_SetItem() will decref it. */
    Py_INCREF(value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    int result = PyList_SetItem(container, 1, value);
    assert(result == -1);
    assert(PyErr_Occurred());
    fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    PyErr_Print();

    /* Yes, has been decremented on failure. */
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    Py_DECREF(container);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

/**
 * A function that checks whether a list increments a reference when using PyList_Append.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 */
void dbg_PyList_Append(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(0);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyList_Append(container, value)) {
        assert(0);
    }
    // PyList_Append increments.
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    PyObject *get_item = PyList_GET_ITEM(container, 0);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    Py_DECREF(container);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    /* Need this. */
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyList_Append_fails_not_a_list(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyTuple_New(1);
    assert(container);
    assert(!PyErr_Occurred());

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    int result = PyList_Append(container, value);
    assert(result);

    assert(PyErr_Occurred());
    fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    PyErr_Print();
    assert(!PyErr_Occurred());

    Py_DECREF(container);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyList_Append_fails_NULL(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(0);
    assert(container);
    assert(!PyErr_Occurred());

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    int result = PyList_Append(container, NULL);
    assert(result);

    assert(PyErr_Occurred());
    fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    PyErr_Print();
    assert(!PyErr_Occurred());

    Py_DECREF(container);

    assert(!PyErr_Occurred());
}

void dbg_PyList_Insert(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(0);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    assert(PyList_GET_SIZE(container) == 0);
    if (PyList_Insert(container, 0L, value)) {
        assert(0);
    }
    assert(PyList_GET_SIZE(container) == 1);
    // PyList_Append increments.
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    PyObject *get_item = PyList_GET_ITEM(container, 0);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    Py_DECREF(container);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    /* Need this. */
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyList_Insert_Is_Truncated(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(0);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyList_Insert(container, 4L, value)) {
        assert(0);
    }
    // PyList_Insert increments.
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    PyObject *get_item;
    // PyList_Insert at 4 actually inserts at 0.
    assert(PyList_GET_SIZE(container) == 1L);
    get_item = PyList_GET_ITEM(container, 0L);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    Py_DECREF(container);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    /* Need this. */
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyList_Insert_Negative_Index(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(0);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyList_Insert(container, -1L, value)) {
        assert(0);
    }
    // PyList_Insert increments.
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    PyObject *get_item;
    // PyList_Insert at -1 actually inserts at 0.
    assert(PyList_GET_SIZE(container) == 1L);
    get_item = PyList_GET_ITEM(container, 0L);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    Py_DECREF(container);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    /* Need this. */
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyList_Insert_fails_not_a_list(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyTuple_New(1);
    assert(container);
    assert(!PyErr_Occurred());

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    int result = PyList_Insert(container, 1L, value);
    assert(result);

    assert(PyErr_Occurred());
    fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    PyErr_Print();
    assert(!PyErr_Occurred());

    Py_DECREF(container);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyList_Insert_fails_NULL(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(1);
    assert(container);
    assert(!PyErr_Occurred());

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    int result = PyList_Insert(container, 1L, NULL);
    assert(result);

    assert(PyErr_Occurred());
    fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    PyErr_Print();
    assert(!PyErr_Occurred());

    Py_DECREF(container);

    assert(!PyErr_Occurred());
}

/**
 * Function that explores Py_BuildValue("(O)", ...).
 */
void dbg_PyList_Py_BuildValue(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count;

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyObject *container = Py_BuildValue("[O]", value);

    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    Py_DECREF(container);

    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

#pragma mark - Dictionaries - setters

void dbg_PyDict_SetItem_increments(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;
    PyObject *get_item;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);

    if (PyDict_SetItem(container, key, value_a)) {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);

    get_item = PyDict_GetItem(container, key);
    assert(get_item == value_a);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    /* Now replace the value using the same key. */
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 1);

    if (PyDict_SetItem(container, key, value_b)) {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);

    get_item = PyDict_GetItem(container, key);
    assert(get_item == value_b);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    // Replace with existing key/value_b. Reference counts should remain the same.
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);
    if (PyDict_SetItem(container, key, value_b)) {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);

    Py_DECREF(container);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 1);

    Py_DECREF(key);
    Py_DECREF(value_a);
    Py_DECREF(value_b);

    assert(!PyErr_Occurred());
}

#if ACCEPT_SIGSEGV

void dbg_PyDict_SetItem_NULL_key(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    PyObject *container = PyDict_New();
    assert(container);

    PyObject *key = NULL;
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    // Segfault
    PyDict_SetItem(container, key, value);
}

void dbg_PyDict_SetItem_NULL_value(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    PyObject *container = PyDict_New();
    assert(container);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    PyObject *value = NULL;
    // Segfault
    PyDict_SetItem(container, key, value);
}

#endif // ACCEPT_SIGSEGV

void dbg_PyDict_SetItem_fails_not_a_dict(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(0);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    int result = PyDict_SetItem(container, key, value);
    if (result) {
        assert(PyErr_Occurred());
        fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print(); /* Clears the error. */
    } else {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    Py_DECREF(container);
    Py_DECREF(key);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyDict_SetItem_fails_not_hashable(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = PyList_New(0);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    int result = PyDict_SetItem(container, key, value);
    if (result) {
        assert(PyErr_Occurred());
        fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print(); /* Clears the error. */
    } else {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    Py_DECREF(container);
    Py_DECREF(key);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyDict_SetDefault_default_unused(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;
    PyObject *get_item;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyDict_SetItem(container, key, value)) {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    get_item = PyDict_GetItem(container, key);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    /* Now check PyDict_SetDefault() which does not use the default. */
    PyObject *value_default = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_default);
    assert(ref_count == 1);

    get_item = PyDict_SetDefault(container, key, value_default);
    if (! get_item) {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_default);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);
    assert(get_item == value);

    Py_DECREF(container);

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(value);
    Py_DECREF(value_default);

    assert(!PyErr_Occurred());
}

void dbg_PyDict_SetDefault_default_used(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;
    PyObject *get_item;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);

    /* Do not do this so the default is invoked.
    if (PyDict_SetItem(container, key, value)) {
        assert(0);
    }
    */

    /* Now check PyDict_SetDefault() which *does* use the default. */
    PyObject *value_default = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_default);
    assert(ref_count == 1);

    get_item = PyDict_SetDefault(container, key, value_default);
    if (! get_item) {
        assert(0);
    }
    assert(PyDict_Size(container) == 1);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_default);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);
    assert(get_item == value_default);

    Py_DECREF(container);

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(value_default);

    assert(!PyErr_Occurred());
}

#pragma mark - Dictionaries [Python3.13]

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

// PyDict_SetDefaultRef
// int PyDict_SetDefaultRef(PyObject *p, PyObject *key, PyObject *default_value, PyObject **result)
// https://docs.python.org/3/c-api/dict.html#c.PyDict_SetDefaultRef
void dbg_PyDict_SetDefaultRef_default_unused(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyDict_SetItem(container, key, value)) {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    PyObject *get_item = PyDict_GetItem(container, key);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    /* Now check PyDict_SetDefault() which does not use the default. */
    PyObject *default_value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(default_value);
    assert(ref_count == 1);

    /* From https://docs.python.org/3/c-api/dict.html#c.PyDict_SetDefaultRef lightly edited.
     *
     * Inserts default_value into the dictionary p with a key of key if the key is not already
     * present in the dictionary.
     * If result is not NULL, then *result is set to a strong reference to either default_value,
     * if the key was not present, or the existing value, if key was already present in the dictionary.
     * Returns:
     * 1 if the key was present and default_value was not inserted.
     * 0 if the key was not * present and default_value was inserted.
     * -1 on failure, sets an exception, and sets *result to NULL.
     *
     * For clarity: if you have a strong reference to default_value before calling this function,
     * then after it returns, you hold a strong reference to both default_value and *result (if it’s not NULL).
     * These may refer to the same object: in that case you hold two separate references to it.
     */
    PyObject *result = NULL;
    int return_value = PyDict_SetDefaultRef(container, key, default_value, &result);
    if (return_value != 1) {
        assert(0);
    }

    assert(result == value);

    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(default_value);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(result);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 3);
    assert(get_item == value);

    Py_DECREF(container);

    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(default_value);
    assert(ref_count == 1);

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(value);
    Py_DECREF(value);
    Py_DECREF(default_value);

    assert(!PyErr_Occurred());
}

void dbg_PyDict_SetDefaultRef_default_used(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);

    PyObject *value_default = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_default);
    assert(ref_count == 1);

    /* From https://docs.python.org/3/c-api/dict.html#c.PyDict_SetDefaultRef lightly edited.
     *
     * Inserts default_value into the dictionary p with a key of key if the key is not already
     * present in the dictionary.
     * If result is not NULL, then *result is set to a strong reference to either default_value,
     * if the key was not present, or the existing value, if key was already present in the dictionary.
     * Returns:
     * 1 if the key was present and default_value was not inserted.
     * 0 if the key was not * present and default_value was inserted.
     * -1 on failure, sets an exception, and sets *result to NULL.
     *
     * For clarity: if you have a strong reference to default_value before calling this function,
     * then after it returns, you hold a strong reference to both default_value and *result (if it’s not NULL).
     * These may refer to the same object: in that case you hold two separate references to it.
     */
    PyObject *result = NULL;
    int return_value = PyDict_SetDefaultRef(container, key, value_default, &result);
    if (return_value != 0) {
        assert(0);
    }

    assert(result == value_default);

    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_default);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(result);
    assert(ref_count == 3);

    Py_DECREF(container);

    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_default);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(result);
    assert(ref_count == 2);

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(value_default);
    Py_DECREF(value_default);

    assert(!PyErr_Occurred());
}

/*
 * This explores using PyDict_SetDefaultRef when result is a live Python object.
 * The previous version of result is abandoned.
 */
void dbg_PyDict_SetDefaultRef_default_unused_result_non_null(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyDict_SetItem(container, key, value)) {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    PyObject *get_item = PyDict_GetItem(container, key);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    /* Now check PyDict_SetDefault() which does not use the default. */
    PyObject *value_default = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_default);
    assert(ref_count == 1);

    PyObject *result_live = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(result_live);
    assert(ref_count == 1);

    PyObject *result = result_live;
    int return_value = PyDict_SetDefaultRef(container, key, value_default, &result);
    if (return_value != 1) {
        assert(0);
    }

    assert(result != result_live);
    assert(result == value);

    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(value_default);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(result_live);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(result);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 3);
    assert(get_item == value);

    Py_DECREF(container);

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(value);
    Py_DECREF(value);
    Py_DECREF(value_default);
    Py_DECREF(result_live);

    assert(!PyErr_Occurred());
}

#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

#pragma mark Dictionaries - getters

void dbg_PyDict_GetItem(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;
    PyObject *get_item;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);


    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);

    // No Key in the dictionary, no exception set.
    assert(!PyErr_Occurred());
    get_item = PyDict_GetItem(container, key);
    assert(get_item == NULL);
    assert(!PyErr_Occurred());

    // Set a value
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyDict_SetItem(container, key, value)) {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    get_item = PyDict_GetItem(container, key);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    Py_DECREF(container);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    Py_DECREF(key);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

void dbg_PyDict_GetItemRef(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);

    // Create something for result to point to and check it is abandoned.
    PyObject *dummy_result = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(dummy_result);
    assert(ref_count == 1);
    PyObject *result = dummy_result;

    // No Key in the dictionary, no exception set.
    assert(!PyErr_Occurred());
    int ret_val = PyDict_GetItemRef(container, key, &result);
    assert(!PyErr_Occurred());
    assert(ret_val == 0);
    assert(result == NULL);
    ref_count = Py_REFCNT(dummy_result);
    assert(ref_count == 1);

    // Set a value
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyDict_SetItem(container, key, value)) {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    assert(!PyErr_Occurred());
    ret_val = PyDict_GetItemRef(container, key, &result);
    assert(!PyErr_Occurred());
    assert(ret_val == 1);
    // value reference count has been incremented.
    assert(result == value);
    ref_count = Py_REFCNT(result);
    assert(ref_count == 3);

    Py_DECREF(container);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    Py_DECREF(key);
    Py_DECREF(value);
    Py_DECREF(value);
    Py_DECREF(dummy_result);

    assert(!PyErr_Occurred());
}

#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

/**
 * This tests PyDict_GetItemWithError which contrary to the Python documentation
 * does *not* set an exception if the key exists.
 */
void dbg_PyDict_GetItemWithError_fails(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;
    PyObject *get_item;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);


    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);

    // No Key in the dictionary, exception set.
    assert(!PyErr_Occurred());
    get_item = PyDict_GetItemWithError(container, key);
    assert(get_item == NULL);
    /* This is correct, the key is absent. */
    assert(!PyErr_Occurred());

    /* So what error conditinos are handled?
     * Firstly this will segfault. */
#if 0
    assert(!PyErr_Occurred());
    get_item = PyDict_GetItemWithError(container, NULL);
    assert(get_item == NULL);
    assert(PyErr_Occurred());
#endif

    PyObject *new_container = PyList_New(0);
    assert(!PyErr_Occurred());
    get_item = PyDict_GetItemWithError(new_container, key);
    assert(get_item == NULL);
    assert(PyErr_Occurred());
    PyErr_Print(); /* Clears exception. */
    Py_DECREF(new_container);

    Py_DECREF(container);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);

    Py_DECREF(key);

    assert(!PyErr_Occurred());
}

#pragma mark - Dictionaries - deleters

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

// PyDict_Pop
// int PyDict_Pop(PyObject *p, PyObject *key, PyObject **result)
// https://docs.python.org/3/c-api/dict.html#c.PyDict_Pop
void dbg_PyDict_Pop_key_present(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyDict_SetItem(container, key, value)) {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);

    PyObject *get_item = PyDict_GetItem(container, key);
    assert(get_item == value);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);

    assert(PyDict_GET_SIZE(container) == 1);

    PyObject *result = NULL;
    int return_value = PyDict_Pop(container, key, &result);
    if (return_value != 1) {
        assert(0);
    }

    assert(PyDict_GET_SIZE(container) == 0);

    assert(result == value);

    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(result);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);
    assert(get_item == value);

    Py_DECREF(container);

    /* Dupe of above as the container is empty. */
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(result);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 2);
    assert(get_item == value);

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(value);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyDict_Pop_key_absent(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);

    assert(PyDict_GET_SIZE(container) == 0);

    /* Not inserted into the dict, just used so that result references it. */
    PyObject *dummy_value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(dummy_value);
    assert(ref_count == 1);

    PyObject *result = dummy_value;
    int return_value = PyDict_Pop(container, key, &result);
    if (return_value != 0) {
        assert(0);
    }

    assert(PyDict_GET_SIZE(container) == 0);

    assert(result == NULL);

    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(dummy_value);
    assert(ref_count == 1);

    Py_DECREF(container);

    /* Dupe of above as the container is empty. */
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(dummy_value);
    assert(ref_count == 1);

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(dummy_value);

    assert(!PyErr_Occurred());
}

#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

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

#pragma mark - Struct Sequence

static PyStructSequence_Field struct_sequence_simple_type_fields[] = {
        {"family_name", "Family name."},
        {"given_name", "Given name."},
        {NULL, NULL}
};

static PyStructSequence_Desc struct_sequence_simple_type_desc = {
        "module.struct_sequence_simple",
        ".",
        struct_sequence_simple_type_fields,
        2,
};

static PyTypeObject *static_struct_sequence_simple_type = NULL;

void dbg_PyStructSequence_simple_ctor(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    if (static_struct_sequence_simple_type == NULL) {
        static_struct_sequence_simple_type = PyStructSequence_NewType(&struct_sequence_simple_type_desc);
    }
    assert(static_struct_sequence_simple_type != NULL);
    /* Hmm the refcount is 8. */
//    ref_count = Py_REFCNT(example_type);
//    assert(ref_count == 1);

    PyObject *instance = PyStructSequence_New(static_struct_sequence_simple_type);

    ref_count = Py_REFCNT(instance);
    assert(ref_count == 1);

    /* Get an unset item. */
    PyObject *get_item = NULL;
    get_item = PyStructSequence_GetItem(instance, 0);
    assert(get_item == NULL);

    /* Now set items. */
    PyObject *set_item = NULL;
    set_item = new_unique_string(__FUNCTION__, "NAME");
    PyStructSequence_SetItem(instance, 0, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);
    set_item = new_unique_string(__FUNCTION__, "GENDER");
    PyStructSequence_SetItem(instance, 1, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);

    /* Get items. */
    get_item = PyStructSequence_GetItem(instance, 0);
    assert(get_item != NULL);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);
    get_item = PyStructSequence_GetItem(instance, 1);
    assert(get_item != NULL);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    /* Clean up. */
    Py_DECREF(instance);
    Py_DECREF(static_struct_sequence_simple_type);
}

void dbg_PyStructSequence_setitem_abandons(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    if (static_struct_sequence_simple_type == NULL) {
        static_struct_sequence_simple_type = PyStructSequence_NewType(&struct_sequence_simple_type_desc);
    }
    assert(static_struct_sequence_simple_type != NULL);
    /* Hmm the ref count is 7. */
//    ref_count = Py_REFCNT(example_type);
//    assert(ref_count == 1);

    PyObject *instance = PyStructSequence_New(static_struct_sequence_simple_type);

    ref_count = Py_REFCNT(instance);
    assert(ref_count == 1);

    /* Now set items. */
    PyObject *set_item = NULL;
    set_item = new_unique_string(__FUNCTION__, "NAME");
    PyStructSequence_SetItem(instance, 0, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);
    /* Set it again. */
    PyStructSequence_SetItem(instance, 0, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);

    /* Clean up. */
    Py_DECREF(instance);
    Py_DECREF(static_struct_sequence_simple_type);
}

PyDoc_STRVAR(
    struct_sequence_n_in_sequence_too_large_docstring,
    "This uses struct_sequence_simple_type_fields but n_in_sequence is 3 rather than 2."
);

/*
 * This uses struct_sequence_simple_type_fields but n_in_sequence is 3 rather than 2.
 */
static PyStructSequence_Desc struct_sequence_n_in_sequence_too_large_type_desc = {
        "module.struct_sequence_n_in_sequence_too_large",
        struct_sequence_n_in_sequence_too_large_docstring,
        struct_sequence_simple_type_fields,
        3,
};

void dbg_PyStructSequence_n_in_sequence_too_large(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print(); /* Clears error. */
        return;
    }
    assert(!PyErr_Occurred());
//    Py_ssize_t ref_count;
    static PyTypeObject *example_type = NULL;

    if (example_type == NULL) {
        example_type = PyStructSequence_NewType(&struct_sequence_n_in_sequence_too_large_type_desc);
    }
    assert(example_type == NULL);
    assert(PyErr_Occurred());
    /* TypeError: tp_basicsize for type 'module.struct_sequence_n_in_sequence_too_large' (16) is too small for base 'tuple' (24). */
    fprintf(stderr, "%s(): On exit PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    PyErr_Print(); /* Clears error. */
}


void dbg_PyStructSequence_with_unnamed_field(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyStructSequence_Field struct_sequence_with_unnamed_fields[] = {
            {"family_name", "Family name."},
            /* Use NULL then replace with PyStructSequence_UnnamedField
             * otherwise get an error "initializer element is not a compile-time constant" */
            {"given_name", "Given name."},
            {PyStructSequence_UnnamedField, "Documentation for an unnamed field."},
            {NULL, NULL}
    };
//    struct_sequence_with_unnamed_fields[2].name = PyStructSequence_UnnamedField;

    PyStructSequence_Desc struct_sequence_with_unnamed_field_type_desc = {
            "module.struct_sequence_simple_with_unnamed_field",
            "Documentation.",
            struct_sequence_with_unnamed_fields,
            2,
    };

    PyTypeObject *example_type = NULL;
    if (example_type == NULL) {
        example_type = PyStructSequence_NewType(&struct_sequence_with_unnamed_field_type_desc);
    }
    assert(example_type != NULL);
    /* Hmm. Refcount is 8. */
//    ref_count = Py_REFCNT(example_type);
//    assert(ref_count == 1);

    PyObject *instance = PyStructSequence_New(example_type);

    ref_count = Py_REFCNT(instance);
    assert(ref_count == 1);

    /* Get an unset item. */
    PyObject *get_item = NULL;
    get_item = PyStructSequence_GetItem(instance, 0);
    assert(get_item == NULL);

    /* Now set items. */
    PyObject *set_item = NULL;
    set_item = new_unique_string(__FUNCTION__, "NAME");
    PyStructSequence_SetItem(instance, 0, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);
    set_item = new_unique_string(__FUNCTION__, "GENDER");
    PyStructSequence_SetItem(instance, 1, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    fprintf(stdout, "Calling PyObject_Print(instance, stdout, 0);\n");
    PyObject_Print(instance, stdout, 0);
    fprintf(stdout, "\n");
//    if (PyErr_Occurred()) {
//        PyErr_Print();
//    }
    assert(!PyErr_Occurred());
    fprintf(stdout, "Calling PyObject_Print(instance, stdout, Py_PRINT_RAW);\n");
    PyObject_Print(instance, stdout, Py_PRINT_RAW);
    printf("\n");
//    if (PyErr_Occurred()) {
//        PyErr_Print();
//    }
    assert(!PyErr_Occurred());
    fprintf(stdout, "Calling PyObject_Print DONE\n");

    /* Get items. */
    get_item = PyStructSequence_GetItem(instance, 0);
    assert(get_item != NULL);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);
    get_item = PyStructSequence_GetItem(instance, 1);
    assert(get_item != NULL);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());

    /* Clean up. */
    Py_DECREF(instance);
    Py_DECREF(example_type);
}

#pragma mark - Code that sefgfaults

#if ACCEPT_SIGSEGV

void dbg_PyTuple_SetItem_SIGSEGV_on_same_value(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
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

    int result = PyTuple_SetItem(container, 0, value);
    assert(result == 0);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyObject *get_value = PyTuple_GetItem(container, 0);
    assert(get_value == value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    /* This causes value to be free'd. */
    result = PyTuple_SetItem(container, 0, value);
    assert(result == 0);
    ref_count = Py_REFCNT(value);
    assert(ref_count != 1);

    fprintf(stderr, "%s(): Undefined behaviour, possible SIGSEGV %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    /* This may cause a SIGSEGV. */
    Py_DECREF(container);
    fprintf(stderr, "%s(): SIGSEGV did not happen %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
}

void dbg_PyList_SetItem_SIGSEGV_on_same_value(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(1);
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    int result = PyList_SetItem(container, 0, value);
    assert(result == 0);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    PyObject *get_value = PyList_GetItem(container, 0);
    assert(get_value == value);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    /* This causes value to be free'd. */
    result = PyList_SetItem(container, 0, value);
    assert(result == 0);
    ref_count = Py_REFCNT(value);
    assert(ref_count != 1);

    fprintf(stderr, "%s(): Undefined behaviour, possible SIGSEGV %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    /* This may cause a SIGSEGV. */
    Py_DECREF(container);
    fprintf(stderr, "%s(): SIGSEGV did not happen %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
}

void dbg_PyDict_SetItem_SIGSEGV_on_key_NULL(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = NULL;
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    fprintf(stderr, "%s(): PyDict_SetItem() with NULL key causes SIGSEGV %s#%d:\n",
            __FUNCTION__, __FILE_NAME__, __LINE__);
    int result = PyDict_SetItem(container, key, value);
    fprintf(stderr, "%s(): SIGSEGV did not happen %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    if (result) {
        assert(PyErr_Occurred());
        fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print(); /* Clears the error. */
    } else {
        assert(0);
    }
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    Py_DECREF(container);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
}

void dbg_PyDict_SetItem_SIGSEGV_on_value_NULL(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    PyObject *value = NULL;

    fprintf(stderr, "%s(): PyDict_SetItem() with NULL value causes SIGSEGV %s#%d:\n",
            __FUNCTION__, __FILE_NAME__, __LINE__);
    int result = PyDict_SetItem(container, key, value);
    fprintf(stderr, "%s(): SIGSEGV did not happen %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    if (result) {
        assert(PyErr_Occurred());
        fprintf(stderr, "%s(): PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print(); /* Clears the error. */
    } else {
        assert(0);
    }
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);

    Py_DECREF(container);
    Py_DECREF(key);

    assert(!PyErr_Occurred());
}

void dbg_PyDict_GetItem_key_NULL(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;
    PyObject *get_item;

    PyObject *container = PyDict_New();
    assert(container);

    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);

    PyObject *key = NULL;

    // No Key in the dictionary, no exception set.
    assert(!PyErr_Occurred());
    get_item = PyDict_GetItem(container, key);
    assert(get_item == NULL);
    assert(!PyErr_Occurred());

    Py_DECREF(container);

    assert(!PyErr_Occurred());
}

#endif // ACCEPT_SIGSEGV
