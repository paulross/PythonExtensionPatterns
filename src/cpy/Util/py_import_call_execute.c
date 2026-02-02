//
//  py_import_call_execute.c
//  PythonSubclassList
//
//  Created by Paul Ross on 10/06/2016.
//  Copyright (c) 2016 Paul Ross. All rights reserved.
//
#include <Python.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "py_import_call_execute.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the current working directory using \c getcwd().
 *
 * See: https://pubs.opengroup.org/onlinepubs/9699919799/functions/getcwd.html
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

/**
 * The Python  initialization has changed since Python 3.8.
 * See:
 *
 *  - https://docs.python.org/3/c-api/init_config.html#init-config
 *  - https://docs.python.org/3/c-api/init_config.html#c-preinit
 *
 * We run Python in isolated mode so this is a Python separate from the system Python.
 * See: https://docs.python.org/3/c-api/init_config.html#init-isolated-conf
 */
int initialise_python(int argc, char *const *argv) {
    /* We need at least one argument, the program name. */
    if (argc < 1) {
        return -1;
    }
    PyStatus status;
    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config.isolated = 1;
    /* Decode command line arguments.
     * Implicitly pre-initialise Python (in isolated mode).
     * See: https://docs.python.org/3/c-api/init_config.html#c.PyConfig_SetBytesArgv
     * */
    status = PyConfig_SetBytesArgv(&config, argc, argv);
    if (PyStatus_Exception(status)) {
        goto exception;
    }
    /* Set the program name. Firstly this needs to be converted to wchar_t. */
    wchar_t wchar_buffer[1024];
    swprintf(wchar_buffer, sizeof wchar_buffer/sizeof *wchar_buffer, L"%hs", argv[0]);
    status = PyConfig_SetString(&config, &config.program_name, wchar_buffer);
    if (PyStatus_Exception(status)) {
        goto exception;
    }
    printf("Initialised Python program \"%ls\"\n", config.program_name);
    /* Now initialise. */
    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        goto exception;
    }
    /* Clean up. */
    PyConfig_Clear(&config);
    /* The following call would hand over control to the repl.
     * We don't want to do that here aas we are going to call Python APIs directly. */
    /* return Py_RunMain(); */

    /* Signal success. */
    return 0;
exception:
    PyConfig_Clear(&config);
    if (PyStatus_IsExit(status)) {
        return status.exitcode;
    }
    /* Display the error message and exit the process with
     * non-zero exit code */
    Py_ExitStatusException(status);
}

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

/**
 * This imports a Python module and calls a specific function in it.
 *
 * Arguments:
 *
 * - Name of the Python module.
 * - Name of the function in the module.
 *
 * The Python interpreter must have been initialised and the path to the Python module
 * must have been added to sys.paths so that the module will be imported.
 * The function will be called with no arguments and its return value will be
 * ignored.
 *
 * This returns 0 on success, non-zero on failure.
 *
 */
int import_call_execute(const char *python_module_name, const char *python_function_name) {
    int return_value = 0;
    PyObject *pModule   = NULL;
    PyObject *pFunc     = NULL;
    PyObject *pResult   = NULL;
    
    pModule = PyImport_ImportModule(python_module_name);
    if (! pModule) {
        fprintf(stderr, "Failed to load module \"%s\"\n", python_module_name);
        return_value = -1;
        goto except;
    }
    pFunc = PyObject_GetAttrString(pModule, python_function_name);
    if (! pFunc) {
        fprintf(stderr, "Can not find function \"%s\"\n", python_function_name);
        return_value = -2;
        goto except;
    }
    if (! PyCallable_Check(pFunc)) {
        fprintf(stderr, "Function \"%s\" is not callable\n", python_function_name);
        return_value = -3;
        goto except;
    }
    pResult = PyObject_CallObject(pFunc, NULL);
    if (! pResult) {
        fprintf(stderr, "Function call \"%s\"() failed\n", python_function_name);
        return_value = -4;
        goto except;
    }
#ifdef DEBUG
    printf("%s: PyObject_CallObject() \"%s.%s()\" succeeded\n", python_module_name, python_function_name);
#endif
    assert(! PyErr_Occurred());
    goto finally;
except:
    assert(PyErr_Occurred());
    PyErr_Print();
finally:
    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);
    Py_XDECREF(pResult);
    return return_value;
}
    
#ifdef __cplusplus
//    extern "C" {
#endif
