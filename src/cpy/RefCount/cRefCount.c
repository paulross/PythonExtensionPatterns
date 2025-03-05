//
// Created by Paul Ross on 20/10/2024.
//
// This explores reference counts with the Python C-API.
#define PPY_SSIZE_T_CLEAN

#include "Python.h"

/* For access to new_unique_string().*/
#include "pyextpatt_util.h"

#define CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(return_value)       \
do {                                                            \
    if (PyErr_Occurred()) {                                     \
        fprintf(stderr, "%s(): %s#%d entered with error.\n",    \
                __FUNCTION__, __FILE_NAME__, __LINE__);         \
        /* PyErr_Print(); */                                    \
        return return_value;                                    \
    }                                                           \
} while(0)


/**
 * Decrement the reference counts of each set value by one.
 *
 * @param op The set.
 * @return 0 on success, non-zero on failure in which case a Python Exception will have been set.
 */
static int
decref_set_values(PyObject *op) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(-1);
    assert(!PyErr_Occurred());

    if (!PySet_Check(op)) {
        PyErr_Format(PyExc_ValueError, "Argument must be type set not type %s", Py_TYPE(op)->tp_name);
        return 1;
    }
    /* https://docs.python.org/3/c-api/object.html#c.PyObject_GetIter
     * This returns a new reference. */
    PyObject *iterator = PyObject_GetIter(op);
    if (iterator == NULL) {
        PyErr_Format(PyExc_ValueError, "Can not obtain iterator for type %s", Py_TYPE(op)->tp_name);
        return 2;
    }
    PyObject *item;
    /* https://docs.python.org/3/c-api/iter.html#c.PyIter_Next
     * This returns a new reference. */
    while ((item = PyIter_Next(iterator))) {
        Py_DECREF(item); /* This is the point of this function. */
        Py_DECREF(item); /* As this is a new reference. */
    }
    Py_DECREF(iterator);
    if (PyErr_Occurred()) {
        return 3;
    }
    return 0;
}

/**
 * Checks the reference counts when creating and adding to a \c tuple.
 *
 * @param _unused_module
 * @return Zero on success, non-zero on error.
 */
static PyObject *
tuple_steals(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long result = 0;
    PyObject *container = PyTuple_New(1);
    if (container->ob_refcnt != 1) {
        result |= 1 << 0;
    }
//    fprintf(stdout, "TRACE: tuple->ob_refcnt = %ld result %ld\n", tuple->ob_refcnt, result);
    PyObject *value = PyLong_FromLong(123456);
    if (value->ob_refcnt != 1) {
        result |= 1 << 1;
    }
//    fprintf(stdout, "TRACE: value->ob_refcnt = %ld result %ld\n", value->ob_refcnt, result);
    PyTuple_SET_ITEM(container, 0, value);
    result |= value->ob_refcnt != 1;
    if (value->ob_refcnt != 1) {
        result |= 1 << 2;
    }
//    fprintf(stdout, "TRACE: value->ob_refcnt = %ld result %ld\n", value->ob_refcnt, result);
    if (PyTuple_GET_ITEM(container, 0)->ob_refcnt != 1) {
        result |= 1 << 3;
    }
//    fprintf(stdout, "TRACE: value->ob_refcnt = %ld result %ld\n", PyTuple_GET_ITEM(tuple, 0)->ob_refcnt, result);
    Py_DECREF(container);
    assert(!PyErr_Occurred());
    return PyLong_FromLong(result);
}

/**
 * Checks the reference counts when creating a \c tuple with \c Py_BuildValue.
 *
 * @param _unused_module
 * @return Zero on success, non-zero on error.
 */
static PyObject *
tuple_buildvalue_steals(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    int result = 0;
    PyObject *value_0 = PyLong_FromLong(123456);
    if (value_0->ob_refcnt != 1) {
        result |= 1 << 0;
    }
    PyObject *value_1 = PyLong_FromLong(1234567);
    if (value_1->ob_refcnt != 1) {
        result |= 1 << 1;
    }
    PyObject *container = Py_BuildValue("ii", value_0, value_1);
    if (container->ob_type != &PyTuple_Type) {
        result |= 1 << 2;
    }
    if (container->ob_refcnt != 1) {
        result |= 1 << 3;
    }
    result |= value_0->ob_refcnt != 1;
    if (value_0->ob_refcnt != 1) {
        result |= 1 << 4;
    }
    result |= value_1->ob_refcnt != 1;
    if (value_1->ob_refcnt != 1) {
        result |= 1 << 5;
    }
    if (PyTuple_GET_ITEM(container, 0)->ob_refcnt != 1) {
        result |= 1 << 6;
    }
    if (PyTuple_GET_ITEM(container, 1)->ob_refcnt != 1) {
        result |= 1 << 7;
    }
    Py_DECREF(container);
    assert(!PyErr_Occurred());
    return PyLong_FromLong(result);
}

/**
 * Checks the reference counts when creating and adding to a \c list.
 *
 * @param _unused_module
 * @return Zero on success, non-zero on error.
 */
static PyObject *
list_steals(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long result = 0;
    PyObject *container = PyList_New(1);
    if (container->ob_refcnt != 1) {
        result |= 1 << 0;
    }
    PyObject *value = PyLong_FromLong(123456);
    if (value->ob_refcnt != 1) {
        result |= 1 << 1;
    }
    PyList_SET_ITEM(container, 0, value);
    if (value->ob_refcnt != 1) {
        result |= 1 << 2;
    }
    if (PyList_GET_ITEM(container, 0)->ob_refcnt != 1) {
        result |= 1 << 3;
    }
    Py_DECREF(container);
    assert(!PyErr_Occurred());
    return PyLong_FromLong(result);
}

/**
 * Checks the reference counts when creating a \c list with \c Py_BuildValue.
 *
 * @param _unused_module
 * @return Zero on success, non-zero on error.
 */
static PyObject *
list_buildvalue_steals(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    int result = 0;
    PyObject *value_0 = PyLong_FromLong(123456);
    if (value_0->ob_refcnt != 1) {
        result |= 1 << 0;
    }
    PyObject *value_1 = PyLong_FromLong(1234567);
    if (value_1->ob_refcnt != 1) {
        result |= 1 << 1;
    }
    PyObject *container = Py_BuildValue("[ii]", value_0, value_1);
    if (container->ob_type != &PyList_Type) {
        result |= 1 << 2;
    }
    if (container->ob_refcnt != 1) {
        result |= 1 << 3;
    }
    result |= value_0->ob_refcnt != 1;
    if (value_0->ob_refcnt != 1) {
        result |= 1 << 4;
    }
    result |= value_1->ob_refcnt != 1;
    if (value_1->ob_refcnt != 1) {
        result |= 1 << 5;
    }
    if (PyList_GET_ITEM(container, 0)->ob_refcnt != 1) {
        result |= 1 << 6;
    }
    if (PyList_GET_ITEM(container, 1)->ob_refcnt != 1) {
        result |= 1 << 7;
    }
    Py_DECREF(container);
    assert(!PyErr_Occurred());
    return PyLong_FromLong(result);
}

/**
 * Checks the reference counts when creating and adding to a \c set.
 *
 * The \c set object *does* increment the reference count.
 * @param _unused_module
 * @return Zero on success, non-zero on error.
 */
static PyObject *
set_no_steals(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long result = 0;
    PyObject *container = PySet_New(NULL);
    if (container->ob_refcnt != 1) {
        result |= 1 << 0;
    }
    PyObject *value = PyLong_FromLong(123456);
    if (value->ob_refcnt != 1) {
        result |= 1 << 1;
    }
    PySet_Add(container, value);
    if (value->ob_refcnt != 2) {
        result |= 1 << 2;
    }
    if (PySet_Size(container) != 1) {
        result |= 1 << 3;
    }
    PyObject *pop = PySet_Pop(container);
    if (pop->ob_refcnt != 2) {
        result |= 1 << 4;
    }
    if (pop != value) {
        result |= 1 << 5;
    }
    Py_DECREF(container);
    if (value->ob_refcnt != 2) {
        result |= 1 << 6;
    }
    Py_DECREF(value);
    Py_DECREF(value);
    assert(!PyErr_Occurred());
    return PyLong_FromLong(result);
}

/**
 * Checks the reference counts when creating and adding to a \c set.
 * This uses \c decref_set_values().
 *
 * The \c set object *does* increment the reference count.
 * @param _unused_module
 * @return Zero on success, non-zero on error.
 */
static PyObject *
set_no_steals_decref(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long result = 0;
    PyObject *container = PySet_New(NULL);
    if (container->ob_refcnt != 1) {
        result |= 1 << 0;
    }
    PyObject *value = PyLong_FromLong(123456);
    if (value->ob_refcnt != 1) {
        result |= 1 << 1;
    }
    PySet_Add(container, value);
    if (value->ob_refcnt != 2) {
        result |= 1 << 2;
    }
    if (PySet_Size(container) != 1) {
        result |= 1 << 3;
    }
    // Use decref_set_values()
    if (decref_set_values(container)) {
        result |= 1 << 4;
    }
    if (value->ob_refcnt != 1) {
        result |= 1 << 5;
    }
    PyObject *pop = PySet_Pop(container);
    if (pop->ob_refcnt != 1) {
        result |= 1 << 6;
    }
    if (PySet_Size(container) != 0) {
        result |= 1 << 6;
    }
    Py_DECREF(container);
    Py_DECREF(value);
    assert(!PyErr_Occurred());
    return PyLong_FromLong(result);
}

/**
 * Checks the reference counts when creating and adding to a \c dict.
 * The \c dict object *does* increment the reference count for the key and the value.
 *
 * @param _unused_module
 * @return Zero on success, non-zero on error.
 */
static PyObject *
dict_no_steals(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long result = 0;
    int result_shift = 0;
    // Create the container
    PyObject *container = PyDict_New();
    if (container->ob_refcnt != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    // Create the key and value.
    PyObject *key = PyLong_FromLong(123456);
    if (key->ob_refcnt != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    PyObject *value = PyLong_FromLong(1234567);
    if (value->ob_refcnt != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    // Set the key and value.
    PyDict_SetItem(container, key, value);
    // Check the container size.
    if (PyDict_Size(container) != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    // Check the key and value have incremented reference counts.
    if (key->ob_refcnt != 2) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    if (value->ob_refcnt != 2) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    // Delete the key/value.
    if (PyDict_DelItem(container, key)) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    // Check the key and value have decremented reference counts.
    if (key->ob_refcnt != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    if (value->ob_refcnt != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    // Clean up.
    Py_DECREF(key);
    Py_DECREF(value);
    Py_DECREF(container);
    assert(!PyErr_Occurred());
    return PyLong_FromLong(result);
}

/**
 * Checks the reference counts when creating and adding to a \c dict.
 * The \c dict object *does* increment the reference count for the key and the value.
 * This demonstrates the canonical way of decrementing new objects immediately after calling PyDict_Set().
 *
 * @param _unused_module
 * @return Zero on success, non-zero on error.
 */
static PyObject *
dict_no_steals_decref_after_set(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long result = 0;
    int result_shift = 0;
    // Create the container
    PyObject *container = PyDict_New();
    if (container->ob_refcnt != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    // Create the key and value.
    PyObject *key = PyLong_FromLong(123456);
    if (key->ob_refcnt != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    PyObject *value = PyLong_FromLong(1234567);
    if (value->ob_refcnt != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    // Set the key and value.
    PyDict_SetItem(container, key, value);
    // Check the container size.
    if (PyDict_Size(container) != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    if (key->ob_refcnt != 2) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    if (value->ob_refcnt != 2) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    // Now decrement the newly created objects
    Py_DECREF(key);
    // Check the key and value have single reference counts.
    if (key->ob_refcnt != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    Py_DECREF(value);
    if (value->ob_refcnt != 1) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    // Delete the key/value.
    if (PyDict_DelItem(container, key)) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    if (PyDict_Size(container) != 0) {
        result |= 1 << result_shift;
    }
    ++result_shift;
    // Clean up.
    Py_DECREF(container);
    assert(!PyErr_Occurred());
    return PyLong_FromLong(result);
}

/**
 * Checks the reference counts when creating a \c dict with \c Py_BuildValue.
 * The \c dict object *does* increment the reference count for the key and the value.
 *
 * @param _unused_module
 * @return Zero on success, non-zero on error.
 */
static PyObject *
dict_buildvalue_no_steals(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    int result = 0;
    int result_shift = 0;
    PyObject *key = PyLong_FromLong(123456);
    if (key->ob_refcnt != 1) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;
    PyObject *value = PyLong_FromLong(1234567);
    if (value->ob_refcnt != 1) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;
    // Build the dict
    PyObject *container = Py_BuildValue("{OO}", key, value);
    if (container == NULL) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;
    // Check the container type.
    if (container->ob_type != &PyDict_Type) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;
    // Check the container reference count.
    if (container->ob_refcnt != 1) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;
    // Check the container size.
    if (PyDict_Size(container) != 1) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;

//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() key->ob_refcnt=%ld value->ob_refcnt=%ld\n", key->ob_refcnt, value->ob_refcnt);
    // Check the key and value have incremented reference counts.
    if (key->ob_refcnt != 2) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;
    if (value->ob_refcnt != 2) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;

//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() key=%ld value=%ld\n", (long)key, (long)value);
//    PyObject_Print(container, stdout, Py_PRINT_RAW);
//    fprintf(stdout, "\n");

    // Check the container has the key.
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() PyDict_Contains(container, key) %d\n", PyDict_Contains(container, key));
    if (PyDict_Contains(container, key) != 1) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;
    // Delete the key/value.
    if (PyDict_DelItem(container, key)) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;
    // Check the key and value have decremented reference counts.
    if (key->ob_refcnt != 1) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;
    if (value->ob_refcnt != 1) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
//    fprintf(stdout, "TRACE: dict_buildvalue_no_steals() result_shift=%d result %d\n", result_shift, result);
    ++result_shift;
    // Check the container size.
    if (PyDict_Size(container) != 0) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
    ++result_shift;
    // Check the container does not have the key.
    if (PyDict_Contains(container, key) != 0) {
        result |= 1 << result_shift;
        return PyLong_FromLong(result);
    }
    ++result_shift;
    // Clean up.
    Py_DECREF(key);
    Py_DECREF(value);
    Py_DECREF(container);
    assert(!PyErr_Occurred());
    return PyLong_FromLong(result);
}

#define TEST_REF_COUNT_THEN_OR_RETURN_VALUE(variable, expected, commentary)                             \
    do  {                                                                                               \
        Py_ssize_t _ref_count = Py_REFCNT(variable);                                                    \
        if (_ref_count != expected) {                                                                   \
            fprintf(                                                                                    \
                stderr,                                                                                 \
                "Py_REFCNT(%s) != %ld but %ld. Test: %d Commentary: %s File: %s Line: %d\n",            \
                #variable, expected, _ref_count, error_flag_position, commentary, __FILE__, __LINE__    \
            );                                                                                          \
            return_value |= 1 << error_flag_position;                                                   \
        }                                                                                               \
        error_flag_position++;                                                                          \
    } while (0)


#pragma mark - Teting Tuples

/**
 * A function that checks whether a tuple steals a reference when using PyTuple_SetItem.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 *
 * @param _unused_module
 * @return 0 on success.
 */
static PyObject *
test_PyTuple_SetItem_steals(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    Py_ssize_t ref_count;
    PyObject *get_item = NULL;

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "PyTuple_New()");

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "value = new_unique_string(__FUNCTION__, NULL)");

    if (PyTuple_SetItem(container, 0, value)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "value after PyTuple_SetItem()");

    get_item = PyTuple_GET_ITEM(container, 0);
    ref_count = Py_REFCNT(get_item);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyTuple_GetItem(container, 0);
    ref_count = Py_REFCNT(get_item);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);
    /* NO as container deals with this. */
    /* Py_DECREF(value); */
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

/**
 * A function that checks whether a tuple steals a reference when using PyTuple_SET_ITEM.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 *
 * @param _unused_module
 * @return 0 on success.
 */
static PyObject *
test_PyTuple_SET_ITEM_steals(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    Py_ssize_t ref_count;

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(container);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    PyTuple_SET_ITEM(container, 0, value);

    ref_count = Py_REFCNT(value);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    PyObject *get_item = PyTuple_GET_ITEM(container, 0);
    ref_count = Py_REFCNT(get_item);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);
    /* NO as container deals with this. */
    /* Py_DECREF(value); */
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

/**
 * A function that checks whether a tuple steals a reference when using PyTuple_SetItem on an existing item.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 *
 * This DOES leak an existing value contrary to the Python documentation.
 *
 * @param _unused_module
 * @return None
 */
static PyObject *
test_PyTuple_SetItem_steals_replace(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    Py_ssize_t ref_count;
    PyObject *get_item = NULL;

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(container);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_0);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    if (PyTuple_SetItem(container, 0, value_0)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(value_0);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyTuple_GET_ITEM(container, 0);
    if (get_item != value_0) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;
    ref_count = Py_REFCNT(get_item);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Now create a new value that will overwrite the old one. */
    PyObject *value_1 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_1);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Preserve the value_0 as this reference count is about to be decremented. */
    Py_INCREF(value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 2);

    /* Preserve the value_1 so that we can see Py_DECREF(container) decrements it. */
    Py_INCREF(value_1);
    ref_count = Py_REFCNT(value_1);
    assert(ref_count == 2);

    /* This will overwrite value_0 leaving it with a reference count of 1.*/
    if (PyTuple_SetItem(container, 0, value_1)) {
        fprintf(stdout, "PyTuple_SetItem(container, 0, value_1)\n");
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Previous value is decremented. */
    ref_count = Py_REFCNT(value_0);
    if (ref_count != 1) {
        fprintf(stdout, "Py_REFCNT(value_0) != 1 but %ld\n", Py_REFCNT(value_0));
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(value_1);
    if (ref_count != 2) {
        fprintf(stdout, "Py_REFCNT(value_1) != 2 but %ld\n", Py_REFCNT(value_1));
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyTuple_GET_ITEM(container, 0);
    if (get_item != value_1) {
        fprintf(stdout, "get_item != value_1\n");
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(get_item);
    if (ref_count != 2) {
        fprintf(stdout, "Py_REFCNT(get_item) != 1 but %ld\n", Py_REFCNT(get_item));
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);

    ref_count = Py_REFCNT(value_1);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(value_1);

    ref_count = Py_REFCNT(value_0);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(value_0);

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

/**
 * A function that checks whether a tuple steals a reference when using PyTuple_SET_ITEM on an existing item.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 *
 * @param _unused_module
 * @return None
 */
static PyObject *
test_PyTuple_SET_ITEM_steals_replace(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "Create container.");

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_0, 1L, "Create value_0.");

    PyTuple_SET_ITEM(container, 0, value_0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_0, 1L, "PyTuple_SET_ITEM(container, 0, value_0);");

    get_item = PyTuple_GET_ITEM(container, 0);
    if (get_item != value_0) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 1L, "PyTuple_GET_ITEM(container, 0);");

    /* Now create a new value that will overwrite the old one. */
    PyObject *value_1 = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_1, 1L, "Create value_1");

    /* Preserve the value_0 as this reference count is about to be decremented. */
    Py_INCREF(value_0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_0, 2L, "Py_INCREF(value_0);");

    /* Preserve the value_1 so that we can see Py_DECREF(container) decrements it. */
    Py_INCREF(value_1);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_1, 2L, "Py_INCREF(value_1);");

    /* This will overwrite value_0 but not dec ref value_0 leaving
     * value_0 still with a reference count of 2.
     * This is a leak. */
    PyTuple_SET_ITEM(container, 0, value_1);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_0, 2L,
                                        "Py_REFCNT(value_0) after PyTuple_SET_ITEM(container, 0, value_1);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_1, 2L,
                                        "Py_REFCNT(value_1) after PyTuple_SET_ITEM(container, 0, value_1);");

    get_item = PyTuple_GET_ITEM(container, 0);
    if (get_item != value_1) {
        fprintf(stdout, "get_item != value_1\n");
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 2L, "PyTuple_GET_ITEM(container, 0);");

    Py_DECREF(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_1, 1L, "value_1 after Py_DECREF(container);");

    Py_DECREF(value_1);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_0, 2L, "value_0 after Py_DECREF(container);");
    Py_DECREF(value_0);
    Py_DECREF(value_0);

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

/**
 * Function that check the behaviour of PyTuple_SetItem() when setting the *same* value.
 * See also dbg_PyTuple_SetItem_replace_with_same() in src/cpy/Containers/DebugContainers.c
 *
 * @param _unused_module
 * @return
 */
static PyObject *
test_PyTuple_SetItem_replace_same(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyObject *container = PyTuple_New(1);");

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *value = new_unique_string(__FUNCTION__, NULL);");
    /* Set the first time. */
    if (PyTuple_SetItem(container, 0, value)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After first PyTuple_SetItem(container, 0, value);");
    /*Get and test the first item. */
    get_item = PyTuple_GET_ITEM(container, 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 1L, "After PyTuple_GET_ITEM(container, 0);");
    if (get_item != value) {
        fprintf(stderr, "get_item != value at File: %s Line: %d\n", __FILE__, __LINE__);
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Now incref the value so we can prevent a SIGSEGV with a double PyTuple_SetItem(). */
    Py_INCREF(value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "After Py_INCREF(value);");

    /* Second PyTuple_SetItem(). */
    /* This will overwrite value leaving it with a reference count of 1 if it wasn't for the Py_INCREF(value); above.*/
    if (PyTuple_SetItem(container, 0, value)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;
    /* This checks that PyTuple_SetItem() has decremented the original reference count. */
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After second PyTuple_SetItem(container, 0, value);");

    /* Check  the value is the same. */
    get_item = PyTuple_GET_ITEM(container, 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 1L, "After PyTuple_GET_ITEM(container, 0);");
    if (get_item != value) {
        fprintf(stderr, "get_item != value at File: %s Line: %d\n", __FILE__, __LINE__);
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Decref the container. value will be ree'd. Double check values reference count is 1. */
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "Before Py_DECREF(container);");
    Py_DECREF(container);

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyTuple_SET_ITEM_replace_same(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyObject *container = PyTuple_New(1);");

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *value = new_unique_string(__FUNCTION__, NULL);");
    /* Set the first time. Does not alter reference count. */
    PyTuple_SET_ITEM(container, 0, value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After first PyTuple_SET_ITEM(container, 0, value);");
    /* Get and test the first item. */
    get_item = PyTuple_GET_ITEM(container, 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 1L, "After PyTuple_GET_ITEM(container, 0);");
    if (get_item != value) {
        fprintf(stderr, "get_item != value at File: %s Line: %d\n", __FILE__, __LINE__);
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Second PyTuple_SET_ITEM(). Does not alter reference count. */
    PyTuple_SET_ITEM(container, 0, value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After second PyTuple_SET_ITEM(container, 0, value);");

    /* Check  the value is the same. */
    get_item = PyTuple_GET_ITEM(container, 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 1L, "After PyTuple_GET_ITEM(container, 0);");
    if (get_item != value) {
        fprintf(stderr, "get_item != value at File: %s Line: %d\n", __FILE__, __LINE__);
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_INCREF(value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "Before Py_DECREF(container);");
    /* Decref the container. value will be decref'd. Double check values reference count is 1. */
    Py_DECREF(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After Py_DECREF(container);");

    /* Clean up. */
    Py_DECREF(value);

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyTuple_SetItem_NULL(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "Create container.");

    PyTuple_SetItem(container, 0, NULL);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyTuple_GET_ITEM(container, 0);
    if (get_item != NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyTuple_SET_ITEM_NULL(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    Py_ssize_t ref_count;
    PyObject *get_item = NULL;

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(container);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    PyTuple_SET_ITEM(container, 0, NULL);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyTuple_GET_ITEM(container, 0);
    if (get_item != NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyTuple_SetIem_NULL_SetItem(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "Create container.");

    if (PyTuple_SetItem(container, 0, NULL)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyTuple_GET_ITEM(container, 0);
    if (get_item != NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Now set a non-null value. */
    PyObject *value = new_unique_string(__FUNCTION__, NULL);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "Create value.");

    /* Increment so we can  check after deleting the container. */
    Py_INCREF(value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "Py_INCREF(value);");

    /* Set, replacing NULL. */
    if (PyTuple_SetItem(container, 0, value)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyTuple_GET_ITEM(container, 0);
    if (get_item == NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "Py_INCREF(value);");

    Py_DECREF(value);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyTuple_SET_ITEM_NULL_SET_ITEM(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "Create container.");

    PyTuple_SET_ITEM(container, 0, NULL);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyTuple_GET_ITEM(container, 0);
    if (get_item != NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Now set a non-null value. */
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "PyObject *value = new_unique_string(__FUNCTION__, NULL);.");

    /* Increment so we can  check after deleting the container. */
    Py_INCREF(value);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "PyObject *value after Py_INCREF.");

    /* Set, replacing NULL. */
    PyTuple_SET_ITEM(container, 0, value);

    get_item = PyTuple_GET_ITEM(container, 0);
    if (get_item == NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "PyObject *value after Py_DECREF(container);.");

    Py_DECREF(value);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyTuple_SetItem_fails_not_a_tuple(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());

    PyObject *container = PyList_New(1);
    if (!container) {
        return NULL;
    }
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* This should fail. */
    if (PyTuple_SetItem(container, 0, value)) {
        /* DO NOT do this, it has been done by the failure PyTuple_SetItem(). */
        /* Py_DECREF(value); */
        Py_DECREF(container);
        assert(PyErr_Occurred());
        return NULL;
    }
    Py_DECREF(value);
    Py_DECREF(container);
    PyErr_Format(PyExc_RuntimeError, "Should have raised an error.");
    return NULL;
}

static PyObject *
test_PyTuple_SetItem_fails_out_of_range(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return NULL;
    }
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* This should fail. */
    if (PyTuple_SetItem(container, 1, value)) {
        /* DO NOT do this, it has been done by the failure PyTuple_SetItem(). */
        /* Py_DECREF(value); */
        Py_DECREF(container);
        assert(PyErr_Occurred());
        return NULL;
    }
    Py_DECREF(value);
    Py_DECREF(container);
    PyErr_Format(PyExc_RuntimeError, "Should have raised an error.");
    return NULL;
}

static PyObject *
test_PyTuple_Py_PyTuple_Pack(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());

    long return_value = 0L;
    int error_flag_position = 0;

    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_a, 1L,
                                        "After PyObject *value_a = new_unique_string(__FUNCTION__, NULL);");
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_b, 1L,
                                        "After PyObject *value_b = new_unique_string(__FUNCTION__, NULL);");

    PyObject *container = PyTuple_Pack(2, value_a, value_b);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L,
                                        "After PyObject *container = PyTuple_Pack(2, value_a, value_b);");

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_a, 2L,
                                        "value_a after PyObject *container = PyTuple_Pack(2, value_a, value_b);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_b, 2L,
                                        "value_b after PyObject *container = PyTuple_Pack(2, value_a, value_b);");

    Py_DECREF(container);

    /* Leaks: */
    assert(Py_REFCNT(value_a) == 1);
    assert(Py_REFCNT(value_b) == 1);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_a, 1L, "value_a after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_b, 1L, "value_b after Py_DECREF(container);");
    /* Fix leaks: */
    Py_DECREF(value_a);
    Py_DECREF(value_b);

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyTuple_Py_BuildValue(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
//    PyObject *get_item = NULL;

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *value = new_unique_string(__FUNCTION__, NULL);");

    PyObject *container = Py_BuildValue("(O)", value);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "After PyObject *container = Py_BuildValue(\"(O)\", value);");

    assert(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "Container");

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *container = Py_BuildValue(\"(O)\", value);");

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

#pragma mark - Testing Lists

/**
 * A function that checks whether a tuple steals a reference when using PyList_SetItem.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 *
 * @param _unused_module
 * @return 0 on success.
 */
static PyObject *
test_PyList_SetItem_steals(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    Py_ssize_t ref_count;
    PyObject *get_item = NULL;

    PyObject *container = PyList_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "PyList_New()");

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "value = new_unique_string(__FUNCTION__, NULL)");

    if (PyList_SetItem(container, 0, value)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "value after PyList_SetItem()");

    get_item = PyList_GET_ITEM(container, 0);
    ref_count = Py_REFCNT(get_item);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyList_GetItem(container, 0);
    ref_count = Py_REFCNT(get_item);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);
    /* NO as container deals with this. */
    /* Py_DECREF(value); */
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

/**
 * A function that checks whether a tuple steals a reference when using PyList_SET_ITEM.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 *
 * @param _unused_module
 * @return 0 on success.
 */
static PyObject *
test_PyList_SET_ITEM_steals(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    Py_ssize_t ref_count;

    PyObject *container = PyList_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(container);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    PyList_SET_ITEM(container, 0, value);

    ref_count = Py_REFCNT(value);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    PyObject *get_item = PyList_GET_ITEM(container, 0);
    ref_count = Py_REFCNT(get_item);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);
    /* NO as container deals with this. */
    /* Py_DECREF(value); */
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

/**
 * A function that checks whether a tuple steals a reference when using PyList_SetItem on an existing item.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 *
 * This DOES leak an existing value contrary to the Python documentation.
 *
 * @param _unused_module
 * @return None
 */
static PyObject *
test_PyList_SetItem_steals_replace(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    Py_ssize_t ref_count;
    PyObject *get_item = NULL;

    PyObject *container = PyList_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(container);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_0);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    if (PyList_SetItem(container, 0, value_0)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(value_0);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyList_GET_ITEM(container, 0);
    if (get_item != value_0) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;
    ref_count = Py_REFCNT(get_item);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Now create a new value that will overwrite the old one. */
    PyObject *value_1 = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(value_1);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Preserve the value_0 as this reference count is about to be decremented. */
    Py_INCREF(value_0);
    ref_count = Py_REFCNT(value_0);
    assert(ref_count == 2);

    /* Preserve the value_1 so that we can see Py_DECREF(container) decrements it. */
    Py_INCREF(value_1);
    ref_count = Py_REFCNT(value_1);
    assert(ref_count == 2);

    /* This will overwrite value_0 leaving it with a reference count of 1.*/
    if (PyList_SetItem(container, 0, value_1)) {
        fprintf(stdout, "PyList_SetItem(container, 0, value_1)\n");
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Previous value is decremented. */
    ref_count = Py_REFCNT(value_0);
    if (ref_count != 1) {
        fprintf(stdout, "Py_REFCNT(value_0) != 1 but %ld\n", Py_REFCNT(value_0));
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(value_1);
    if (ref_count != 2) {
        fprintf(stdout, "Py_REFCNT(value_1) != 2 but %ld\n", Py_REFCNT(value_1));
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyList_GET_ITEM(container, 0);
    if (get_item != value_1) {
        fprintf(stdout, "get_item != value_1\n");
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(get_item);
    if (ref_count != 2) {
        fprintf(stdout, "Py_REFCNT(get_item) != 1 but %ld\n", Py_REFCNT(get_item));
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);

    ref_count = Py_REFCNT(value_1);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(value_1);

    ref_count = Py_REFCNT(value_0);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(value_0);

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

/**
 * A function that checks whether a tuple steals a reference when using PyList_SET_ITEM on an existing item.
 * This can be stepped through in the debugger.
 * asserts are use for the test so this is expected to be run in DEBUG mode.
 *
 * @param _unused_module
 * @return None
 */
static PyObject *
test_PyList_SET_ITEM_steals_replace(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyList_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "Create container.");

    PyObject *value_0 = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_0, 1L, "Create value_0.");

    PyList_SET_ITEM(container, 0, value_0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_0, 1L, "PyList_SET_ITEM(container, 0, value_0);");

    get_item = PyList_GET_ITEM(container, 0);
    if (get_item != value_0) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 1L, "PyList_GET_ITEM(container, 0);");

    /* Now create a new value that will overwrite the old one. */
    PyObject *value_1 = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_1, 1L, "Create value_1");

    /* Preserve the value_0 as this reference count is about to be decremented. */
    Py_INCREF(value_0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_0, 2L, "Py_INCREF(value_0);");

    /* Preserve the value_1 so that we can see Py_DECREF(container) decrements it. */
    Py_INCREF(value_1);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_1, 2L, "Py_INCREF(value_1);");

    /* This will overwrite value_0 but not dec ref value_0 leaving
     * value_0 still with a reference count of 2.
     * This is a leak. */
    PyList_SET_ITEM(container, 0, value_1);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_0, 2L,
                                        "Py_REFCNT(value_0) after PyList_SET_ITEM(container, 0, value_1);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_1, 2L,
                                        "Py_REFCNT(value_1) after PyList_SET_ITEM(container, 0, value_1);");

    get_item = PyList_GET_ITEM(container, 0);
    if (get_item != value_1) {
        fprintf(stdout, "get_item != value_1\n");
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 2L, "PyList_GET_ITEM(container, 0);");

    Py_DECREF(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_1, 1L, "value_1 after Py_DECREF(container);");

    Py_DECREF(value_1);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_0, 2L, "value_0 after Py_DECREF(container);");
    Py_DECREF(value_0);
    Py_DECREF(value_0);

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyList_SetItem_replace_same(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyList_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyObject *container = PyList_New(1);");

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *value = new_unique_string(__FUNCTION__, NULL);");
    /* Set the first time. */
    if (PyList_SetItem(container, 0, value)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After first PyList_SetItem(container, 0, value);");
    /*Get and test the first item. */
    get_item = PyList_GET_ITEM(container, 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 1L, "After PyList_GET_ITEM(container, 0);");
    if (get_item != value) {
        fprintf(stderr, "get_item != value at File: %s Line: %d\n", __FILE__, __LINE__);
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Now incref the value so we can prevent a SIGSEGV with a double PyList_SetItem(). */
    Py_INCREF(value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "After Py_INCREF(value);");

    /* Second PyList_SetItem(). */
    /* This will overwrite value leaving it with a reference count of 1 if it wasn't for the Py_INCREF(value); above.*/
    if (PyList_SetItem(container, 0, value)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;
    /* This checks that PyList_SetItem() has decremented the original reference count. */
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After second PyList_SetItem(container, 0, value);");

    /* Check  the value is the same. */
    get_item = PyList_GET_ITEM(container, 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 1L, "After PyList_GET_ITEM(container, 0);");
    if (get_item != value) {
        fprintf(stderr, "get_item != value at File: %s Line: %d\n", __FILE__, __LINE__);
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Decref the container. value will be ree'd. Double check values reference count is 1. */
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "Before Py_DECREF(container);");
    Py_DECREF(container);

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyList_SET_ITEM_replace_same(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyList_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyObject *container = PyList_New(1);");

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *value = new_unique_string(__FUNCTION__, NULL);");
    /* Set the first time. Does not alter reference count. */
    PyList_SET_ITEM(container, 0, value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After first PyList_SET_ITEM(container, 0, value);");
    /* Get and test the first item. */
    get_item = PyList_GET_ITEM(container, 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 1L, "After PyList_GET_ITEM(container, 0);");
    if (get_item != value) {
        fprintf(stderr, "get_item != value at File: %s Line: %d\n", __FILE__, __LINE__);
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Second PyList_SET_ITEM(). Does not alter reference count. */
    PyList_SET_ITEM(container, 0, value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After second PyList_SET_ITEM(container, 0, value);");

    /* Check  the value is the same. */
    get_item = PyList_GET_ITEM(container, 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 1L, "After PyList_GET_ITEM(container, 0);");
    if (get_item != value) {
        fprintf(stderr, "get_item != value at File: %s Line: %d\n", __FILE__, __LINE__);
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_INCREF(value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "Before Py_DECREF(container);");
    /* Decref the container. value will be decref'd. Double check values reference count is 1. */
    Py_DECREF(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After Py_DECREF(container);");

    /* Clean up. */
    Py_DECREF(value);

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyList_SetItem_NULL(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyList_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "Create container.");

    PyList_SetItem(container, 0, NULL);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyList_GET_ITEM(container, 0);
    if (get_item != NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyList_SET_ITEM_NULL(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    Py_ssize_t ref_count;
    PyObject *get_item = NULL;

    PyObject *container = PyList_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    ref_count = Py_REFCNT(container);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    PyList_SET_ITEM(container, 0, NULL);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyList_GET_ITEM(container, 0);
    if (get_item != NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyList_SetIem_NULL_SetItem(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyList_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "Create container.");

    if (PyList_SetItem(container, 0, NULL)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyList_GET_ITEM(container, 0);
    if (get_item != NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Now set a non-null value. */
    PyObject *value = new_unique_string(__FUNCTION__, NULL);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "Create value.");

    /* Increment so we can  check after deleting the container. */
    Py_INCREF(value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "Py_INCREF(value);");

    /* Set, replacing NULL. */
    if (PyList_SetItem(container, 0, value)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyList_GET_ITEM(container, 0);
    if (get_item == NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "Py_INCREF(value);");

    Py_DECREF(value);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyList_SET_ITEM_NULL_SET_ITEM(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyList_New(1);
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "Create container.");

    PyList_SET_ITEM(container, 0, NULL);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    get_item = PyList_GET_ITEM(container, 0);
    if (get_item != NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    /* Now set a non-null value. */
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "PyObject *value = new_unique_string(__FUNCTION__, NULL);.");

    /* Increment so we can  check after deleting the container. */
    Py_INCREF(value);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "PyObject *value after Py_INCREF.");

    /* Set, replacing NULL. */
    PyList_SET_ITEM(container, 0, value);

    get_item = PyList_GET_ITEM(container, 0);
    if (get_item == NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "PyObject *value after Py_DECREF(container);.");

    Py_DECREF(value);

    if (PyErr_Occurred()) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    assert(!PyErr_Occurred());
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyList_SetItem_fails_not_a_list(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return NULL;
    }
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* This should fail. */
    if (PyList_SetItem(container, 0, value)) {
        /* DO NOT do this, it has been done by the failure PyTuple_SetItem(). */
        /* Py_DECREF(value); */
        Py_DECREF(container);
        assert(PyErr_Occurred());
        return NULL;
    }
    Py_DECREF(value);
    Py_DECREF(container);
    PyErr_Format(PyExc_RuntimeError, "Should have raised an error.");
    return NULL;
}

static PyObject *
test_PyList_SetItem_fails_out_of_range(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());

    PyObject *container = PyList_New(1);
    if (!container) {
        return NULL;
    }
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* This should fail. */
    if (PyList_SetItem(container, 1, value)) {
        /* DO NOT do this, it has been done by the failure PyTuple_SetItem(). */
        /* Py_DECREF(value); */
        Py_DECREF(container);
        assert(PyErr_Occurred());
        return NULL;
    }
    Py_DECREF(value);
    Py_DECREF(container);
    PyErr_Format(PyExc_RuntimeError, "Should have raised an error.");
    return NULL;
}

static PyObject *
test_PyList_Append(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;

    PyObject *container = PyList_New(0);
    if (!container) {
        return NULL;
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyObject *container = PyList_New(0);");
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *value = new_unique_string(__FUNCTION__, NULL);");

    if (PyList_Append(container, value)) {
        assert(PyErr_Occurred());
        return NULL;
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "After PyList_Append(container, value);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyList_Append(container, value);");

    Py_DECREF(value);
    Py_DECREF(container);

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyList_Append_fails_not_a_list(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return NULL;
    }
    PyObject *value = new_unique_string(__FUNCTION__, NULL);

    int result = PyList_Append(container, value);
    assert(result);
    Py_DECREF(value);
    Py_DECREF(container);
    assert(PyErr_Occurred());
    return NULL;
}

static PyObject *
test_PyList_Append_fails_NULL(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());

    PyObject *container = PyList_New(0);
    if (!container) {
        return NULL;
    }

    int result = PyList_Append(container, NULL);
    assert(result);
    Py_DECREF(container);
    assert(PyErr_Occurred());
    return NULL;
}

static PyObject *
test_PyList_Insert(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;

    PyObject *container = PyList_New(0);
    if (!container) {
        return NULL;
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyObject *container = PyList_New(0);");
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *value = new_unique_string(__FUNCTION__, NULL);");

    if (PyList_Insert(container, 0L, value)) {
        assert(PyErr_Occurred());
        return NULL;
    }
    if (PyList_GET_SIZE(container) != 1) {
        Py_DECREF(container);
        Py_DECREF(value);
        return NULL;
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "After PyList_Append(container, value);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyList_Append(container, value);");

    Py_DECREF(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After Py_DECREF(container);");
    Py_DECREF(value);

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyList_Insert_Is_Truncated(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;

    PyObject *container = PyList_New(0);
    if (!container) {
        return NULL;
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyObject *container = PyList_New(0);");
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *value = new_unique_string(__FUNCTION__, NULL);");

    if (PyList_Insert(container, 4L, value)) {
        assert(PyErr_Occurred());
        return NULL;
    }
    if (PyList_GET_SIZE(container) != 1) {
        Py_DECREF(container);
        Py_DECREF(value);
        return NULL;
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "After PyList_Append(container, value);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyList_Append(container, value);");

    Py_DECREF(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After Py_DECREF(container);");
    Py_DECREF(value);

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyList_Insert_Negative_Index(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;

    PyObject *container = PyList_New(0);
    if (!container) {
        return NULL;
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyObject *container = PyList_New(0);");
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *value = new_unique_string(__FUNCTION__, NULL);");

    if (PyList_Insert(container, -1L, value)) {
        assert(PyErr_Occurred());
        return NULL;
    }
    if (PyList_GET_SIZE(container) != 1) {
        Py_DECREF(container);
        Py_DECREF(value);
        return NULL;
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "After PyList_Append(container, value);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "After PyList_Append(container, value);");

    Py_DECREF(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After Py_DECREF(container);");
    Py_DECREF(value);

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyList_Insert_fails_not_a_list(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return NULL;
    }
    PyObject *value = new_unique_string(__FUNCTION__, NULL);

    int result = PyList_Insert(container, 1L, value);
    assert(result);
    Py_DECREF(value);
    Py_DECREF(container);
    assert(PyErr_Occurred());
    return NULL;
}

static PyObject *
test_PyList_Insert_fails_NULL(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());

    PyObject *container = PyList_New(0);
    if (!container) {
        return NULL;
    }

    int result = PyList_Insert(container, 1L, NULL);
    assert(result);
    Py_DECREF(container);
    assert(PyErr_Occurred());
    return NULL;
}

static PyObject *
test_PyList_Py_BuildValue(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
//    PyObject *get_item = NULL;

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *value = new_unique_string(__FUNCTION__, NULL);");

    PyObject *container = Py_BuildValue("[O]", value);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "After PyObject *container = Py_BuildValue(\"(O)\", value);");

    assert(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "Container");

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "After PyObject *container = Py_BuildValue(\"(O)\", value);");

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

#pragma mark - Testing Dictionaries

static PyObject *
test_PyDict_SetItem_increments(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyDict_New();
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "PyDict_New()");

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "key = new_unique_string(__FUNCTION__, NULL)");
    PyObject *value_a = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_a, 1L, "value = new_unique_string(__FUNCTION__, NULL)");

    if (PyDict_SetItem(container, key, value_a)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key after PyDict_SetItem()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_a, 2L, "value_a after PyDict_SetItem()");

    get_item = PyDict_GetItem(container, key);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 2L, "get_item = PyDict_GetItem(container, key);");
    if (get_item != value_a) {
        fprintf(stderr, "get_item = PyDict_GetItem(container, key); is not value_a\n");
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    /* Now replace value_a with a new value, value_b. */
    PyObject *value_b = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_b, 1L, "value_a = new_unique_string(__FUNCTION__, NULL)");

    if (PyDict_SetItem(container, key, value_b)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key after PyDict_SetItem()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_a, 1L, "value_a after PyList_SetItem()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_b, 2L, "value_b after PyList_SetItem()");

    get_item = PyDict_GetItem(container, key);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 2L, "get_item = PyDict_GetItem(container, key);");
    if (get_item != value_b) {
        fprintf(stderr, "get_item = PyDict_GetItem(container, key); is not value_b\n");
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    // Replace with existing key/value_b. Reference counts should remain the same.
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key before PyDict_SetItem(container, key, value_b)");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_b, 2L, "value_b before PyDict_SetItem(container, key, value_b)");
    if (PyDict_SetItem(container, key, value_b)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key before PyDict_SetItem(container, key, value_b)");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_b, 2L, "value_b before PyDict_SetItem(container, key, value_b)");

    Py_DECREF(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "key after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_b, 1L, "value_b after Py_DECREF(container);");
    Py_DECREF(key);
    Py_DECREF(value_a);
    Py_DECREF(value_b);
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyDict_SetItem_fails_not_a_dict(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());

    PyObject *container = PyList_New(0);
    if (!container) {
        assert(PyErr_Occurred());
        return NULL;
    }
    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* This should fail. */
    if (PyDict_SetItem(container, key, value)) {
        Py_DECREF(container);
        Py_DECREF(key);
        Py_DECREF(value);
        assert(PyErr_Occurred());
        return NULL;
    }
    Py_DECREF(container);
    Py_DECREF(key);
    Py_DECREF(value);
    PyErr_Format(PyExc_RuntimeError, "Should have raised an error.");
    return NULL;
}

static PyObject *
test_PyDict_SetItem_fails_not_hashable(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());

    PyObject *container = PyDict_New();
    if (!container) {
        assert(PyErr_Occurred());
        return NULL;
    }
    PyObject *key = PyList_New(0);;
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* This should fail. */
    if (PyDict_SetItem(container, key, value)) {
        Py_DECREF(container);
        Py_DECREF(key);
        Py_DECREF(value);
        assert(PyErr_Occurred());
        return NULL;
    }
    Py_DECREF(container);
    Py_DECREF(key);
    Py_DECREF(value);
    PyErr_Format(PyExc_RuntimeError, "Should have raised an error.");
    return NULL;
}

static PyObject *
test_PyDict_SetDefault_default_unused(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item;

    PyObject *container = PyDict_New();
    assert(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "container after PyObject *container = PyDict_New();");

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "New key");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "New value");

    if (PyDict_SetItem(container, key, value)) {
        assert(0);
        goto finally;
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key after PyDict_SetItem()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after PyDict_SetItem()");

    get_item = PyDict_GetItem(container, key);
    assert(get_item == value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 2L, "value after PyDict_GetItem()");

    /* Now check PyDict_SetDefault() which does not use the default. */
    PyObject *value_default = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_default, 1L, "New value_default");

    get_item = PyDict_SetDefault(container, key, value_default);
    assert(get_item == value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key after PyDict_SetDefault()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after PyDict_SetDefault()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 2L, "value after PyDict_SetDefault()");

    Py_DECREF(container);

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(value);
    Py_DECREF(value_default);

    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

PyObject *
test_PyDict_SetDefault_default_used(void) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item;

    PyObject *container = PyDict_New();
    assert(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "container after PyObject *container = PyDict_New();");

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "container after PyObject *container = PyDict_New();");

    /* Do not do this so the default is invoked.
    if (PyDict_SetItem(container, key, value)) {
        assert(0);
    }
    */

    /* Now check PyDict_SetDefault() which *does* use the default. */
    PyObject *value_default = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_default, 1L, "container after PyObject *container = PyDict_New();");

    get_item = PyDict_SetDefault(container, key, value_default);
    if (!get_item) {
        assert(0);
    }
    assert(PyDict_Size(container) == 1);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key after PyDict_SetDefault()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_default, 2L, "value_default after PyDict_SetDefault()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 2L, "get_item after PyDict_SetDefault()");
    assert(get_item == value_default);

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "key after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_default, 1L, "value_default after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 1L, "get_item after Py_DECREF(container);");
    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(value_default);

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

static PyObject *
test_PyDict_SetDefaultRef_default_unused(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item;

    PyObject *container = PyDict_New();
    assert(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "container after PyObject *container = PyDict_New();");

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "New key");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "New value");

    if (PyDict_SetItem(container, key, value)) {
        assert(0);
        goto finally;
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key after PyDict_SetItem()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after PyDict_SetItem()");

    get_item = PyDict_GetItem(container, key);
    assert(get_item == value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 2L, "get_item after PyDict_GetItem()");

    /* Now check PyDict_SetDefault() which does not use the default. */
    PyObject *value_default = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_default, 1L, "New value_default");

    PyObject *result = NULL;
    int ret_val = PyDict_SetDefaultRef(container, key, value_default, &result);
    if (ret_val != 1) {
        return_value = -1;
    }
    assert(result == value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key after PyDict_SetDefaultRef()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 3L, "value after PyDict_SetDefaultRef()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(result, 3L, "value after PyDict_SetDefaultRef()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_default, 1L, "value_default after PyDict_SetDefaultRef()");

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "key after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(result, 2L, "value after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_default, 1L, "value_default after Py_DECREF(container);");

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(value);
    Py_DECREF(value);
    Py_DECREF(value_default);

    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

PyObject *
test_PyDict_SetDefaultRef_default_used(void) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;

    PyObject *container = PyDict_New();
    assert(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "container after PyObject *container = PyDict_New();");

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "container after PyObject *container = PyDict_New();");

    /* Do not do this so the default is invoked.
    if (PyDict_SetItem(container, key, value)) {
        assert(0);
    }
    */

    /* Now check PyDict_SetDefault() which *does* use the default. */
    PyObject *value_default = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_default, 1L, "container after PyObject *container = PyDict_New();");

    PyObject *result = NULL;
    int ret_val = PyDict_SetDefaultRef(container, key, value_default, &result);
    if (ret_val != 0) {
        return_value = -1;
    }
    assert(result == value_default);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key after PyDict_SetDefaultRef()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_default, 3L, "value after PyDict_SetDefaultRef()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(result, 3L, "result after PyDict_SetDefaultRef()");

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "key after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value_default, 2L, "value after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(result, 2L, "result after Py_DECREF(container);");

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(value_default);
    Py_DECREF(value_default);

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}


#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

static PyObject *
test_PyDict_GetItem(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item = NULL;

    PyObject *container = PyDict_New();
    if (!container) {
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "PyDict_New()");

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "key = new_unique_string(__FUNCTION__, NULL)");
    if (PyDict_GetItem(container, key) != NULL) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "value = new_unique_string(__FUNCTION__, NULL)");

    if (PyDict_SetItem(container, key, value)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key after PyDict_SetItem()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value_a after PyDict_SetItem()");

    get_item = PyDict_GetItem(container, key);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 2L, "get_item = PyDict_GetItem(container, key);");
    if (get_item != value) {
        fprintf(stderr, "get_item = PyDict_GetItem(container, key); is not value_a\n");
        return_value |= 1 << error_flag_position;
        goto finally;
    }
    error_flag_position++;

    Py_DECREF(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "key after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "value_b after Py_DECREF(container);");
    Py_DECREF(key);
    Py_DECREF(value);
    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

static PyObject *
test_PyDict_Pop_key_present(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;
    PyObject *get_item;

    PyObject *container = PyDict_New();
    assert(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "container after PyObject *container = PyDict_New();");

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "New key");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "New value");

    if (PyDict_SetItem(container, key, value)) {
        assert(0);
        goto finally;
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 2L, "key after PyDict_SetItem()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after PyDict_SetItem()");

    get_item = PyDict_GetItem(container, key);
    assert(get_item == value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(get_item, 2L, "get_item after PyDict_GetItem()");

    PyObject *result = NULL;
    int ret_val = PyDict_Pop(container, key, &result);
    if (ret_val != 1) {
        return PyLong_FromLong(-1);
    }
    assert(result == value);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "key after PyDict_Pop()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after PyDict_Pop()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(result, 2L, "result after PyDict_Pop()");

    Py_DECREF(container);

    /* Duplicate of above as Py_DECREF(container); does not affect the key/value. */
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "key after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(result, 2L, "result after Py_DECREF(container);");

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(value);
    Py_DECREF(value);

    finally:
    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PyDict_Pop_key_absent(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;

    PyObject *container = PyDict_New();
    assert(container);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "container after PyObject *container = PyDict_New();");

    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "New key");

    /* Not inserted into the dict, just used so that result references it. */
    PyObject *dummy_value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(dummy_value, 1L, "New value");

    PyObject *result = dummy_value;
    int ret_val = PyDict_Pop(container, key, &result);
    if (ret_val != 0) {
        return PyLong_FromLong(-1);
    }
    assert(result == NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "key after PyDict_Pop()");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(dummy_value, 1L, "value after PyDict_Pop()");

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(key, 1L, "key after Py_DECREF(container);");
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(dummy_value, 1L, "value after PyDict_Pop()");

    /* Clean up. */
    Py_DECREF(key);
    Py_DECREF(dummy_value);

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

static PyObject *
test_PySet_Add(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;

    PyObject *container = PySet_New(NULL);
    assert(container);
    assert(PySet_GET_SIZE(container) == 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "container after PyObject *container = PySet_New(NULL);");

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "New value");

    int ret_val = PySet_Add(container, value);
    if (ret_val != 0) {
        return PyLong_FromLong(-1);
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after PySet_Add()");

    // Add duplicate.
    ret_val = PySet_Add(container, value);
    if (ret_val != 0) {
        return PyLong_FromLong(-1);
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after second PySet_Add()");

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "value after Py_DECREF(container);");

    /* Clean up. */
    Py_DECREF(value);

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PySet_Discard(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;

    PyObject *container = PySet_New(NULL);
    assert(container);
    assert(PySet_GET_SIZE(container) == 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "container after PyObject *container = PySet_New(NULL);");

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "New value");

    int ret_val = PySet_Add(container, value);
    if (ret_val != 0) {
        return PyLong_FromLong(-1);
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after PySet_Add()");

    // Discard.
    if (PySet_Discard(container, value) != 1) {
        return PyLong_FromLong(-2);
    }
    assert(PySet_GET_SIZE(container) == 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "value after PySet_Discard(container, value)");

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "value after Py_DECREF(container);");

    /* Clean up. */
    Py_DECREF(value);

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

static PyObject *
test_PySet_Pop(PyObject *Py_UNUSED(module)) {
    CHECK_FOR_PYERROR_ON_FUNCTION_ENTRY(NULL);
    assert(!PyErr_Occurred());
    long return_value = 0L;
    int error_flag_position = 0;

    PyObject *container = PySet_New(NULL);
    assert(container);
    assert(PySet_GET_SIZE(container) == 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "container after PyObject *container = PySet_New(NULL);");

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "New value");

    int ret_val = PySet_Add(container, value);
    if (ret_val != 0) {
        return PyLong_FromLong(-1);
    }
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after PySet_Add()");

    // Pop.
    PyObject *popped_value = PySet_Pop(container);
    assert(popped_value == value);
    assert(PySet_GET_SIZE(container) == 0);
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after PySet_Pop(container)");

    Py_DECREF(container);

    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 2L, "value after Py_DECREF(container);");

    /* Clean up. */
    Py_DECREF(value);
    Py_DECREF(value);

    assert(!PyErr_Occurred());
    return PyLong_FromLong(return_value);
}

#define MODULE_NOARGS_ENTRY(name, doc)  \
    {                                   \
        #name,                          \
        (PyCFunction) name,             \
        METH_NOARGS,                    \
        doc,                            \
    }

static PyMethodDef module_methods[] = {
        MODULE_NOARGS_ENTRY(tuple_steals, "Checks that PyTuple_SET_ITEM steals a reference count."),
        MODULE_NOARGS_ENTRY(tuple_buildvalue_steals, "Checks that Py_BuildValue tuple steals a reference count."),
        MODULE_NOARGS_ENTRY(list_steals, "Checks that PyTuple_SET_ITEM list steals a reference count."),
        MODULE_NOARGS_ENTRY(list_buildvalue_steals, "Checks that Py_BuildValue list steals a reference count."),
        MODULE_NOARGS_ENTRY(set_no_steals, "Checks that a set increments a reference count."),
        MODULE_NOARGS_ENTRY(set_no_steals_decref,
                            "Checks that a set increments a reference count and uses decref_set_values."),
        MODULE_NOARGS_ENTRY(dict_no_steals, "Checks that a dict increments a reference counts for key and value."),
        MODULE_NOARGS_ENTRY(dict_no_steals_decref_after_set,
                            "Checks that a dict increments a reference counts for key and value."
                            " They are decremented after PyDict_Set()"
        ),
        MODULE_NOARGS_ENTRY(dict_buildvalue_no_steals,
                            "Checks that a Py_BuildValue dict increments a reference counts for key and value."),
#pragma mark - Testing Tuples
        /* Test ref counts with container APIs. */
        MODULE_NOARGS_ENTRY(test_PyTuple_SetItem_steals, "Check that PyTuple_SetItem() steals a reference."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SET_ITEM_steals, "Check that PyTuple_SET_ITEM() steals a reference."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SetItem_steals_replace,
                            "Check that PyTuple_SetItem() steals a reference on replacement."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SET_ITEM_steals_replace,
                            "Check that PyTuple_SET_ITEM() steals a reference on replacement."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SetItem_replace_same,
                            "Check how PyTuple_SetItem() behaves on replacement of the same value."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SET_ITEM_replace_same,
                            "Check how PyTuple_SET_ITEM() behaves on replacement of the same value."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SetItem_NULL, "Check that PyTuple_SetItem() with NULL does not error."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SET_ITEM_NULL, "Check that PyTuple_SET_ITEM() with NULL does not error."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SetIem_NULL_SetItem,
                            "Check that PyTuple_SetItem() with NULL then with an object does not error."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SET_ITEM_NULL_SET_ITEM,
                            "Check that PyTuple_SET_ITEM() with NULL then with an object does not error."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SetItem_fails_not_a_tuple,
                            "Check that PyTuple_SET_ITEM() fails when not a tuple."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SetItem_fails_out_of_range,
                            "Check that PyTuple_SET_ITEM() fails when index out of range."),
        MODULE_NOARGS_ENTRY(test_PyTuple_Py_PyTuple_Pack,
                            "Check that Py_PyTuple_Pack() increments reference counts."),
        MODULE_NOARGS_ENTRY(test_PyTuple_Py_BuildValue,
                            "Check that Py_BuildValue() increments reference counts."),
#pragma mark - Testing Lists
        /* Test ref counts with container APIs. */
        MODULE_NOARGS_ENTRY(test_PyList_SetItem_steals, "Check that PyList_SetItem() steals a reference."),
        MODULE_NOARGS_ENTRY(test_PyList_SET_ITEM_steals, "Check that PyList_SET_ITEM() steals a reference."),
        MODULE_NOARGS_ENTRY(test_PyList_SetItem_steals_replace,
                            "Check that PyList_SetItem() steals a reference on replacement."),
        MODULE_NOARGS_ENTRY(test_PyList_SET_ITEM_steals_replace,
                            "Check that PyList_SET_ITEM() steals a reference on replacement."),
        MODULE_NOARGS_ENTRY(test_PyList_SetItem_replace_same,
                            "Check how PyTuple_SetItem() behaves on replacement of the same value."),
        MODULE_NOARGS_ENTRY(test_PyList_SET_ITEM_replace_same,
                            "Check how PyTuple_SET_ITEM() behaves on replacement of the same value."),
        MODULE_NOARGS_ENTRY(test_PyList_SetItem_NULL, "Check that PyList_SetItem() with NULL does not error."),
        MODULE_NOARGS_ENTRY(test_PyList_SET_ITEM_NULL, "Check that PyList_SET_ITEM() with NULL does not error."),
        MODULE_NOARGS_ENTRY(test_PyList_SetIem_NULL_SetItem,
                            "Check that PyList_SetItem() with NULL then with an object does not error."),
        MODULE_NOARGS_ENTRY(test_PyList_SET_ITEM_NULL_SET_ITEM,
                            "Check that PyList_SET_ITEM() with NULL then with an object does not error."),
        MODULE_NOARGS_ENTRY(test_PyList_SetItem_fails_not_a_list,
                            "Check that PyList_SET_ITEM() fails when not a tuple."),
        MODULE_NOARGS_ENTRY(test_PyList_SetItem_fails_out_of_range,
                            "Check that PyList_SET_ITEM() fails when index out of range."),
        MODULE_NOARGS_ENTRY(test_PyList_Append,
                            "Check that PyList_Append() increments reference counts."),
        MODULE_NOARGS_ENTRY(test_PyList_Append_fails_not_a_list,
                            "Check that PyList_Append() raises when not a list."),
        MODULE_NOARGS_ENTRY(test_PyList_Append_fails_NULL,
                            "Check that PyList_Append() raises on NULL."),
        MODULE_NOARGS_ENTRY(test_PyList_Insert,
                            "Check that PyList_Insert() increments reference counts."),
        MODULE_NOARGS_ENTRY(test_PyList_Insert_Is_Truncated,
                            "Check that PyList_Insert() truncates index."),
        MODULE_NOARGS_ENTRY(test_PyList_Insert_Negative_Index,
                            "Check that PyList_Insert() with negative index."),
        MODULE_NOARGS_ENTRY(test_PyList_Insert_fails_not_a_list,
                            "Check that PyList_Insert() raises when not a list."),
        MODULE_NOARGS_ENTRY(test_PyList_Insert_fails_NULL,
                            "Check that PyList_Insert() raises on NULL."),
        MODULE_NOARGS_ENTRY(test_PyList_Py_BuildValue,
                            "Check that Py_BuildValue() increments reference counts."),
#pragma mark - Testing Dictionaries
        MODULE_NOARGS_ENTRY(test_PyDict_SetItem_increments, "Check that PyDict_SetItem() works as expected."),
        MODULE_NOARGS_ENTRY(test_PyDict_SetItem_fails_not_a_dict,
                            "Check that PyDict_SetItem() fails when not a dictionary."),
        MODULE_NOARGS_ENTRY(test_PyDict_SetItem_fails_not_hashable,
                            "Check that PyDict_SetItem() fails when key is not hashable."),
        MODULE_NOARGS_ENTRY(test_PyDict_SetDefault_default_unused,
                            "Check that PyDict_SetDefault() works when the default is not used."),
        MODULE_NOARGS_ENTRY(test_PyDict_SetDefault_default_used,
                            "Check that PyDict_SetDefault() works when the default not used."),
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
        MODULE_NOARGS_ENTRY(test_PyDict_SetDefaultRef_default_unused,
                            "Check that PyDict_SetDefaultRef() works when the default is not used."),
        MODULE_NOARGS_ENTRY(test_PyDict_SetDefaultRef_default_used,
                            "Check that PyDict_SetDefaultRef() works when the default not used."),
#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

        MODULE_NOARGS_ENTRY(test_PyDict_GetItem,
                            "Checks PyDict_GetItem()."),

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
        MODULE_NOARGS_ENTRY(test_PyDict_Pop_key_present,
                            "Check that PyDict_Pop() works when the key is present."),
        MODULE_NOARGS_ENTRY(test_PyDict_Pop_key_absent,
                            "Check that PyDict_Pop() works when the key is absent."),
#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
#pragma mark - Testing Sets
        MODULE_NOARGS_ENTRY(test_PySet_Add, "Check PySet_Add()."),
        MODULE_NOARGS_ENTRY(test_PySet_Discard, "Check test_PySet_Discard()."),
        MODULE_NOARGS_ENTRY(test_PySet_Pop, "Check PySet_Pop()."),
        {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef cRefCount = {
        PyModuleDef_HEAD_INIT,
        .m_name = "cRefCount",
        .m_doc = "Exploring reference counts.",
        .m_size = -1,
        .m_methods = module_methods,
};

PyMODINIT_FUNC PyInit_cRefCount(void) {
    return PyModule_Create(&cRefCount);
}
