//
// Created by Paul Ross on 28/12/2024.
//
// Example of a Struct Sequence Object (equivalent to a named tuple).
// Documentation: https://docs.python.org/3/c-api/tuple.html#struct-sequence-objects
// Example test case: Modules/_testcapimodule.c test_structseq_newtype_doesnt_leak()
// Fairly complicated example: Modules/posixmodule.c
/**
 * TODO:
 *
 * We should cover named tuples/dataclasses etc.:
 * https://docs.python.org/3/c-api/tuple.html#struct-sequence-objects
 *
 */


#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "structmember.h"

// Example test case: Modules/_testcapimodule.c test_structseq_newtype_doesnt_leak()
static PyObject *
test_structseq_newtype_doesnt_leak(PyObject *Py_UNUSED(self),
                                   PyObject *Py_UNUSED(args))
{
    PyStructSequence_Desc descr;
    PyStructSequence_Field descr_fields[3];

    descr_fields[0] = (PyStructSequence_Field){"foo", "foo value"};
    descr_fields[1] = (PyStructSequence_Field){NULL, "some hidden value"};
    descr_fields[2] = (PyStructSequence_Field){0, NULL};

    descr.name = "_testcapi.test_descr";
    descr.doc = "This is used to test for memory leaks in NewType";
    descr.fields = descr_fields;
    descr.n_in_sequence = 1;

    PyTypeObject* structseq_type = PyStructSequence_NewType(&descr);
    assert(structseq_type != NULL);
    assert(PyType_Check(structseq_type));
    assert(PyType_FastSubclass(structseq_type, Py_TPFLAGS_TUPLE_SUBCLASS));
    Py_DECREF(structseq_type);

    Py_RETURN_NONE;
}

// Fairly complicated examples in: Modules/posixmodule.c
// A simple example:

PyDoc_STRVAR(TerminalSize_docstring,
             "A tuple of (columns, lines) for holding terminal window size");

static PyStructSequence_Field TerminalSize_fields[] = {
        {"columns", "width of the terminal window in characters"},
        {"lines", "height of the terminal window in characters"},
        {NULL, NULL}
};

static PyStructSequence_Desc TerminalSize_desc = {
        "os.terminal_size",
        TerminalSize_docstring,
        TerminalSize_fields,
        3,
};

//void foo() {
////    {PyStructSequence_UnnamedField, "height of the terminal window in characters"},
//    TerminalSize_desc.fields[2].name = PyStructSequence_UnnamedField;
//}

// PyObject *TerminalSizeType = (PyObject *)PyStructSequence_NewType(&TerminalSize_desc);

// Module initialisation
typedef struct {
    PyObject *billion;
    PyObject *DirEntryType;
    PyObject *ScandirIteratorType;
#if defined(HAVE_SCHED_SETPARAM) || defined(HAVE_SCHED_SETSCHEDULER) || defined(POSIX_SPAWN_SETSCHEDULER) || defined(POSIX_SPAWN_SETSCHEDPARAM)
    PyObject *SchedParamType;
#endif
    PyObject *StatResultType;
    PyObject *StatVFSResultType;
    PyObject *TerminalSizeType;
    PyObject *TimesResultType;
    PyObject *UnameResultType;
#if defined(HAVE_WAITID) && !defined(__APPLE__)
    PyObject *WaitidResultType;
#endif
#if defined(HAVE_WAIT3) || defined(HAVE_WAIT4)
    PyObject *struct_rusage;
#endif
    PyObject *st_mode;
} _posixstate;

#if 0
/* initialize TerminalSize_info */
PyObject *TerminalSizeType = (PyObject *)PyStructSequence_NewType(&TerminalSize_desc);
if (TerminalSizeType == NULL) {
return -1;
}
Py_INCREF(TerminalSizeType);
PyModule_AddObject(m, "terminal_size", TerminalSizeType);
state->TerminalSizeType = TerminalSizeType;
#endif


/* Example of a C struct to PyStructSequence ??? */




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

