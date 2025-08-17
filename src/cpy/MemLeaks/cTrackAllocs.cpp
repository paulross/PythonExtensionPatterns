//
// Created by Paul Ross on 16/08/2025.
//
#include "Python.h"

#define TRACK_ALLOCS_AND_DEALLOCS 1

#if TRACK_ALLOCS_AND_DEALLOCS
#include <iostream>

#include "TrackAllocs.h"

static NewAndDeallocTracker s_NewAndDeallocTracker;

static PyObject *
cTrackAllocs_dump_remaining(PyObject *Py_UNUSED(module), PyObject *Py_UNUSED(args)) {
    std::string result = s_NewAndDeallocTracker.dump_remaining();
    return PyUnicode_FromStringAndSize(result.c_str(), result.size());

}

static PyObject *
cTrackAllocs_statistics(PyObject *Py_UNUSED(module), PyObject *Py_UNUSED(args)) {
    return Py_BuildValue(
            "nnnn",
            s_NewAndDeallocTracker.total_new(),
            s_NewAndDeallocTracker.total_tp_basicsize(),
            s_NewAndDeallocTracker.total_dealloc(),
            s_NewAndDeallocTracker.max_allocs()
            );
}

/**
 * A Function that can be registered with Py_AtExit() that will dump the tracker state
 * when the Python interpreter is torn down.
 * NOTE: This should not call any CPython APIs as the interpreter is in an uncertain state.
 */
static void cTrackAllocs_dump_remaining_atexit(void) {
    std::cout << __FUNCTION__ << "() AT EXIT START:" << std::endl;
    std::cout << "File: " << __FILE__ << " Line: " << __LINE__ << std::endl;
    std::cout << s_NewAndDeallocTracker.dump_remaining() << std::endl;
    std::cout << __FUNCTION__ << "() AT EXIT DONE" << std::endl;
}
#endif

typedef struct {
    PyObject_HEAD
    PyObject *pBytes; /* Just allocate some bytes to use memory. */
} ObjectWithBytes;

/** Forward declaration. */
//static PyTypeObject ObjectWithBytes_Type;
#define ObjectWithBytes_Check(v) (Py_TYPE(v) == &ObjectWithBytes_Type)

static void
ObjectWithBytes_dealloc(ObjectWithBytes *self) {
#if TRACK_ALLOCS_AND_DEALLOCS
    s_NewAndDeallocTracker.add_dealloc((PyObject*)self);
#endif
    Py_XDECREF(self->pBytes);
    PyObject_Del(self);
}

// typedef PyObject *(*newfunc)(PyTypeObject *, PyObject *, PyObject *);
// See: https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_new
static PyObject *
ObjectWithBytes_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    ObjectWithBytes *self;
    self = (ObjectWithBytes *) type->tp_alloc(type, 0);
    if (self == NULL) {
        return NULL;
    }
    self->pBytes = NULL;
#if TRACK_ALLOCS_AND_DEALLOCS
    s_NewAndDeallocTracker.add_new((PyObject*)self, __FUNCTION__, __FILE__, __LINE__);
#endif
    return (PyObject *) self;
}

static int
ObjectWithBytes_init(ObjectWithBytes *self, PyObject *args, PyObject *kwds) {
    static const char *kwlist[] = {"length", NULL};
    long long length = -1;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "k", (char **)kwlist, &length)) {
        return -1;
    }
    self->pBytes = PyBytes_FromStringAndSize(NULL, length);
    return 0;
}

static PyObject *
ObjectWithBytes___str__(ObjectWithBytes *self, PyObject *Py_UNUSED(ignored)) {
    assert(!PyErr_Occurred());
    return PyUnicode_FromFormat(
            "<ObjectWithBytes @: %p with bytes of size %ld index %ld",
            self, PyBytes_Size(self->pBytes)
    );
}

static PyMethodDef ObjectWithBytes_methods[] = {
        {NULL, NULL, 0, NULL} /* sentinel */
};

static PyTypeObject ObjectWithBytes_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "cTrackAllocs.ObjectWithBytes",
        .tp_basicsize = sizeof(ObjectWithBytes),
        .tp_itemsize = 0,
        .tp_dealloc = (destructor) ObjectWithBytes_dealloc,
        .tp_str = (reprfunc) ObjectWithBytes___str__,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = "Object containing a bytes object.",
        .tp_methods = ObjectWithBytes_methods,
        .tp_init = (initproc) ObjectWithBytes_init,
        .tp_new = ObjectWithBytes_new,
};

/* List of functions defined in the module */
static PyMethodDef cTrackAllocs_functions[] = {
#if TRACK_ALLOCS_AND_DEALLOCS
        {"dump_remaining", (PyCFunction) cTrackAllocs_dump_remaining, METH_NOARGS,
         PyDoc_STR("Returns a string describing the remaining allocations.")},
        {"statistics", (PyCFunction) cTrackAllocs_statistics, METH_NOARGS,
         PyDoc_STR("A tuple of (total_new, total_tp_basicsize, total_dealloc, max_allocs).")},
#endif
        {NULL, NULL, 0, NULL}           /* sentinel */
};

PyDoc_STRVAR(cTrackAllocs_doc, "cTrackAllocs is an example of tracking allocations and de-allocations.");

static struct PyModuleDef cObject = {
        PyModuleDef_HEAD_INIT,
        "cTrackAllocs",
        cTrackAllocs_doc,
        -1,
        cTrackAllocs_functions,
        NULL,
        NULL,
        NULL,
        NULL
};

PyMODINIT_FUNC
PyInit_cTrackAllocs(void) {
    PyObject *m = NULL;
    /* Create the module and add the functions */
    m = PyModule_Create(&cObject);
    if (m == NULL) {
        goto fail;
    }
    /* Finalize the type object including setting type of the new type
     * object; doing it here is required for portability, too. */
    if (PyType_Ready(&ObjectWithBytes_Type) < 0) {
        goto fail;
    }
    if (PyModule_AddObject(m, "ObjectWithBytes", (PyObject *) &ObjectWithBytes_Type)) {
        goto fail;
    }
#if TRACK_ALLOCS_AND_DEALLOCS
    if (Py_AtExit(&cTrackAllocs_dump_remaining_atexit)) {
        goto fail;
    }
#endif
    return m;
    fail:
    Py_XDECREF(m);
    return NULL;
}
