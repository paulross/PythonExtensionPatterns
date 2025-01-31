//
// Created by Paul Ross on 30/01/2025.
//

#include "pyextpatt_util.h"

/* This is used to guarantee that Python is not caching a string value when we want to check the
 * reference counts after each string creation.
 * */
static long debug_test_count = 0L;

PyObject *
new_unique_string(const char *function_name, const char *suffix) {
    PyObject *value = NULL;
    if (suffix) {
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
