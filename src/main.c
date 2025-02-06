//
//  main.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 07/05/2014.
//  Copyright (c) 2014 Paul Ross. All rights reserved.
//
#define PPY_SSIZE_T_CLEAN

#include <Python.h>

#include <stdio.h>

#include "DebugContainers.h"
#include "DictWatcher.h"

/**
 * Get the current working directory using \c getcwd().
 *
 * @return The current working directory or NULL on failure.
 */
const char *current_working_directory(const char *extend) {
    static char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "%s(): Can not get current working directory.\n", __FUNCTION__);
        return NULL;
    }
    if (extend) {
        if (snprintf(cwd + strlen(cwd), strlen(extend) + 2, "/%s", extend) == 0) {
            fprintf(stderr, "%s(): Can not compose buffer.\n", __FUNCTION__);
            return NULL;
        }
    }
    return cwd;
}

#if 0
/** Takes a path and adds it to sys.paths by calling PyRun_SimpleString.
 * This does rather laborious C string concatenation so that it will work in
 * a primitive C environment.
 *
 * Returns 0 on success, non-zero on failure.
 */
int add_path_to_sys_module(const char *path) {
    int ret = 0;
    const char *prefix = "import sys\nsys.path.append(\"";
    const char *suffix = "\")\n";
    char *command = (char*)malloc(strlen(prefix)
                                  + strlen(path)
                                  + strlen(suffix)
                                  + 1);
    if (! command) {
        return -1;
    }
    strcpy(command, prefix);
    strcat(command, path);
    strcat(command, suffix);
    ret = PyRun_SimpleString(command);
#ifdef DEBUG
    printf("Calling PyRun_SimpleString() with:\n");
    printf("%s", command);
    printf("PyRun_SimpleString() returned: %d\n", ret);
    fflush(stdout);
#endif
    free(command);
    return ret;
}

int import_call_execute_no_args(const char *module_name, const char *function_name) {
    int return_value = 0;
    PyObject *pModule   = NULL;
    PyObject *pFunc     = NULL;
    PyObject *pResult   = NULL;

    assert(! PyErr_Occurred());
    pModule = PyImport_ImportModule(module_name);
    if (! pModule) {
        fprintf(stderr, "%s: Failed to load module \"%s\"\n", __FUNCTION__ , module_name);
        return_value = -3;
        goto except;
    }
    assert(! PyErr_Occurred());
    pFunc = PyObject_GetAttrString(pModule, function_name);
    if (! pFunc) {
        fprintf(stderr,
                "%s: Can not find function \"%s\"\n", __FUNCTION__, function_name);
        return_value = -4;
        goto except;
    }
    assert(! PyErr_Occurred());
    if (! PyCallable_Check(pFunc)) {
        fprintf(stderr,
                "%s: Function \"%s\" is not callable\n", __FUNCTION__, function_name);
        return_value = -5;
        goto except;
    }
    assert(! PyErr_Occurred());
    pResult = PyObject_CallObject(pFunc, NULL);
    if (! pResult) {
        fprintf(stderr, "%s: Function call %s failed\n", __FUNCTION__, function_name);
        return_value = -6;
        goto except;
    }
    assert(! PyErr_Occurred());
goto finally;
    except:
    assert(PyErr_Occurred());
    PyErr_Print();
finally:
    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);
    Py_XDECREF(pResult);
    Py_Finalize();
    return return_value;
}

#endif

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    Py_Initialize();
    const char *cwd = current_working_directory("..");
    int failure = 0;

    int32_t py_version_hex = PY_VERSION_HEX;
    printf("Python version %d.%d.%d Release level: 0x%x Serial: %d Numeric: %12d 0x%08x",
           PY_MAJOR_VERSION, PY_MINOR_VERSION, PY_MICRO_VERSION,
           PY_RELEASE_LEVEL, PY_RELEASE_SERIAL,
           py_version_hex, py_version_hex
    );
//    failure = add_path_to_sys_module(cwd);
//    if (failure) {
//        printf("add_path_to_sys_module(): Failed with error code %d\n", failure);
//        goto finally;
//    }

//    failure = dbg_cPyRefs();
//    if (failure) {
//        printf("dbg_cPyRefs(): Failed with error code %d\n", failure);
//        return  -1;
//    }

//    /* cPyExtPatt.cRefCount tests*/
//    failure = test_dbg_cRefCount();
//    if (failure) {
//        printf("test_dbg_cRefCount(): Failed with error code %d\n", failure);
//        goto finally;
//    }

#pragma mark - Tuples
    dbg_PyTuple_SetItem_steals();
    dbg_PyTuple_SET_ITEM_steals();
    dbg_PyTuple_SetItem_steals_replace();
    dbg_PyTuple_SET_ITEM_steals_replace();
    dbg_PyTuple_SetItem_replace_with_same();
    dbg_PyTuple_SET_ITEM_replace_with_same();
    dbg_PyTuple_SetIem_NULL();
    dbg_PyTuple_SET_ITEM_NULL();
    dbg_PyTuple_SetIem_NULL_SetItem();
    dbg_PyTuple_SET_ITEM_NULL_SET_ITEM();
    dbg_PyTuple_SetItem_fails_not_a_tuple();
    dbg_PyTuple_SetItem_fails_out_of_range();
    dbg_PyTuple_PyTuple_Pack();
    dbg_PyTuple_Py_BuildValue();

#pragma mark - Lists
    dbg_PyList_SetItem_steals();
    dbg_PyList_SET_ITEM_steals();
    dbg_PyList_SetItem_steals_replace();
    dbg_PyList_SET_ITEM_steals_replace();
    dbg_PyList_SetItem_replace_with_same();
    dbg_PyList_SET_ITEM_replace_with_same();
    dbg_PyList_SetIem_NULL();
    dbg_PyList_SET_ITEM_NULL();
    dbg_PyList_SetIem_NULL_SetItem();
    dbg_PyList_SET_ITEM_NULL_SET_ITEM();
    dbg_PyList_SetItem_fails_not_a_tuple();
    dbg_PyList_SetItem_fails_out_of_range();
    dbg_PyList_Append();
    dbg_PyList_Append_fails_not_a_list();
    dbg_PyList_Append_fails_NULL();
    dbg_PyList_Insert();
    dbg_PyList_Insert_Is_Truncated();
    dbg_PyList_Insert_Negative_Index();
    dbg_PyList_Insert_fails_not_a_list();
    dbg_PyList_Insert_fails_NULL();
    dbg_PyList_Py_BuildValue();

#pragma mark - Dictionaries
    dbg_PyDict_SetItem_increments();

#if ACCEPT_SIGSEGV
    dbg_PyDict_SetItem_NULL_key();
    dbg_PyDict_SetItem_NULL_value();
#endif // ACCEPT_SIGSEGV

    dbg_PyDict_SetItem_fails_not_a_dict();
    dbg_PyDict_SetItem_fails_not_hashable();
    dbg_PyDict_SetDefault_default_unused();
    dbg_PyDict_SetDefault_default_used();
    dbg_PyDict_SetDefaultRef_default_unused();

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    dbg_PyDict_SetDefaultRef_default_used();
    dbg_PyDict_SetDefaultRef_default_unused_result_non_null();
#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

    dbg_PyDict_GetItem();

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    dbg_PyDict_GetItemRef();
#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
    dbg_PyDict_Pop_key_present();
    dbg_PyDict_Pop_key_absent();
#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

#if ACCEPT_SIGSEGV
    /* Comment out as desired. */
    dbg_PyTuple_SetItem_SIGSEGV_on_same_value();
    dbg_PyList_SetItem_SIGSEGV_on_same_value();
    dbg_PyDict_SetItem_SIGSEGV_on_key_NULL();
    dbg_PyDict_SetItem_SIGSEGV_on_value_NULL();
    dbg_PyDict_GetItem_key_NULL();
#endif

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12
    // Watchers
    dbg_PyDict_EVENT_ADDED();
    dbg_PyDict_EVENT_MODIFIED();
    dbg_PyDict_EVENT_MODIFIED_same_value_no_event();
#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12

#pragma mark - Dictionaries
    dbg_PySet_Add();
    dbg_PySet_Discard();
    dbg_PySet_Pop();


    printf("Bye, bye!\n");
    return failure;
}

