//
//  main.c
//  PythonSubclassList
//
//  Created by Paul Ross on 01/05/2016.
//  Copyright (c) 2016 Paul Ross. All rights reserved.
//

#include <stdio.h>

#include <Python.h>

#define MODULE_DIR "/Users/paulross/dev/Xcode/Python/PythonSubclassList/PythonSubclassList"

int main(int argc, const char *argv[]) {
    PyObject *pModule   = NULL;
    PyObject *pFunc     = NULL;
    PyObject *pResult   = NULL;
    int return_value = 0;
    
    Py_SetProgramName((wchar_t*)argv[0]);
    Py_Initialize();
    if (argc != 3) {
        fprintf(stderr, "Wrong arguments. Usage: pyxcode module function\n");
        return_value = 1;
        goto except;
    }
#ifndef MODULE_DIR
/* Don't really need this as an error will be generated since
 * undefined macros default to the value 0 but this error is clearer.
 */
#error "Must define MODULE_DIR, the path to your Python module that I am going to import."
#endif
    PyRun_SimpleString("import sys\n"
                       "sys.path.append(\""
                       MODULE_DIR
                       "\")\n"
                       );
    pModule = PyImport_ImportModule(argv[1]);
    if (pModule) {
        pFunc = PyObject_GetAttrString(pModule, argv[2]);
        if (pFunc && PyCallable_Check(pFunc)) {
            pResult = PyObject_CallObject(pFunc, NULL);
            if (pResult) {
                printf("pyxcode: Call succeeded\n");
                Py_DECREF(pResult);
            } else {
                Py_DECREF(pFunc);
                Py_DECREF(pModule);
                fprintf(stderr,"pyxcode: Function call failed\n");
                return_value = 1;
                goto except;
            }
        } else {
            fprintf(stderr, "pyxcode: Can not call function \"%s\"\n", argv[2]);
            return_value = 1;
            goto except;
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    } else {
        fprintf(stderr, "pyxcode: Failed to load module \"%s\"\n", argv[1]);
        return_value = 1;
        goto except;
    }
    assert(! PyErr_Occurred());
    goto finally;
except:
    assert(PyErr_Occurred());
    PyErr_Print();
finally:
    Py_Finalize();
    return return_value;
}

