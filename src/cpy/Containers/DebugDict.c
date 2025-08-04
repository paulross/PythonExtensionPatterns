//
// Created by Paul Ross on 01/08/2025.
//
#define PPY_SSIZE_T_CLEAN

#include "Python.h"

#include "DebugDict.h"
/* For access to new_unique_string().*/
#include "pyextpatt_util.h"

#ifndef __FILE_NAME__
#define __FILE_NAME__ "DebugDict.c"
#endif

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

/**
 * See: https://docs.python.org/3/c-api/dict.html#c.PyDict_Next
 */
void dbg_PyDict_Next(void) {
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

    /* Populate the dictionary with four key/values. */
    for (int i = 0; i < 4; ++i) {
        PyObject *key = new_unique_string(__FUNCTION__, NULL);
        ref_count = Py_REFCNT(key);
        assert(ref_count == 1);

        // No Key in the dictionary, no exception set.
        assert(!PyErr_Occurred());
        get_item = PyDict_GetItem(container, key);
        assert(get_item == NULL);
        assert(!PyErr_Occurred());

        // Set a key/value
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
        Py_DECREF(key);
        Py_DECREF(value);
    }

    PyObject *key, *value;
    Py_ssize_t pos = 0;

    while (PyDict_Next(container, &pos, &key, &value)) {
        ref_count = Py_REFCNT(key);
        assert(ref_count == 1);
        ref_count = Py_REFCNT(value);
        assert(ref_count == 1);
    }
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
    assert(PyDict_GET_SIZE(container) == 0);

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);


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

#pragma mark - Dictionaries - other
/** Checks using PyDict_Merge with two dicts with one item in each.
 * The keys match, the values do not.
 * PyDict_Merge is called with override=0.
 */
void dbg_PyDict_Merge_with_match_no_override(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    /* Set up dict A. */
    PyObject *dict_a = PyDict_New();
    assert(dict_a);
    ref_count = Py_REFCNT(dict_a);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_a) == 0);
    PyObject *key_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_a, key_a, value_a)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_a) == 1);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);

    /* Set up dict B. Same key, different value. */
    PyObject *dict_b = PyDict_New();
    assert(dict_b);
    ref_count = Py_REFCNT(dict_b);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_b) == 0);
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_b, key_a, value_b)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_b) == 1);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);

    /* Now merge with override=0. */
    if (PyDict_Merge(dict_a, dict_b, 0)) {
        assert(0);
    }
    assert(!PyErr_Occurred());
    assert(PyDict_GET_SIZE(dict_a) == 1);
    assert(PyDict_GET_SIZE(dict_b) == 1);

    /* Check reference counts. key_a 3, value_a 2, value_b 2.*/
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);

    /* Check GetItem is value_a */
    assert(!PyErr_Occurred());
    PyObject *get_item = PyDict_GetItem(dict_a, key_a);
    assert(get_item == value_a);
    assert(!PyErr_Occurred());

    /* Clean up. */
    Py_DECREF(dict_a);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);

    Py_DECREF(dict_b);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 1);

    Py_DECREF(key_a);
    Py_DECREF(value_a);
    Py_DECREF(value_b);

    assert(!PyErr_Occurred());
}

/** Checks using PyDict_Merge with two dicts with one item in each.
 * The keys match, the values do not.
 * PyDict_Merge is called with override=1.
 */
void dbg_PyDict_Merge_with_match_with_override(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    /* Set up dict A. */
    PyObject *dict_a = PyDict_New();
    assert(dict_a);
    ref_count = Py_REFCNT(dict_a);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_a) == 0);
    PyObject *key_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_a, key_a, value_a)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_a) == 1);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);

    /* Set up dict B. Same key, different value. */
    PyObject *dict_b = PyDict_New();
    assert(dict_b);
    ref_count = Py_REFCNT(dict_b);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_b) == 0);
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_b, key_a, value_b)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_b) == 1);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);

    /* Now merge with override=1. */
    if (PyDict_Merge(dict_a, dict_b, 1)) {
        assert(0);
    }
    assert(!PyErr_Occurred());
    assert(PyDict_GET_SIZE(dict_a) == 1);
    assert(PyDict_GET_SIZE(dict_b) == 1);

    /* Check reference counts. key_a 3, value_a 1, value_b 3.
     * value_a has been decref'd as it is replaced.
     * Compare with dbg_PyDict_Merge_with_match_no_override() above. */
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 3);

    /* Check GetItem is value_b */
    assert(!PyErr_Occurred());
    PyObject *get_item = PyDict_GetItem(dict_a, key_a);
    assert(get_item == value_b);
    assert(!PyErr_Occurred());

    /* Clean up. */
    Py_DECREF(dict_a);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);

    Py_DECREF(dict_b);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 1);

    Py_DECREF(key_a);
    Py_DECREF(value_a);
    Py_DECREF(value_b);

    assert(!PyErr_Occurred());
}

/** Checks using PyDict_Merge with two dicts with one item in each.
 * The keys do not match, the values do not.
 * PyDict_Merge is called with override=0.
 */
void dbg_PyDict_Merge_no_match_no_override(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    /* Set up dict A. */
    PyObject *dict_a = PyDict_New();
    assert(dict_a);
    ref_count = Py_REFCNT(dict_a);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_a) == 0);
    PyObject *key_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_a, key_a, value_a)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_a) == 1);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);

    /* Set up dict B. Same key, different value. */
    PyObject *dict_b = PyDict_New();
    assert(dict_b);
    ref_count = Py_REFCNT(dict_b);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_b) == 0);
    PyObject *key_b = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 1);
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_b, key_b, value_b)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_b) == 1);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);

    /* Now merge with override=0. */
    if (PyDict_Merge(dict_a, dict_b, 0)) {
        assert(0);
    }
    assert(!PyErr_Occurred());
    assert(PyDict_GET_SIZE(dict_a) == 2);
    assert(PyDict_GET_SIZE(dict_b) == 1);

    /* Check reference counts. */
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 3);

    /* Check GetItem is value_b */
    assert(!PyErr_Occurred());
    PyObject *get_item = NULL;
    get_item = PyDict_GetItem(dict_a, key_a);
    assert(get_item == value_a);
    get_item = PyDict_GetItem(dict_a, key_b);
    assert(get_item == value_b);
    assert(!PyErr_Occurred());

    /* Clean up. */
    Py_DECREF(dict_a);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);

    Py_DECREF(dict_b);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 1);

    Py_DECREF(key_a);
    Py_DECREF(value_a);
    Py_DECREF(key_b);
    Py_DECREF(value_b);

    assert(!PyErr_Occurred());
}

/** Checks using PyDict_Merge with two dicts with one item in each.
 * The keys do not match, the values do not.
 * PyDict_Merge is called with override=1.
 */
void dbg_PyDict_Merge_no_match_with_override(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    /* Set up dict A. */
    PyObject *dict_a = PyDict_New();
    assert(dict_a);
    ref_count = Py_REFCNT(dict_a);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_a) == 0);
    PyObject *key_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_a, key_a, value_a)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_a) == 1);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);

    /* Set up dict B. Different key, different value. */
    PyObject *dict_b = PyDict_New();
    assert(dict_b);
    ref_count = Py_REFCNT(dict_b);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_b) == 0);
    PyObject *key_b = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 1);
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_b, key_b, value_b)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_b) == 1);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);

    /* Now merge with override=1. */
    if (PyDict_Merge(dict_a, dict_b, 1)) {
        assert(0);
    }
    assert(!PyErr_Occurred());
    assert(PyDict_GET_SIZE(dict_a) == 2);
    assert(PyDict_GET_SIZE(dict_b) == 1);

    /* Check reference counts. */
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 3);

    /* Check GetItem is value_b */
    assert(!PyErr_Occurred());
    PyObject *get_item = NULL;
    get_item = PyDict_GetItem(dict_a, key_a);
    assert(get_item == value_a);
    get_item = PyDict_GetItem(dict_a, key_b);
    assert(get_item == value_b);
    assert(!PyErr_Occurred());

    /* Clean up. */
    Py_DECREF(dict_a);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 2);

    Py_DECREF(dict_b);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_b);
    assert(ref_count == 1);

    Py_DECREF(key_a);
    Py_DECREF(value_a);
    Py_DECREF(key_b);
    Py_DECREF(value_b);

    assert(!PyErr_Occurred());
}

/** Checks using PyDict_Merge with two dicts with one item in each.
 * The keys do not match, the values do match.
 * PyDict_Merge is called with override=0.
 */
void dbg_PyDict_Merge_no_match_no_override_same_value(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    /* Set up dict A. */
    PyObject *dict_a = PyDict_New();
    assert(dict_a);
    ref_count = Py_REFCNT(dict_a);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_a) == 0);
    PyObject *key_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_a, key_a, value_a)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_a) == 1);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);

    /* Set up dict B. Different key, same value. */
    PyObject *dict_b = PyDict_New();
    assert(dict_b);
    ref_count = Py_REFCNT(dict_b);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_b) == 0);
    PyObject *key_b = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_b, key_b, value_a)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_b) == 1);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 3);

    /* Now merge with override=0. */
    if (PyDict_Merge(dict_a, dict_b, 0)) {
        assert(0);
    }
    assert(!PyErr_Occurred());
    assert(PyDict_GET_SIZE(dict_a) == 2);
    assert(PyDict_GET_SIZE(dict_b) == 1);

    /* Check reference counts. */
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 4);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 3);

    /* Check GetItem is value_b */
    assert(!PyErr_Occurred());
    PyObject *get_item = NULL;
    get_item = PyDict_GetItem(dict_a, key_a);
    assert(get_item == value_a);
    get_item = PyDict_GetItem(dict_a, key_b);
    assert(get_item == value_a);
    assert(!PyErr_Occurred());

    /* Clean up. */
    Py_DECREF(dict_a);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 2);

    Py_DECREF(dict_b);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(key_b);
    assert(ref_count == 1);

    Py_DECREF(key_a);
    Py_DECREF(value_a);
    Py_DECREF(key_b);

    assert(!PyErr_Occurred());
}

/** Checks using PyDict_Merge with two dicts with one item in each.
 * The dictionaries are identical.
 * PyDict_Merge is called with override=0.
 */
void dbg_PyDict_Merge_identical_no_override(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    /* Set up dict A. */
    PyObject *dict_a = PyDict_New();
    assert(dict_a);
    ref_count = Py_REFCNT(dict_a);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_a) == 0);
    PyObject *key_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_a, key_a, value_a)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_a) == 1);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);

    /* Set up dict B. Same key, same value. */
    PyObject *dict_b = PyDict_New();
    assert(dict_b);
    ref_count = Py_REFCNT(dict_b);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_b) == 0);
    if (PyDict_SetItem(dict_b, key_a, value_a)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_b) == 1);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 3);

    /* Now merge with override=0. */
    if (PyDict_Merge(dict_a, dict_b, 0)) {
        assert(0);
    }
    assert(!PyErr_Occurred());
    assert(PyDict_GET_SIZE(dict_a) == 1);
    assert(PyDict_GET_SIZE(dict_b) == 1);

    /* Check reference counts. */
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 3);

    /* Check GetItem is value_a */
    assert(!PyErr_Occurred());
    PyObject *get_item = NULL;
    get_item = PyDict_GetItem(dict_a, key_a);
    assert(get_item == value_a);
    assert(!PyErr_Occurred());

    /* Clean up. */
    Py_DECREF(dict_a);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);

    Py_DECREF(dict_b);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);

    Py_DECREF(key_a);
    Py_DECREF(value_a);

    assert(!PyErr_Occurred());
}

/** Checks using PyDict_Merge with two dicts with one item in each.
 * The dictionaries are identical.
 * PyDict_Merge is called with override=1.
 */
void dbg_PyDict_Merge_identical_with_override(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    /* Set up dict A. */
    PyObject *dict_a = PyDict_New();
    assert(dict_a);
    ref_count = Py_REFCNT(dict_a);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_a) == 0);
    PyObject *key_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict_a, key_a, value_a)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_a) == 1);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);

    /* Set up dict B. Same key, same value. */
    PyObject *dict_b = PyDict_New();
    assert(dict_b);
    ref_count = Py_REFCNT(dict_b);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict_b) == 0);
    if (PyDict_SetItem(dict_b, key_a, value_a)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict_b) == 1);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 3);

    /* Now merge with override=0. */
    if (PyDict_Merge(dict_a, dict_b, 1)) {
        assert(0);
    }
    assert(!PyErr_Occurred());
    assert(PyDict_GET_SIZE(dict_a) == 1);
    assert(PyDict_GET_SIZE(dict_b) == 1);

    /* Check reference counts. */
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 3);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 3);

    /* Check GetItem is value_a */
    assert(!PyErr_Occurred());
    PyObject *get_item = NULL;
    get_item = PyDict_GetItem(dict_a, key_a);
    assert(get_item == value_a);
    assert(!PyErr_Occurred());

    /* Clean up. */
    Py_DECREF(dict_a);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 2);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 2);

    Py_DECREF(dict_b);
    ref_count = Py_REFCNT(key_a);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value_a);
    assert(ref_count == 1);

    Py_DECREF(key_a);
    Py_DECREF(value_a);

    assert(!PyErr_Occurred());
}

/**
 * Trys self merge. Nothing happens as dict_merge() tests if the two arguments are
 * the same and does nothing if they are.
 * There is a test in dict_merge() in dictobject.c
 * if (other == mp || other->ma_used == 0) return 0;
 */
void dbg_PyDict_Merge_same_dict(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    /* Set up dict A. */
    PyObject *dict = PyDict_New();
    assert(dict);
    ref_count = Py_REFCNT(dict);
    assert(ref_count == 1);
    assert(PyDict_GET_SIZE(dict) == 0);
    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);
    if (PyDict_SetItem(dict, key, value)) {
        assert(0);
    }
    assert(PyDict_GET_SIZE(dict) == 1);
    Py_DECREF(key);
    Py_DECREF(value);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    ref_count = Py_REFCNT(value);
    assert(ref_count == 1);

    if (PyDict_Merge(dict, dict, 0)) {
        assert(0);
    }
    if (PyDict_Merge(dict, dict, 1)) {
        assert(0);
    }
    Py_DECREF(dict);
    assert(!PyErr_Occurred());
}

#pragma mark - Code that sefgfaults

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
