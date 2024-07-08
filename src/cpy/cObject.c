/* Use this file as a template to start implementing a module that
   also declares object types. All occurrences of 'MyObj' should be changed
   to something reasonable for your objects. After that, all other
   occurrences of 'cObject' should be changed to something reasonable for your
   module. If your module is named foo your sourcefile should be named
   foomodule.c.

   You will probably want to delete all references to 'x_attr' and add
   your own types of attributes instead.  Maybe you want to name your
   local variables other than 'self'.  If your object type is needed in
   other files, you'll have to create a file "foobarobject.h"; see
   floatobject.h for an example. */

/* MyObj objects */

#include "Python.h"

static PyObject *ErrorObject;

typedef struct {
    PyObject_HEAD
    PyObject *x_attr; /* Attributes dictionary, NULL on construction, will be populated by MyObj_getattro. */
} ObjectWithAttributes;

/** Forward declaration. */
static PyTypeObject ObjectWithAttributes_Type;

#define ObjectWithAttributes_Check(v)      (Py_TYPE(v) == &ObjectWithAttributes_Type)

static ObjectWithAttributes *
ObjectWithAttributes_new(PyObject *Py_UNUSED(arg)) {
    ObjectWithAttributes *self;
    self = PyObject_New(ObjectWithAttributes, &ObjectWithAttributes_Type);
    if (self == NULL) {
        return NULL;
    }
    self->x_attr = NULL;
    return self;
}

/* ObjectWithAttributes methods */
static void
ObjectWithAttributes_dealloc(ObjectWithAttributes *self) {
    Py_XDECREF(self->x_attr);
    PyObject_Del(self);
}

static PyObject *
ObjectWithAttributes_demo(ObjectWithAttributes *Py_UNUSED(self), PyObject *args) {
    if (!PyArg_ParseTuple(args, ":demo")) {
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef ObjectWithAttributes_methods[] = {
        {"demo", (PyCFunction) ObjectWithAttributes_demo, METH_VARARGS,
                        PyDoc_STR("demo() -> None")},
        {NULL, NULL, 0, NULL} /* sentinel */
};

static PyObject *
ObjectWithAttributes_getattro(ObjectWithAttributes *self, PyObject *name) {
    if (self->x_attr != NULL) {
        PyObject *v = PyDict_GetItem(self->x_attr, name);
        if (v != NULL) {
            Py_INCREF(v);
            return v;
        }
    }
    return PyObject_GenericGetAttr((PyObject *) self, name);
}

static int
ObjectWithAttributes_setattr(ObjectWithAttributes *self, char *name, PyObject *v) {
    if (self->x_attr == NULL) {
        self->x_attr = PyDict_New();
        if (self->x_attr == NULL)
            return -1;
    }
    if (v == NULL) {
        int rv = PyDict_DelItemString(self->x_attr, name);
        if (rv < 0)
            PyErr_SetString(PyExc_AttributeError,
                            "delete non-existing ObjectWithAttributes attribute");
        return rv;
    } else
        /* v is a borrowed reference, then PyDict_SetItemString() does NOT steal it so nothing to do. */
        return PyDict_SetItemString(self->x_attr, name, v);
}

static PyTypeObject ObjectWithAttributes_Type = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(NULL, 0)
        "cObject.ObjectWithAttributes",             /*tp_name*/
        sizeof(ObjectWithAttributes),          /*tp_basicsize*/
        0,                          /*tp_itemsize*/
        /* methods */
        (destructor) ObjectWithAttributes_dealloc,    /*tp_dealloc*/
#if PY_MINOR_VERSION < 8
        0,                          /*tp_print*/
#else
        0,                          /* Py_ssize_t tp_vectorcall_offset; */
#endif
        (getattrfunc) 0,             /*tp_getattr*/
        (setattrfunc) ObjectWithAttributes_setattr,   /*tp_setattr*/
        0,                          /*tp_reserved*/
        0,                          /*tp_repr*/
        0,                          /*tp_as_number*/
        0,                          /*tp_as_sequence*/
        0,                          /*tp_as_mapping*/
        0,                          /*tp_hash*/
        0,                          /*tp_call*/
        0,                          /*tp_str*/
        (getattrofunc) ObjectWithAttributes_getattro, /*tp_getattro*/
        0,                          /*tp_setattro*/
        0,                          /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT,         /*tp_flags*/
        0,                          /*tp_doc*/
        0,                          /*tp_traverse*/
        0,                          /*tp_clear*/
        0,                          /*tp_richcompare*/
        0,                          /*tp_weaklistoffset*/
        0,                          /*tp_iter*/
        0,                          /*tp_iternext*/
        ObjectWithAttributes_methods,                /*tp_methods*/
        0,                          /*tp_members*/
        0,                          /*tp_getset*/
        0,                          /*tp_base*/
        0,                          /*tp_dict*/
        0,                          /*tp_descr_get*/
        0,                          /*tp_descr_set*/
        0,                          /*tp_dictoffset*/
        0,                          /*tp_init*/
        0,                          /*tp_alloc*/
//    PyType_GenericNew,          /*tp_new*/
        (newfunc) ObjectWithAttributes_new,          /*tp_new*/
        0,                          /*tp_free*/
        0,                          /*tp_is_gc*/
        NULL,                   /* tp_bases */
        NULL,                   /* tp_mro */
        NULL,                   /* tp_cache */
        NULL,               /* tp_subclasses */
        NULL,                    /* tp_weaklist */
        NULL,                       /* tp_del */
        0,                  /* tp_version_tag */
        NULL,                   /* tp_finalize */
#if PY_MINOR_VERSION > 7
        NULL,                   /* tp_vectorcall */
#endif
#if PY_MINOR_VERSION == 8
        0,                          /*tp_print*/
#endif
#if PY_MINOR_VERSION >= 12
        '\0',                   /* unsigned char tp_watched */
#if PY_MINOR_VERSION >= 13
        0,                      /* uint16_t tp_versions_used */
#endif
#endif
};
/* --------------------------------------------------------------------- */

/* Function of two integers returning integer */

#if 0
PyDoc_STRVAR(cObject_foo_doc,
             "foo(i,j)\n\
\n\
Return the sum of i and j.");

static PyObject *
cObject_foo(PyObject *Py_UNUSED(self), PyObject *args) {
    long i, j;
    long res;
    if (!PyArg_ParseTuple(args, "ll:foo", &i, &j)) {
        return NULL;
    }
    res = i + j; /* cObjX Do something here */
    return PyLong_FromLong(res);
}


/* Function of no arguments returning new MyObj object */
static PyObject *
cObject_ObjectWithAttributes_new(PyObject *Py_UNUSED(self), PyObject *args) {
    ObjectWithAttributes *rv;

    if (!PyArg_ParseTuple(args, ":new")) {
        return NULL;
    }
    rv = ObjectWithAttributes_new(args);
    if (rv == NULL) {
        return NULL;
    }
    return (PyObject *) rv;
}

/* Example with subtle bug from extensions manual ("Thin Ice"). */
static PyObject *
cObject_thin_ice_bug(PyObject *Py_UNUSED(self), PyObject *args) {
    PyObject *list, *item;

    if (!PyArg_ParseTuple(args, "O:bug", &list))
        return NULL;

    item = PyList_GetItem(list, 0);
    /* Py_INCREF(item); */
    PyList_SetItem(list, 1, PyLong_FromLong(0L));
    PyObject_Print(item, stdout, 0);
    printf("\n");
    /* Py_DECREF(item); */

    Py_INCREF(Py_None);
    return Py_None;
}

/* Test bad format character */
static PyObject *
cObject_roj(PyObject *Py_UNUSED(self), PyObject *args) {
    PyObject *a;
    long b;
    if (!PyArg_ParseTuple(args, "O#:roj", &a, &b)) {
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}
#endif

/* ---------- */

static PyTypeObject Str_Type = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(NULL, 0)
        "cObject.Str",             /*tp_name*/
        0,                          /*tp_basicsize*/
        0,                          /*tp_itemsize*/
        /* methods */
        0,                          /*tp_dealloc*/
#if PY_MINOR_VERSION < 8
        0,                          /*tp_print*/
#else
        0,                          /* Py_ssize_t tp_vectorcall_offset; */
#endif
        0,                          /*tp_getattr*/
        0,                          /*tp_setattr*/
        0,                          /*tp_reserved*/
        0,                          /*tp_repr*/
        0,                          /*tp_as_number*/
        0,                          /*tp_as_sequence*/
        0,                          /*tp_as_mapping*/
        0,                          /*tp_hash*/
        0,                          /*tp_call*/
        0,                          /*tp_str*/
        0,                          /*tp_getattro*/
        0,                          /*tp_setattro*/
        0,                          /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
        0,                          /*tp_doc*/
        0,                          /*tp_traverse*/
        0,                          /*tp_clear*/
        0,                          /*tp_richcompare*/
        0,                          /*tp_weaklistoffset*/
        0,                          /*tp_iter*/
        0,                          /*tp_iternext*/
        0,                          /*tp_methods*/
        0,                          /*tp_members*/
        0,                          /*tp_getset*/
        0, /* see PyInit_cObject */      /*tp_base*/
        0,                          /*tp_dict*/
        0,                          /*tp_descr_get*/
        0,                          /*tp_descr_set*/
        0,                          /*tp_dictoffset*/
        0,                          /*tp_init*/
        0,                          /*tp_alloc*/
        0,                          /*tp_new*/
        0,                          /*tp_free*/
        0,                          /*tp_is_gc*/
        NULL,                   /* tp_bases */
        NULL,                   /* tp_mro */
        NULL,                   /* tp_cache */
        NULL,               /* tp_subclasses */
        NULL,                    /* tp_weaklist */
        NULL,                       /* tp_del */
        0,                  /* tp_version_tag */
        NULL,                   /* tp_finalize */
#if PY_MINOR_VERSION > 7
        NULL,                   /* tp_vectorcall */
#endif
#if PY_MINOR_VERSION == 8
        0,                          /*tp_print*/
#endif
#if PY_MINOR_VERSION >= 12
        '\0',                   /* unsigned char tp_watched */
#if PY_MINOR_VERSION >= 13
        0,                      /* uint16_t tp_versions_used */
#endif
#endif
};

/* ---------- */

static PyObject *
null_richcompare(PyObject *Py_UNUSED(self), PyObject *Py_UNUSED(other), int Py_UNUSED(op)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
}

static PyTypeObject Null_Type = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(NULL, 0)
        "cObject.Null",            /*tp_name*/
        0,                          /*tp_basicsize*/
        0,                          /*tp_itemsize*/
        /* methods */
#if PY_MINOR_VERSION < 8
        0,                          /*tp_print*/
#else
        0,                          /* Py_ssize_t tp_vectorcall_offset; */
#endif
        0,                          /*tp_print*/
        0,                          /*tp_getattr*/
        0,                          /*tp_setattr*/
        0,                          /*tp_reserved*/
        0,                          /*tp_repr*/
        0,                          /*tp_as_number*/
        0,                          /*tp_as_sequence*/
        0,                          /*tp_as_mapping*/
        0,                          /*tp_hash*/
        0,                          /*tp_call*/
        0,                          /*tp_str*/
        0,                          /*tp_getattro*/
        0,                          /*tp_setattro*/
        0,                          /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
        0,                          /*tp_doc*/
        0,                          /*tp_traverse*/
        0,                          /*tp_clear*/
        null_richcompare,           /*tp_richcompare*/
        0,                          /*tp_weaklistoffset*/
        0,                          /*tp_iter*/
        0,                          /*tp_iternext*/
        0,                          /*tp_methods*/
        0,                          /*tp_members*/
        0,                          /*tp_getset*/
        0, /* see PyInit_cObject */      /*tp_base*/
        0,                          /*tp_dict*/
        0,                          /*tp_descr_get*/
        0,                          /*tp_descr_set*/
        0,                          /*tp_dictoffset*/
        0,                          /*tp_init*/
        0,                          /*tp_alloc*/
        0, /* see PyInit_cObject */      /*tp_new*/
        0,                          /*tp_free*/
        0,                          /*tp_is_gc*/
        NULL,                   /* tp_bases */
        NULL,                   /* tp_mro */
        NULL,                   /* tp_cache */
        NULL,               /* tp_subclasses */
        NULL,                    /* tp_weaklist */
        NULL,                       /* tp_del */
        0,                  /* tp_version_tag */
        NULL,                   /* tp_finalize */
#if PY_MINOR_VERSION > 7
        NULL,                   /* tp_vectorcall */
#endif
#if PY_MINOR_VERSION == 8
        0,                          /*tp_print*/
#endif
#if PY_MINOR_VERSION >= 12
        '\0',                   /* unsigned char tp_watched */
#if PY_MINOR_VERSION >= 13
        0,                      /* uint16_t tp_versions_used */
#endif
#endif
};


/* ---------- */


/* List of functions defined in the module */
static PyMethodDef cObject_functions[] = {
#if 0
        {"roj",                      cObject_roj,                      METH_VARARGS,
                        PyDoc_STR("roj(a,b) -> None")},
        {"foo",                      cObject_foo,                      METH_VARARGS,
                cObject_foo_doc},
        {"new_ObjectWithAttributes", cObject_ObjectWithAttributes_new, METH_VARARGS,
                        PyDoc_STR("new() -> new ObjectWithAttributes object")},
        {"thin_ice_bug", cObject_thin_ice_bug, METH_VARARGS,
                        PyDoc_STR("bug(o) -> None")},
#endif
        {NULL, NULL, 0, NULL}           /* sentinel */
};

PyDoc_STRVAR(module_doc, "This is a template module just for instruction.");

/* Initialization function for the module (*must* be called PyInit_cObject) */


static struct PyModuleDef cObject = {
        PyModuleDef_HEAD_INIT,
        "cObject",
        module_doc,
        -1,
        cObject_functions,
        NULL,
        NULL,
        NULL,
        NULL
};

PyMODINIT_FUNC
PyInit_cObject(void) {
    PyObject *m = NULL;

    /* Due to cross-platform compiler issues the slots must be filled
     * here. It's required for portability to Windows without requiring
     * C++. */
    Null_Type.tp_base = &PyBaseObject_Type;
    Null_Type.tp_new = PyType_GenericNew;
    Str_Type.tp_base = &PyUnicode_Type;

    /* Create the module and add the functions */
    m = PyModule_Create(&cObject);
    if (m == NULL) {
        goto fail;
    }
    /* Add some symbolic constants to the module */
    if (ErrorObject == NULL) {
        ErrorObject = PyErr_NewException("cObject.error", NULL, NULL);
        if (ErrorObject == NULL)
            goto fail;
    }
    Py_INCREF(ErrorObject);
    if (PyModule_AddObject(m, "error", ErrorObject)) {
        goto fail;
    }
    /* Finalize the type object including setting type of the new type
     * object; doing it here is required for portability, too. */
    if (PyType_Ready(&ObjectWithAttributes_Type) < 0) {
        goto fail;
    }
    if (PyModule_AddObject(m, "ObjectWithAttributes", (PyObject *) &ObjectWithAttributes_Type)) {
        goto fail;
    }
    /* Add Str */
    if (PyType_Ready(&Str_Type) < 0) {
        goto fail;
    }
    if (PyModule_AddObject(m, "Str", (PyObject *) &Str_Type)) {
        goto fail;
    }
    /* Add Null */
    if (PyType_Ready(&Null_Type) < 0) {
        goto fail;
    }
    if (PyModule_AddObject(m, "Null", (PyObject *) &Null_Type)) {
        goto fail;
    }
    return m;
    fail:
    Py_XDECREF(m);
    return NULL;
}
