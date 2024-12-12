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

int import_call_execute(const char *module_name, const char *function_name) {
    int return_value = 0;
    PyObject *pModule   = NULL;
    PyObject *pFunc     = NULL;
    PyObject *pResult   = NULL;

//    if (argc != 4) {
//        fprintf(stderr,
//                "Wrong arguments!"
//                " Usage: %s package_path module function\n", argv[0]);
//        return_value = -1;
//        goto except;
//    }
//    Py_SetProgramName((wchar_t*)argv[0]);
//    Py_Initialize();
//    if (add_path_to_sys_module(argv[1])) {
//        return_value = -2;
//        goto except;
//    }
    pModule = PyImport_ImportModule(module_name);
    if (! pModule) {
        fprintf(stderr, "%s: Failed to load module \"%s\"\n", __FUNCTION__ , module_name);
        return_value = -3;
        goto except;
    }
    pFunc = PyObject_GetAttrString(pModule, function_name);
    if (! pFunc) {
        fprintf(stderr,
                "%s: Can not find function \"%s\"\n", __FUNCTION__, function_name);
        return_value = -4;
        goto except;
    }
    if (! PyCallable_Check(pFunc)) {
        fprintf(stderr,
                "%s: Function \"%s\" is not callable\n", __FUNCTION__, function_name);
        return_value = -5;
        goto except;
    }
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

int foo(void) {
    int success = add_path_to_sys_module("/Users/engun/GitHub/paulross/PythonExtensionPatterns");
    printf("Success: %d", success);
    success = PyRun_SimpleString("from cPyExtPatt import cPyRefs");
    printf("Success: %d", success);
    return import_call_execute("cPyExtPatt.cPyRefs", "subtract_two_longs");
}

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    Py_Initialize();

    foo();


    return 0;
}

