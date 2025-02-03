//
// Created by Paul Ross on 31/01/2025.
//
// Provides Python accessible watchers.

#define PPY_SSIZE_T_CLEAN

#include "Python.h"

/* Version as a single 4-byte hex number, e.g. 0x010502B2 == 1.5.2b2
 * Therefore 0x030C0000 == 3.12.0
 */
#if PY_VERSION_HEX < 0x030C0000

#error "Required version of Python is 3.12+ (PY_VERSION_HEX >= 0x030C0000)"

#else

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
} PyDictWatcher;

/** Forward declaration. */
static PyTypeObject PyDictWatcher_Type;

#define PyDictWatcher_Check(v)      (Py_TYPE(v) == &PyDictWatcher_Type)

static PyDictWatcher *
PyDictWatcher_new(PyObject *Py_UNUSED(arg)) {
    PyDictWatcher *self;
    self = PyObject_New(PyDictWatcher, &PyDictWatcher_Type);
    if (self == NULL) {
        return NULL;
    }
    self->watcher_id = -1;
    self->dict = NULL;
    return self;
}

static PyObject *
PyDictWatcher_init(PyDictWatcher *self, PyObject *args) {
    if (!PyArg_ParseTuple(args, "O", &self->dict)) {
        return NULL;
    }
    if (!PyDict_Check(self->dict)) {
        PyErr_Format(PyExc_TypeError, "Argument must be a dictionary not a %s", Py_TYPE(self->dict)->tp_name);
        return NULL;
    }
    Py_INCREF(self->dict);
    return (PyObject *)self;
}

static void
PyDictWatcher_dealloc(PyDictWatcher *self) {
    Py_DECREF(self->dict);
    PyObject_Del(self);
}

static PyObject *
PyDictWatcher_enter(PyDictWatcher *self, PyObject *Py_UNUSED(args)) {
    self->watcher_id = dict_watcher_verbose_add(self->dict);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *
PyDictWatcher_exit(PyDictWatcher *self, PyObject *Py_UNUSED(args)) {
    int result = dict_watcher_verbose_remove(self->watcher_id, self->dict);
    if (result) {
        PyErr_Format(PyExc_RuntimeError, "dict_watcher_verbose_remove() returned %d", result);
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyMethodDef PyDictWatcher_methods[] = {
        {"__enter__", (PyCFunction) PyDictWatcher_enter, METH_VARARGS,
                PyDoc_STR("__enter__() -> PyDictWatcher")},
        {"__exit__", (PyCFunction) PyDictWatcher_exit, METH_VARARGS,
                PyDoc_STR("__exit__(exc_type, exc_value, exc_tb) -> bool")},
        {NULL, NULL, 0, NULL} /* sentinel */
};

static PyTypeObject PyDictWatcher_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "cWatchers.PyDictWatcher",
        .tp_basicsize = sizeof(PyDictWatcher),
        .tp_dealloc = (destructor) PyDictWatcher_dealloc,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_methods = PyDictWatcher_methods,
        .tp_new = (newfunc) PyDictWatcher_new,
        .tp_init = (initproc) PyDictWatcher_init
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
    if (PyType_Ready(&PyDictWatcher_Type) < 0) {
        goto fail;
    }
    if (PyModule_AddObject(m, "PyDictWatcher", (PyObject *) &PyDictWatcher_Type)) {
        goto fail;
    }
    return m;
fail:
    Py_XDECREF(m);
    return NULL;
}

#endif // #if PY_VERSION_HEX >= 0x030C0000
