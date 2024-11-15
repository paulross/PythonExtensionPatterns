/* A context manager example. */

/* MyObj objects */

#define PY_SSIZE_T_CLEAN

#include "Python.h"

static const ssize_t BUFFER_LENGTH = (ssize_t)1024 * 1024 * 128;

typedef struct {
    PyObject_HEAD
    /* Buffer created for the lifetime of the object. A memory check can show leaks. */
    char *buffer_lifetime;
    /* Buffer created for the lifetime of the context. A memory check can show leaks. */
    char *buffer_context;
} ContextManager;

/** Forward declaration. */
static PyTypeObject ContextManager_Type;

#define ContextManager_Check(v)      (Py_TYPE(v) == &ContextManager_Type)

static ContextManager *
ContextManager_new(PyObject *Py_UNUSED(arg)) {
    ContextManager *self;
    self = PyObject_New(ContextManager, &ContextManager_Type);
    if (self == NULL) {
        return NULL;
    }
    self->buffer_lifetime = malloc(BUFFER_LENGTH);
    // Force an initialisation.
    for (ssize_t i = 0; i < BUFFER_LENGTH; ++i) {
        self->buffer_lifetime[i] = ' ';
    }
    self->buffer_context = NULL;
//    fprintf(stdout, "%24s DONE REFCNT = %zd\n", __FUNCTION__, Py_REFCNT(self));
    return self;
}

/* ContextManager methods */
static void
ContextManager_dealloc(ContextManager *self) {
//    fprintf(stdout, "%24s STRT REFCNT = %zd\n", __FUNCTION__, Py_REFCNT(self));
    free(self->buffer_lifetime);
    self->buffer_lifetime = NULL;
    assert(self->buffer_context == NULL);
    PyObject_Del(self);
//    fprintf(stdout, "%24s DONE REFCNT = %zd\n", __FUNCTION__, Py_REFCNT(self));
}

static PyObject *
ContextManager_enter(ContextManager *self, PyObject *Py_UNUSED(args)) {
    assert(self->buffer_lifetime != NULL);
    assert(self->buffer_context == NULL);
//    fprintf(stdout, "%24s STRT REFCNT = %zd\n", __FUNCTION__, Py_REFCNT(self));
    self->buffer_context = malloc(BUFFER_LENGTH);
    // Force an initialisation.
    for (ssize_t i = 0; i < BUFFER_LENGTH; ++i) {
        self->buffer_context[i] = ' ';
    }
    Py_INCREF(self);
//    fprintf(stdout, "%24s DONE REFCNT = %zd\n", __FUNCTION__, Py_REFCNT(self));
    return (PyObject *)self;
}

static PyObject *
ContextManager_exit(ContextManager *self, PyObject *Py_UNUSED(args)) {
    assert(self->buffer_lifetime != NULL);
    assert(self->buffer_context != NULL);
//    fprintf(stdout, "%24s STRT REFCNT = %zd\n", __FUNCTION__, Py_REFCNT(self));
    free(self->buffer_context);
    self->buffer_context = NULL;
//    fprintf(stdout, "%24s DONE REFCNT = %zd\n", __FUNCTION__, Py_REFCNT(self));
    Py_RETURN_FALSE;
}

static PyObject *
ContextManager_len_buffer_lifetime(ContextManager *self, PyObject *Py_UNUSED(args)) {
    return Py_BuildValue("n", self->buffer_lifetime ? BUFFER_LENGTH : 0);
}

static PyObject *
ContextManager_len_buffer_context(ContextManager *self, PyObject *Py_UNUSED(args)) {
    return Py_BuildValue("n", self->buffer_context ? BUFFER_LENGTH : 0);
}

static PyMethodDef ContextManager_methods[] = {
        {"__enter__", (PyCFunction) ContextManager_enter, METH_NOARGS,
                        PyDoc_STR("__enter__() -> ContextManager")},
        {"__exit__", (PyCFunction) ContextManager_exit, METH_VARARGS,
                        PyDoc_STR("__exit__(exc_type, exc_value, exc_tb) -> bool")},
        {"len_buffer_lifetime", (PyCFunction) ContextManager_len_buffer_lifetime, METH_NOARGS,
                        PyDoc_STR("len_buffer_lifetime() -> int")},
        {"len_buffer_context", (PyCFunction) ContextManager_len_buffer_context, METH_NOARGS,
                        PyDoc_STR("len_buffer_context() -> int")},
        {NULL, NULL, 0, NULL} /* sentinel */
};

static PyTypeObject ContextManager_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "cObject.ContextManager",
        .tp_basicsize = sizeof(ContextManager),
        .tp_dealloc = (destructor) ContextManager_dealloc,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_methods = ContextManager_methods,
        .tp_new = (newfunc) ContextManager_new,
};

PyDoc_STRVAR(module_doc, "Example of a context manager.");

static struct PyModuleDef cCtxMgr = {
        PyModuleDef_HEAD_INIT,
        .m_name = "cCtxMgr",
        .m_doc = module_doc,
        .m_size = -1,
};

PyMODINIT_FUNC
PyInit_cCtxMgr(void) {
    PyObject *m = NULL;
    /* Create the module and add the functions */
    m = PyModule_Create(&cCtxMgr);
    if (m == NULL) {
        goto fail;
    }
    /* Finalize the type object including setting type of the new type
     * object; doing it here is required for portability, too. */
    if (PyType_Ready(&ContextManager_Type) < 0) {
        goto fail;
    }
    if (PyModule_AddObject(m, "ContextManager", (PyObject *) &ContextManager_Type)) {
        goto fail;
    }
    if (PyModule_AddObject(m, "BUFFER_LENGTH", Py_BuildValue("n", BUFFER_LENGTH))) {
        goto fail;
    }
    return m;
fail:
    Py_XDECREF(m);
    return NULL;
}
