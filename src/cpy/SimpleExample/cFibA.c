#define PPY_SSIZE_T_CLEAN

#include "Python.h"

long fibonacci(long index) {
    if (index < 2) {
        return index;
    }
    return fibonacci(index - 2) + fibonacci(index - 1);
}

//long fibonacci(long index) {
//    static long *cache = NULL;
//    if (!cache) {
//        /* FIXME */
//        cache = calloc(1000, sizeof(long));
//    }
//    if (index < 2) {
//        return index;
//    }
//    if (!cache[index]) {
//        cache[index] = fibonacci(index - 2) + fibonacci(index - 1);
//    }
//    return cache[index];
//}

static PyObject *
py_fibonacci(PyObject *Py_UNUSED(module), PyObject *args) {
    long index;

    if (!PyArg_ParseTuple(args, "l", &index)) {
        return NULL;
    }
    long result = fibonacci(index);
    return Py_BuildValue("l", result);
}

//static PyObject *
//py_fibonacci(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwargs) {
//    long index;
//
//    static char *keywords[] = {"index", NULL};
//    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "l", keywords, &index)) {
//        return NULL;
//    }
//    long result = fibonacci(index);
//    return Py_BuildValue("l", result);
//}
//

static PyMethodDef module_methods[] = {
        {"fibonacci",
                (PyCFunction) py_fibonacci,
                METH_VARARGS,
                "Returns the Fibonacci value."
        },
        {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef cFibA = {
        PyModuleDef_HEAD_INIT,
        .m_name = "cFibA",
        .m_doc = "Fibonacci in C.",
        .m_size = -1,
        .m_methods = module_methods,
};

PyMODINIT_FUNC PyInit_cFibA(void) {
    PyObject *m = PyModule_Create(&cFibA);
    return m;
}
