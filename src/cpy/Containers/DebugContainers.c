//
// Created by Paul Ross on 15/12/2024.
//
#define PPY_SSIZE_T_CLEAN

#include "Python.h"

#include "DebugContainers.h"

/* This is used to guarantee that Python is not caching a string value when we want to check the
 * reference counts after each string creation.
 * */
static long debug_test_count = 0L;

PyObject *
new_unique_string(const char *function_name, const char *suffix) {
    PyObject *value =  NULL;
    if (suffix){
        value = PyUnicode_FromFormat("%s-%s-%ld", function_name, suffix, debug_test_count);
    } else {
        value = PyUnicode_FromFormat("%s-%ld", function_name, debug_test_count);
    }
    /* To view in the debugger. */
    Py_UCS1 *buffer = PyUnicode_1BYTE_DATA(value);
    assert(buffer);
    ++debug_test_count;
    return value;
}

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
 *
 * This DOES leak an existing value contrary to the Python documentation.
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

#pragma mark - Dictionaries

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

#if ACCEPT_SIGSEGV

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

    Py_DECREF(container);
    Py_DECREF(key);

    assert(!PyErr_Occurred());
}

#endif

/**
 * TODO:
 *
 * We should cover named tuples/dataclasses etc.:
 * https://docs.python.org/3/c-api/tuple.html#struct-sequence-objects
 *
 */

