//
// Created by Paul Ross on 28/12/2024.
//
// Example of a Struct Sequence Object (equivalent to a named tuple).
// Documentation: https://docs.python.org/3/c-api/tuple.html#struct-sequence-objects
// Example test case: Modules/_testcapimodule.c test_structseq_newtype_doesnt_leak()
// Fairly complicated example: Modules/posixmodule.c

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "structmember.h"









//typedef struct {
//    PyObject_HEAD
//    long *array_long;
//    ssize_t size;
//} SequenceOfLong;

//static PyObject *
//iterate_and_print(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
//    assert(!PyErr_Occurred());
//    static char *kwlist[] = {"sequence", NULL};
//    PyObject *sequence = NULL;
//
//    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &sequence)) {
//        return NULL;
//    }
////    if (!PyIter_Check(sequence)) {
////        PyErr_Format(PyExc_TypeError, "Object of type %s does support the iterator protocol",
////                     Py_TYPE(sequence)->tp_name);
////        return NULL;
////    }
//    PyObject *iterator = PyObject_GetIter(sequence);
//    if (iterator == NULL) {
//        /* propagate error */
//        assert(PyErr_Occurred());
//        return NULL;
//    }
//    PyObject *item = NULL;
//    long index = 0;
//    fprintf(stdout, "%s:\n", __FUNCTION__ );
//    while ((item = PyIter_Next(iterator))) {
//        /* do something with item */
//        fprintf(stdout, "[%ld]: ", index);
//        if (PyObject_Print(item, stdout, Py_PRINT_RAW) == -1) {
//            /* Handle error. */
//            Py_DECREF(item);
//            Py_DECREF(iterator);
//            if (!PyErr_Occurred()) {
//                PyErr_Format(PyExc_RuntimeError,
//                             "Can not print an item of type %s",
//                             Py_TYPE(sequence)->tp_name);
//            }
//            return NULL;
//        }
//        fprintf(stdout, "\n");
//        ++index;
//        /* release reference when done */
//        Py_DECREF(item);
//    }
//    Py_DECREF(iterator);
//    if (PyErr_Occurred()) {
//        /* propagate error */
//        return NULL;
//    }
//    fprintf(stdout, "%s: DONE\n", __FUNCTION__ );
//    fflush(stdout);
//    assert(!PyErr_Occurred());
//    Py_RETURN_NONE;
//}

static PyMethodDef cStructSequence_methods[] = {
//        {"iterate_and_print", (PyCFunction) iterate_and_print, METH_VARARGS,
//                "Iteratee through the argument printing the values."},
        {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef cStructSequence_cmodule = {
        PyModuleDef_HEAD_INIT,
        .m_name = "cStructSequence",
        .m_doc = (
                "Example module that creates an extension type"
                "that has forward and reverse iterators."
        ),
        .m_size = -1,
        .m_methods = cStructSequence_methods,
};

PyMODINIT_FUNC
PyInit_cStructSequence(void) {
    PyObject *m;
    m = PyModule_Create(&cStructSequence_cmodule);
    if (m == NULL) {
        return NULL;
    }

//    if (PyType_Ready(&SequenceOfLongType) < 0) {
//        Py_DECREF(m);
//        return NULL;
//    }
//    Py_INCREF(&SequenceOfLongType);
//    if (PyModule_AddObject(
//            m,
//            "SequenceOfLong",
//            (PyObject *) &SequenceOfLongType) < 0
//            ) {
//        Py_DECREF(&SequenceOfLongType);
//        Py_DECREF(m);
//        return NULL;
//    }
//    if (PyType_Ready(&SequenceOfLongIteratorType) < 0) {
//        Py_DECREF(m);
//        return NULL;
//    }
//    Py_INCREF(&SequenceOfLongIteratorType);
//    // Not strictly necessary unless you need to expose this type.
//    // For type checking for example.
//    if (PyModule_AddObject(
//            m,
//            "SequenceOfLongIterator",
//            (PyObject *) &SequenceOfLongIteratorType) < 0
//            ) {
//        Py_DECREF(&SequenceOfLongType);
//        Py_DECREF(&SequenceOfLongIteratorType);
//        Py_DECREF(m);
//        return NULL;
//    }
    return m;
}

