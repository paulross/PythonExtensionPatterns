//
// Created by Paul Ross on 20/10/2024.
//
// This explores reference counts with the Python C-API.
#define PPY_SSIZE_T_CLEAN

#include "Python.h"

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
                            "Checks that a Py_BuildValue dict increments a reference counts for key and value ."),
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
