//
// Created by Paul Ross on 06/09/2022.
//

#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "structmember.h"

#include <string>
#include <iostream>

/**
 * A simple class that contains a string but reports its method calls.
 */
class Verbose {
public:
    Verbose() : Verbose("Default") {
        std::cout << "Default constructor at " << std::hex << (void *) this << std::dec;
        std::cout << " with argument \"" << m_str << "\"" << std::endl;
    }

    explicit Verbose(const std::string &str) : m_str(str), m_buffer(1024 * 1024 * 64, ' ') {
        std::cout << "Constructor at " << std::hex << (void *) this << std::dec;
        std::cout << " with argument \"" << m_str << "\"" << std::endl;
    }

    Verbose &operator=(const Verbose &rhs) {
        std::cout << "operator= at " << std::hex << (void *) this << std::dec;
        std::cout << " m_str: \"" << m_str << "\"";
        std::cout << " rhs at " << std::hex << (void *) &rhs << std::dec;
        std:: cout << " rhs.m_str: \"" << rhs.m_str << "\"" << std::endl;
        if (this != &rhs) {
            m_str = rhs.m_str;
        }
        return *this;
    }

    void print(const char *message = NULL) {
        if (message) {
            std::cout << message << ": Verbose object at " << std::hex << (void *) this << std::dec;
            std::cout << " m_str: \"" << m_str << "\"" << std::endl;
        } else {
            std::cout << " Verbose object at " << std::hex << (void *) this << std::dec;
            std::cout << " m_str: \"" << m_str << "\"" << std::endl;
        }
    }

    ~Verbose() {
        std::cout << "Destructor at " << std::hex << (void *) this << std::dec;
        std::cout << " m_str: \"" << m_str << "\"" << std::endl;
    }

private:
    std::string m_str;
    // m_buffer is just a large string to provoke the memory manager and detect leaks.
    std::string m_buffer;
};

typedef struct {
    PyObject_HEAD
    Verbose Attr;
    Verbose *pAttr;
} CppCtorDtorInPyObject;

static PyObject *
CppCtorDtorInPyObject_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
    printf("-- %s()\n", __FUNCTION__);
    CppCtorDtorInPyObject *self;
    self = (CppCtorDtorInPyObject *) type->tp_alloc(type, 0);
    if (self != NULL) {
        // Placement new
        new(&self->Attr) Verbose;
        self->Attr.print("Initial self->Attr");
        self->pAttr = new Verbose("pAttr");
        if (self->pAttr == NULL) {
            Py_DECREF(self);
            return NULL;
        } else {
            self->pAttr->print("Initial self->pAttr");
        }
    }
    return (PyObject *) self;
}

//static int
//CppCtorDtorInPyObject_init(CppCtorDtorInPyObject *Py_UNUSED(self), PyObject *Py_UNUSED(args),
//                           PyObject *Py_UNUSED(kwds)) {
//    printf("-- %s()\n", __FUNCTION__);
//    return 0;
//}

static void
CppCtorDtorInPyObject_dealloc(CppCtorDtorInPyObject *self) {
    printf("-- %s()\n", __FUNCTION__);
    self->Attr.print("self->Attr before delete");
    self->Attr.~Verbose();
//    delete (&self->Attr);// self->Attr;
//    ::operator delete (&self->Attr);// self->Attr;
    self->pAttr->print("self->pAttr before delete");
    delete self->pAttr;
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
CppCtorDtorInPyObject_print(CppCtorDtorInPyObject *self, PyObject *Py_UNUSED(ignored)) {
    printf("-- %s()\n", __FUNCTION__);
    self->Attr.print("self->Attr");
    self->pAttr->print("self->pAttr");
    Py_RETURN_NONE;
}

static PyMethodDef CppCtorDtorInPyObject_methods[] = {
        {"print", (PyCFunction) CppCtorDtorInPyObject_print, METH_NOARGS,
                "Print the contents of the object."
        },
        {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyTypeObject CppCtorDtorInPyObjectType = {
        PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "CppCtorDtorInPyObject.CppCtorDtorInPyObject",
        .tp_basicsize = sizeof(CppCtorDtorInPyObject),
        .tp_itemsize = 0,
        .tp_dealloc = (destructor) CppCtorDtorInPyObject_dealloc,
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = "CppCtorDtorInPyObject object",
        .tp_methods = CppCtorDtorInPyObject_methods,
//        .tp_init = (initproc) CppCtorDtorInPyObject_init,
        .tp_new = CppCtorDtorInPyObject_new,
};

static PyModuleDef cpp_module = {
        PyModuleDef_HEAD_INIT,
        .m_name = "CppCtorDtorInPyObject",
        .m_doc = "Example module that creates an C++ extension type containing custom objects.",
        .m_size = -1,
};

PyMODINIT_FUNC
PyInit_CppCtorDtorInPyObject(void) {
//    printf("-- %s()\n", __FUNCTION__);
    PyObject * m = PyModule_Create(&cpp_module);
    if (m == NULL) {
        return NULL;
    }

    if (PyType_Ready(&CppCtorDtorInPyObjectType) < 0) {
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&CppCtorDtorInPyObjectType);
    if (PyModule_AddObject(m, "CppCtorDtorInPyObject", (PyObject *) &CppCtorDtorInPyObjectType) < 0) {
        Py_DECREF(&CppCtorDtorInPyObjectType);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
