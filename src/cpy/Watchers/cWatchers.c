//
// Created by Paul Ross on 31/01/2025.
//
// Provides Python accessible watchers.

#define PPY_SSIZE_T_CLEAN

#include "Python.h"

#pragma mark Dictionary Watcher

#include "DictWatcher.h"

static PyObject *
py_dict_watcher_verbose_add(PyObject *Py_UNUSED(module), PyObject *arg) {
    if (!PyDict_Check(arg)) {
        PyErr_Format(PyExc_TypeError, "Argument must be a dict not type %s", Py_TYPE(arg)->tp_name);
        return NULL;
    }
    long watcher_id = dict_watcher_verbose_add(arg);
    return Py_BuildValue("l", watcher_id);
}


static PyObject *
py_dict_watcher_verbose_remove(PyObject *Py_UNUSED(module), PyObject *args) {
    long watcher_id;
    PyObject *dict = NULL;

    if (!PyArg_ParseTuple(args, "lO", &watcher_id, &dict)) {
        return NULL;
    }

    if (!PyDict_Check(dict)) {
        PyErr_Format(PyExc_TypeError, "Argument must be a dict not type %s", Py_TYPE(dict)->tp_name);
        return NULL;
    }
    long result = dict_watcher_verbose_remove(watcher_id, dict);
    return Py_BuildValue("l", result);
}

#pragma mark Dictionary Watcher Context Manager

typedef struct {
    PyObject_HEAD
    int watcher_id;
    PyObject *dict;
} PyDictWatcherContextManager;

/** Forward declaration. */
static PyTypeObject PyDictWatcherContextManager_Type;

#define PyDictWatcherContextManager_Check(v)      (Py_TYPE(v) == &PyDictWatcherContextManager_Type)

static PyDictWatcherContextManager *
PyDictWatcherContextManager_new(PyObject *Py_UNUSED(arg)) {
    PyDictWatcherContextManager *self;
    self = PyObject_New(PyDictWatcherContextManager, &PyDictWatcherContextManager_Type);
    if (self == NULL) {
        return NULL;
    }
    self->watcher_id = -1;
    self->dict = NULL;
    return self;
}

static PyObject *
PyDictWatcherContextManager_enter(PyDictWatcherContextManager *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, "O", &self->dict)) {
        return NULL;
    }
    self->watcher_id = dict_watcher_verbose_add(self->dict);
    Py_INCREF(self);
    Py_INCREF(self->dict);
    return (PyObject *)self;
}

static PyObject *
PyDictWatcherContextManager_exit(PyDictWatcherContextManager *self, PyObject *Py_UNUSED(args)) {
    long result = dict_watcher_verbose_remove(self->watcher_id, self->dict);
    Py_DECREF(self->dict);
    if (result) {
        PyErr_Format(PyExc_RuntimeError, "dict_watcher_verbose_remove() returned %ld", result);
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyMethodDef PyDictWatcherContextManager_methods[] = {
        {"__enter__", (PyCFunction) PyDictWatcherContextManager_enter, METH_VARARGS,
                PyDoc_STR("__enter__() -> PyDictWatcherContextManager")},
        {"__exit__", (PyCFunction) PyDictWatcherContextManager_exit, METH_VARARGS,
                PyDoc_STR("__exit__(exc_type, exc_value, exc_tb) -> bool")},
        {NULL, NULL, 0, NULL} /* sentinel */
};

static PyTypeObject PyDictWatcherContextManager_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "cObject.ContextManager",
        .tp_basicsize = sizeof(PyDictWatcherContextManager),
//        .tp_dealloc = (destructor) PyDictWatcherContextManager_dealloc,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_methods = PyDictWatcherContextManager_methods,
        .tp_new = (newfunc) PyDictWatcherContextManager_new,
};

static PyMethodDef module_methods[] = {
        {"py_dict_watcher_verbose_add",
                (PyCFunction) py_dict_watcher_verbose_add,
                METH_O,
                "Adds watcher to a dictionary. Returns the watcher ID."
        },
        {"py_dict_watcher_verbose_remove",
                (PyCFunction) py_dict_watcher_verbose_remove,
                METH_VARARGS,
                "Removes the watcher ID from the dictionary."
        },
        {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef cWatchers = {
        PyModuleDef_HEAD_INIT,
        .m_name = "cWatchers",
        .m_doc = "Dictionary and type watchers.",
        .m_size = -1,
        .m_methods = module_methods,
};

PyMODINIT_FUNC PyInit_cWatchers(void) {
    PyObject *m = PyModule_Create(&cWatchers);
    if (!m) {
        goto fail;
    }
    if (PyType_Ready(&PyDictWatcherContextManager_Type) < 0) {
        goto fail;
    }
    if (PyModule_AddObject(m, "PyDictWatcherContextManager", (PyObject *) &PyDictWatcherContextManager_Type)) {
        goto fail;
    }
    return m;
fail:
    Py_XDECREF(m);
    return NULL;
}
