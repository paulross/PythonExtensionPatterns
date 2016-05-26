//
//  SubclassList.c
//  PythonSubclassList
//
//  Created by Paul Ross on 01/05/2016.
//  Copyright (c) 2016 Paul Ross. All rights reserved.
//
// Based on: file:///Library/Frameworks/Python.framework/Versions/3.3/Resources/English.lproj/Documentation/extending/newtypes.html#subclassing-other-types
// That describes the 'Shoddy' module and classs.
// This renames that to ScList ('sub-class list')and, instead of the increment
// menber this counts how many times append() is called and uses the super()
// class to call the base class append.

#include "SubclassList.h"

#include <Python.h>
#include "structmember.h"

#include "py_call_super.h"

typedef struct {
    PyListObject list;
    int appends;
} ScList;

static PyObject *
ScList_append(ScList *self, PyObject *args) {
    PyObject *result = call_super_name((PyObject *)self, "append",
                                       args, NULL);
    if (result) {
        self->appends++;
    }
    return result;
}

static PyMethodDef ScList_methods[] = {
    {"append", (PyCFunction)ScList_append, METH_VARARGS,
        PyDoc_STR("Append to the list")},
    {NULL,	NULL, 0, NULL},
};

static PyMemberDef ScList_members[] = {
    {"appends", T_INT, offsetof(ScList, appends), 0,
        "Number of append operations."},
    {NULL, 0, 0, 0, NULL}  /* Sentinel */
};

static int
ScList_init(ScList *self, PyObject *args, PyObject *kwds)
{
    if (PyList_Type.tp_init((PyObject *)self, args, kwds) < 0) {
        return -1;
    }
    self->appends = 0;
    return 0;
}

static PyTypeObject ScListType = {
    {PyObject_HEAD_INIT(NULL)},
    "ScList.ScList",         /* tp_name */
    sizeof(ScList),          /* tp_basicsize */
    0,                       /* tp_itemsize */
    0,                       /* tp_dealloc */
    0,                       /* tp_print */
    0,                       /* tp_getattr */
    0,                       /* tp_setattr */
    0,                       /* tp_reserved */
    0,                       /* tp_repr */
    0,                       /* tp_as_number */
    0,                       /* tp_as_sequence */
    0,                       /* tp_as_mapping */
    0,                       /* tp_hash */
    0,                       /* tp_call */
    0,                       /* tp_str */
    0,                       /* tp_getattro */
    0,                       /* tp_setattro */
    0,                       /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                       /* tp_doc */
    0,                       /* tp_traverse */
    0,                       /* tp_clear */
    0,                       /* tp_richcompare */
    0,                       /* tp_weaklistoffset */
    0,                       /* tp_iter */
    0,                       /* tp_iternext */
    ScList_methods,          /* tp_methods */
    ScList_members,          /* tp_members */
    0,                       /* tp_getset */
    0,                       /* tp_base */
    0,                       /* tp_dict */
    0,                       /* tp_descr_get */
    0,                       /* tp_descr_set */
    0,                       /* tp_dictoffset */
    (initproc)ScList_init,   /* tp_init */
    0,                       /* tp_alloc */
    0,                       /* tp_new */
    /* To suppress -Wmissing-field-initializers */
    0,                              /* tp_free */
    0,                              /* tp_is_gc */
    0,                              /* tp_bases */
    0,                              /* tp_mro */
    0,                              /* tp_cache */
    0,                              /* tp_subclasses */
    0,                              /* tp_weaklist */
    0,                              /* tp_del */
    0,                              /* tp_version_tag */
};

static PyModuleDef ScListmodule = {
    PyModuleDef_HEAD_INIT,
    "ScList",
    "ScList module",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC
PyInit_ScList(void)
{
    PyObject *m;
    
    ScListType.tp_base = &PyList_Type;
    if (PyType_Ready(&ScListType) < 0)
        return NULL;
    
    m = PyModule_Create(&ScListmodule);
    if (m == NULL)
        return NULL;
    
    Py_INCREF(&ScListType);
    PyModule_AddObject(m, "ScList", (PyObject *) &ScListType);
    return m;
}
