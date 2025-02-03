//
// Created by Paul Ross on 30/01/2025.
//
//#define PPY_SSIZE_T_CLEAN
//
//#include "Python.h"

#include "DictWatcher.h"
#include "pyextpatt_util.h"

/* Version as a single 4-byte hex number, e.g. 0x010502B2 == 1.5.2b2
 * Therefore 0x030C0000 == 3.12.0
 */
#if PY_VERSION_HEX < 0x030C0000

#error "Required version of Python is 3.12+ (PY_VERSION_HEX >= 0x030C0000)"

#else

// Event counters for a dictionary
static long static_dict_added = 0L;
static long static_dict_modified = 0L;
static long static_dict_deleted = 0L;
static long static_dict_cloned = 0L;
static long static_dict_cleared = 0L;
static long static_dict_deallocated = 0L;

#define GET_STATIC_DICT_VALUE(name) \
    long get_##name(void) {         \
        return name;                \
    }                               \


GET_STATIC_DICT_VALUE(static_dict_added)

GET_STATIC_DICT_VALUE(static_dict_modified)

GET_STATIC_DICT_VALUE(static_dict_deleted)

GET_STATIC_DICT_VALUE(static_dict_cloned)

GET_STATIC_DICT_VALUE(static_dict_cleared)

GET_STATIC_DICT_VALUE(static_dict_deallocated)


// Dictionary callback function
static int dict_watcher_inc_event_counter(PyDict_WatchEvent event, PyObject *Py_UNUSED(dict), PyObject *Py_UNUSED(key),
                                          PyObject *Py_UNUSED(new_value)) {
    switch (event) {
        case PyDict_EVENT_ADDED:
            static_dict_added++;
            break;
        case PyDict_EVENT_MODIFIED:
            static_dict_modified++;
            break;
        case PyDict_EVENT_DELETED:
            static_dict_deleted++;
            break;
        case PyDict_EVENT_CLONED:
            static_dict_cloned++;
            break;
        case PyDict_EVENT_CLEARED:
            static_dict_cleared++;
            break;
        case PyDict_EVENT_DEALLOCATED:
            static_dict_deallocated++;
            break;
        default:
            Py_UNREACHABLE();
            break;
    }
    /* No exception set. */
    return 0;
}

/**
 * Create a dictionary, register the callback and exercise it by adding a single key and value.
 */
void dbg_PyDict_EVENT_ADDED(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count = 0;
    int api_ret_val = 0;
    long event_value_previous = 0L;
    long event_value_current = 0L;

    PyObject *container = PyDict_New();
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);
    // Set watcher.
    int watcher_id = PyDict_AddWatcher(&dict_watcher_inc_event_counter);
    api_ret_val = PyDict_Watch(watcher_id, container);
    assert(api_ret_val == 0);
    // Now add a key/value
    event_value_previous = get_static_dict_added();
    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    PyObject *val = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(val);
    assert(ref_count == 1);
    api_ret_val = PyDict_SetItem(container, key, val);
    assert(api_ret_val == 0);
    // Check result
    event_value_current = get_static_dict_added();
    assert(event_value_current == event_value_previous + 1);
    // Clean up.
    api_ret_val = PyDict_Unwatch(watcher_id, container);
    assert(api_ret_val == 0);
    api_ret_val = PyDict_ClearWatcher(watcher_id);
    assert(api_ret_val == 0);
    Py_DECREF(container);
    Py_DECREF(key);
    Py_DECREF(val);
}

/**
 * Create a dictionary, register the callback and exercise it by adding a single key and value then replacing that
 * value with another.
 */
void dbg_PyDict_EVENT_MODIFIED(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count = 0;
    int api_ret_val = 0;
    long event_value_added_previous = 0L;
    long event_value_added_current = 0L;
    long event_value_modified_previous = 0L;
    long event_value_modified_current = 0L;

    PyObject *container = PyDict_New();
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);
    // Set watcher.
    int watcher_id = PyDict_AddWatcher(&dict_watcher_inc_event_counter);
    api_ret_val = PyDict_Watch(watcher_id, container);
    assert(api_ret_val == 0);
    // Now add a key/value
    event_value_added_previous = get_static_dict_added();
    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    PyObject *val_a = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(val_a);
    assert(ref_count == 1);
    api_ret_val = PyDict_SetItem(container, key, val_a);
    assert(api_ret_val == 0);
    // Check result
    event_value_added_current = get_static_dict_added();
    assert(event_value_added_current == event_value_added_previous + 1);
    // Now modify the dictionary by resetting the same value and check the modified counter.
    PyObject *val_b = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(val_b);
    assert(ref_count == 1);
    event_value_modified_previous = get_static_dict_modified();
    api_ret_val = PyDict_SetItem(container, key, val_b);
    assert(api_ret_val == 0);
    event_value_modified_current = get_static_dict_modified();
    assert(event_value_modified_current == event_value_modified_previous + 1);
    // Clean up.
    api_ret_val = PyDict_Unwatch(watcher_id, container);
    assert(api_ret_val == 0);
    api_ret_val = PyDict_ClearWatcher(watcher_id);
    assert(api_ret_val == 0);
    Py_DECREF(container);
    Py_DECREF(key);
    Py_DECREF(val_a);
    Py_DECREF(val_b);
}

/**
 * Demonstrates that replacement with the same value does not generate an PyDict_EVENT_MODIFIED event.
 */
void dbg_PyDict_EVENT_MODIFIED_same_value_no_event(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    int ref_count = 0;
    int api_ret_val = 0;
    long event_value_added_previous = 0L;
    long event_value_added_current = 0L;
    long event_value_modified_previous = 0L;
    long event_value_modified_current = 0L;

    PyObject *container = PyDict_New();
    assert(container);
    ref_count = Py_REFCNT(container);
    assert(ref_count == 1);
    // Set watcher.
    int watcher_id = PyDict_AddWatcher(&dict_watcher_inc_event_counter);
    api_ret_val = PyDict_Watch(watcher_id, container);
    assert(api_ret_val == 0);
    // Now add a key/value
    event_value_added_previous = get_static_dict_added();
    PyObject *key = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(key);
    assert(ref_count == 1);
    PyObject *val = new_unique_string(__FUNCTION__, NULL);
    ref_count = Py_REFCNT(val);
    assert(ref_count == 1);
    api_ret_val = PyDict_SetItem(container, key, val);
    assert(api_ret_val == 0);
    // Check result
    event_value_added_current = get_static_dict_added();
    assert(event_value_added_current == event_value_added_previous + 1);
    // Now modify the dictionary by resetting the same value.
    event_value_modified_previous = static_dict_modified;
    api_ret_val = PyDict_SetItem(container, key, val);
    assert(api_ret_val == 0);
    event_value_modified_current = static_dict_modified;
    assert(event_value_modified_current == event_value_modified_previous + 0);
    // Clean up.
    api_ret_val = PyDict_Unwatch(watcher_id, container);
    assert(api_ret_val == 0);
    api_ret_val = PyDict_ClearWatcher(watcher_id);
    assert(api_ret_val == 0);
    Py_DECREF(container);
    Py_DECREF(key);
    Py_DECREF(val);
}

#pragma mark Verbose watcher to report Python file/line

/** NOTE: This is based on pymemtrace code. */

static const unsigned char MT_U_STRING[] = "";
static const char MT_STRING[] = "";

static const unsigned char *
get_python_file_name(PyFrameObject *frame) {
    if (frame) {
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
        /* See https://docs.python.org/3.11/whatsnew/3.11.html#pyframeobject-3-11-hiding */
        const unsigned char *file_name = PyUnicode_1BYTE_DATA(PyFrame_GetCode(frame)->co_filename);
#else
        const unsigned char *file_name = PyUnicode_1BYTE_DATA(frame->f_code->co_filename);
#endif // PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
        return file_name;
    }
    return MT_U_STRING;
}

static const char *
get_python_function_name(PyFrameObject *frame) {
    const char *func_name = NULL;
    if (frame) {
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
        /* See https://docs.python.org/3.11/whatsnew/3.11.html#pyframeobject-3-11-hiding */
        func_name = (const char *) PyUnicode_1BYTE_DATA(PyFrame_GetCode(frame)->co_name);
#else
        func_name = (const char *) PyUnicode_1BYTE_DATA(frame->f_code->co_name);
#endif // PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 11
        return func_name;
    }
    return MT_STRING;
}

int get_python_line_number(PyFrameObject *frame) {
    if (frame) {
        return PyFrame_GetLineNumber(frame);
    }
    return 0;
}

/**
 *
 * Usage:
 *  write_frame_data_to_outfile(stdout, PyEval_GetFrame(), PyTrace_LINE, Py_None);
 *
 * @param outfile
 * @param frame
 * @param what
 * @param arg
 */
static void
write_frame_data_to_outfile(FILE *outfile, PyFrameObject *frame) {
    if (frame) {
        fprintf(outfile,
                "%-80s %6d %-24s",
                get_python_file_name(frame),
                get_python_line_number(frame),
                get_python_function_name(frame)
                );
    } else {
        fprintf(outfile, "No Python frame available.");
    }
}

static const char *watch_event_name(PyDict_WatchEvent event) {
    switch (event) {
        case PyDict_EVENT_ADDED:
            return "PyDict_EVENT_ADDED";
            break;
        case PyDict_EVENT_MODIFIED:
            return "PyDict_EVENT_MODIFIED";
            break;
        case PyDict_EVENT_DELETED:
            return "PyDict_EVENT_DELETED";
            break;
        case PyDict_EVENT_CLONED:
            return "PyDict_EVENT_CLONED";
            break;
        case PyDict_EVENT_CLEARED:
            return "PyDict_EVENT_CLEARED";
            break;
        case PyDict_EVENT_DEALLOCATED:
            return "PyDict_EVENT_DEALLOCATED";
            break;
        default:
            Py_UNREACHABLE();
            break;
    }
    return "PyDict_EVENT_UNKNOWN";
}


// Verbose dictionary callback function prints out Python file/line, dictionary, key and new value.
static int dict_watcher_verbose(PyDict_WatchEvent event, PyObject *dict, PyObject *key, PyObject *new_value) {
    fprintf(stdout, "Dict @ 0x%p: ", (void *)dict);
    write_frame_data_to_outfile(stdout, PyEval_GetFrame());
    fprintf(stdout, " Event: %-24s", watch_event_name(event));
    fprintf(stdout, "\n");
    if (dict) {
        fprintf(stdout, "    Dict: ");
        PyObject_Print(dict, stdout, Py_PRINT_RAW);
    } else {
        fprintf(stdout, "    Dict: NULL");
    }
    fprintf(stdout, "\n");
    if (key) {
        fprintf(stdout, "    Key (%s): ", Py_TYPE(key)->tp_name);
        PyObject_Print(key, stdout, Py_PRINT_RAW);
    } else {
        fprintf(stdout, "    Key : NULL");
    }
    fprintf(stdout, "\n");
    if (new_value) {
        fprintf(stdout, "    New value (%s): ", Py_TYPE(new_value)->tp_name);
        PyObject_Print(new_value, stdout, Py_PRINT_RAW);
    } else {
        fprintf(stdout, "    New value : NULL");
    }
    fprintf(stdout, "\n");
    return 0;
}

int dict_watcher_verbose_add(PyObject *dict) {
    // Set watcher.
    int watcher_id = PyDict_AddWatcher(&dict_watcher_verbose);
    int api_ret_val = PyDict_Watch(watcher_id, dict);
    assert(api_ret_val == 0);
    return watcher_id;
}

int dict_watcher_verbose_remove(int watcher_id, PyObject *dict) {
    // Clean up.
    int api_ret_val = PyDict_Unwatch(watcher_id, dict);
    if (api_ret_val) {
        return -1;
    }
    api_ret_val = PyDict_ClearWatcher(watcher_id);
    if (api_ret_val) {
        return -2;
    }
    return 0;
}

#endif // PY_VERSION_HEX >= 0x030C0000
