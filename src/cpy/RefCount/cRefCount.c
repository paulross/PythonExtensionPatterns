//
// Created by Paul Ross on 20/10/2024.
//
// This explores reference counts with the Python C-API.
#define PPY_SSIZE_T_CLEAN

#include "Python.h"

/* For access to new_unique_string().*/
#include "DebugContainers.h"

/**
 * Decrement the reference counts of each set value by one.
 *
 * @param op The set.
 * @return 0 on success, non-zero on failure in which case a Python Exception will have been set.
 */
static int
decref_set_values(PyObject *op) {
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
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
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

//    ref_count = Py_REFCNT(container);
//    if (ref_count != 1) {
//        return_value |= 1 << error_flag_position;
//    }
//    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(container, 1L, "PyTuple_New()");

    PyObject *value = new_unique_string(__FUNCTION__, NULL);
//    ref_count = Py_REFCNT(value);
//    if (ref_count != 1) {
//        return_value |= 1 << error_flag_position;
//    }
//    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "value = new_unique_string(__FUNCTION__, NULL)");

    if (PyTuple_SetItem(container, 0, value)) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

//    ref_count = Py_REFCNT(value);
//    if (ref_count != 1) {
//        return_value |= 1 << error_flag_position;
//    }
//    error_flag_position++;
    TEST_REF_COUNT_THEN_OR_RETURN_VALUE(value, 1L, "value after PyTuple_SetItem()");

    get_item = PyTuple_GET_ITEM(container, 0);
    ref_count = Py_REFCNT(get_item);
    if (ref_count != 1) {
        return_value |= 1 << error_flag_position;
    }
    error_flag_position++;

    // TODO: Propogate this below.
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
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
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
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
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
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
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

static PyObject *
test_PyTuple_SetItem_NULL(PyObject *Py_UNUSED(module)) {
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
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
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
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
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
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
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
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
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
    assert(!PyErr_Occurred());

    PyObject *container = PyList_New(1);
    if (!container) {
        return NULL;
    }
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* This should fail. */
    if (PyTuple_SetItem(container, 0, value)) {
        assert(PyErr_Occurred());
        return NULL;
    }
    PyErr_Format(PyExc_RuntimeError, "Should have raised an error.");
    return NULL;
}

static PyObject *
test_PyTuple_SetItem_fails_out_of_range(PyObject *Py_UNUSED(module)) {
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
    assert(!PyErr_Occurred());

    PyObject *container = PyTuple_New(1);
    if (!container) {
        return NULL;
    }
    PyObject *value = new_unique_string(__FUNCTION__, NULL);
    /* This should fail. */
    if (PyTuple_SetItem(container, 1, value)) {
        assert(PyErr_Occurred());
        return NULL;
    }
    PyErr_Format(PyExc_RuntimeError, "Should have raised an error.");
    return NULL;
}

static PyObject *
test_PyTuple_Py_BuildValue(PyObject *Py_UNUSED(module)) {
    if (PyErr_Occurred()) {
        PyErr_Print();
        return NULL;
    }
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
        /* Test ref counts with container APIs. */
        MODULE_NOARGS_ENTRY(test_PyTuple_SetItem_steals, "Check that PyTuple_SetItem() steals a reference."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SET_ITEM_steals, "Check that PyTuple_SET_ITEM() steals a reference."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SetItem_steals_replace,
                            "Check that PyTuple_SetItem() steals a reference on replacement."),
        MODULE_NOARGS_ENTRY(test_PyTuple_SET_ITEM_steals_replace,
                            "Check that PyTuple_SET_ITEM() steals a reference on replacement."),
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
        MODULE_NOARGS_ENTRY(test_PyTuple_Py_BuildValue,
                            "Check that Py_BuildValue() with an existing object."),
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
