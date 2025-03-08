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

#if 0
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
        2,
};
#endif

#pragma mark - A basic Named Tuple

PyDoc_STRVAR(
        BasicNT_docstring,
        "A basic named tuple type with two fields."
);

static PyStructSequence_Field BasicNT_fields[] = {
        {"field_one", "The first field of the named tuple."},
        {"field_two", "The second field of the named tuple."},
        {NULL, NULL}
};

static PyStructSequence_Desc BasicNT_desc = {
        "cStructSequence.BasicNT",
        BasicNT_docstring,
        BasicNT_fields,
        2,
};

static PyObject *
BasicNT_create(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    assert(!PyErr_Occurred());
    static char *kwlist[] = {"field_one", "field_two", NULL};
    PyObject *field_one = NULL;
    PyObject *field_two = NULL;
    static PyTypeObject *static_BasicNT_Type = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO", kwlist, &field_one, &field_two)) {
        return NULL;
    }
    /* The two fields are PyObjects. If your design is that those arguments should be specific types
     * then take the opportunity here to test that they are the expected types.
     */
    if (!static_BasicNT_Type) {
        static_BasicNT_Type = PyStructSequence_NewType(&BasicNT_desc);
        if (!static_BasicNT_Type) {
            PyErr_SetString(
                    PyExc_MemoryError,
                    "Can not initialise a BasicNT type with PyStructSequence_NewType()"
            );
            return NULL;
        }
    }
    PyObject *result = PyStructSequence_New(static_BasicNT_Type);
    if (!result) {
        PyErr_SetString(
                PyExc_MemoryError,
                "Can not create a Struct Sequence with PyStructSequence_New()"
        );
        return NULL;
    }
    /* PyArg_ParseTupleAndKeywords with "O" gives a borrowed reference.
     * https://docs.python.org/3/c-api/arg.html#other-objects
     * "A new strong reference to the object is not created (i.e. its reference count is not increased)."
     * So we increment as PyStructSequence_SetItem seals the reference otherwise if the callers arguments
     * go out of scope we will/may get undefined behaviour when accessing the named tuple fields.
     */
    Py_INCREF(field_one);
    Py_INCREF(field_two);
    PyStructSequence_SetItem(result, 0, field_one);
    PyStructSequence_SetItem(result, 1, field_two);
    return result;
}

#pragma mark - A registered Named Tuple

PyDoc_STRVAR(
        NTRegistered_docstring,
        "A named tuple type with two fields that is"
        "registered with the cStructSequence module."
);

static PyStructSequence_Field NTRegistered_fields[] = {
        {"field_one", "The first field of the named tuple."},
        {"field_two", "The second field of the named tuple."},
        {NULL, NULL}
};

static PyStructSequence_Desc NTRegistered_desc = {
        "cStructSequence.NTRegistered",
        NTRegistered_docstring,
        NTRegistered_fields,
        2,
};

#pragma mark A un-registered Named Tuple

PyDoc_STRVAR(
        NTUnRegistered_docstring,
        "A named tuple type with two fields that is"
        " not registered with the cStructSequence module."
);

static PyStructSequence_Field NTUnRegistered_fields[] = {
        {"field_one", "The first field of the named tuple."},
        {"field_two", "The second field of the named tuple."},
        {NULL, NULL}
};

static PyStructSequence_Desc NTUnRegistered_desc = {
        "cStructSequence.NTUnRegistered",
        NTUnRegistered_docstring,
        NTUnRegistered_fields,
        2,
};

/* Type initailised dynamically by NTUnRegistered_create(). */
static PyTypeObject *static_NTUnRegisteredType = NULL;

/* A function that creates a cStructSequence.NTUnRegistered dynamically. */
static PyObject *
NTUnRegistered_create(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    assert(!PyErr_Occurred());
    static char *kwlist[] = {"field_one", "field_two", NULL};
    PyObject *field_one = NULL;
    PyObject *field_two = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO", kwlist, &field_one, &field_two)) {
        return NULL;
    }
    /* The two fields are PyObjects. If your design is that those arguments should be specific types
     * then take the opportunity here to test that they are the expected types.
     */
    /* Initialise the static static_NTUnRegisteredType.
     * Note: PyStructSequence_NewType returns a new reference.
     */
    if (!static_NTUnRegisteredType) {
        static_NTUnRegisteredType = PyStructSequence_NewType(&NTUnRegistered_desc);
        if (!static_NTUnRegisteredType) {
            PyErr_SetString(
                    PyExc_MemoryError,
                    "Can not initialise a type with PyStructSequence_NewType()"
            );
            return NULL;
        }
    }
    PyObject *result = PyStructSequence_New(static_NTUnRegisteredType);
    if (!result) {
        PyErr_SetString(
                PyExc_MemoryError,
                "Can not create a Struct Sequence with PyStructSequence_New()"
        );
        return NULL;
    }
    /* PyArg_ParseTupleAndKeywords with "O" gives a borrowed reference.
     * https://docs.python.org/3/c-api/arg.html#other-objects
     * "A new strong reference to the object is not created (i.e. its reference count is not increased)."
     * So we increment as PyStructSequence_SetItem seals the reference otherwise if the callers arguments
     * go out of scope we will/may get undefined behaviour when accessing the named tuple fields.
     */
    Py_INCREF(field_one);
    Py_INCREF(field_two);
    PyStructSequence_SetItem(result, 0, field_one);
    PyStructSequence_SetItem(result, 1, field_two);
    return result;
}

#pragma mark - Example of a C struct to PyStructSequence

/**
 * Representation of a simple transaction.
 */
struct cTransaction {
    long id;            /* The transaction id. */
    char *reference;    /* The transaction reference. */
    double amount;      /* The transaction amount. */
};

/**
 * An example function that might recover a transaction from within C code,
 * possibly a C library.
 * In practice this will actually do something more useful that this function does!
 *
 * @param id The transaction ID.
 * @return A struct cTransaction corresponding to the transaction ID.
 */
static struct cTransaction get_transaction(long id) {
    struct cTransaction ret = {id, "Some reference.", 42.76};
    return ret;
}

PyDoc_STRVAR(
        cTransaction_docstring,
        "Example of a named tuple type representing a transaction created in C."
        " The type not registered with the cStructSequence module."
);

static PyStructSequence_Field cTransaction_fields[] = {
        {"id",        "The transaction id."},
        {"reference", "The transaction reference."},
        {"amount",    "The transaction amount."},
        {NULL, NULL}
};

static PyStructSequence_Desc cTransaction_desc = {
        "cStructSequence.cTransaction",
        cTransaction_docstring,
        cTransaction_fields,
        3,
};

/* Type initialised dynamically by get__cTransactionType(). */
static PyTypeObject *static_cTransactionType = NULL;

static PyTypeObject *get_cTransactionType(void) {
    if (static_cTransactionType == NULL) {
        static_cTransactionType = PyStructSequence_NewType(&cTransaction_desc);
        if (static_cTransactionType == NULL) {
            PyErr_SetString(
                    PyExc_MemoryError,
                    "Can not initialise a cTransaction type with PyStructSequence_NewType()"
            );
            return NULL;
        }
    }
    return static_cTransactionType;
}

/* A function that creates a cStructSequence.NTUnRegistered dynamically. */
PyObject *
cTransaction_get(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    assert(!PyErr_Occurred());
    static char *kwlist[] = {"id", NULL};
    long id = 0l;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "l", kwlist, &id)) {
        return NULL;
    }
    PyObject *result = PyStructSequence_New(get_cTransactionType());
    if (!result) {
        assert(PyErr_Occurred());
        return NULL;
    }

    struct cTransaction transaction = get_transaction(id);
    PyStructSequence_SetItem(result, 0, PyLong_FromLong(transaction.id));
    PyStructSequence_SetItem(result, 1, PyUnicode_FromString(transaction.reference));
    PyStructSequence_SetItem(result, 2, PyFloat_FromDouble(transaction.amount));
    return result;
}

#pragma mark - A registered Named Tuple with excess fields

PyDoc_STRVAR(
        ExcessNT_docstring,
        "A basic named tuple type with excess fields."
);

static PyStructSequence_Field ExcessNT_fields[] = {
        {"field_one",   "The first field of the named tuple."},
        {"field_two",   "The second field of the named tuple."},
        {"field_three", "The third field of the named tuple, not available to Python."},
        {NULL, NULL}
};

static PyStructSequence_Desc ExcessNT_desc = {
        "cStructSequence.ExcessNT",
        ExcessNT_docstring,
        ExcessNT_fields,
        2, /* Of three fields only two are available to Python. */
};

static PyObject *
ExcessNT_create(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    assert(!PyErr_Occurred());
    static char *kwlist[] = {"field_one", "field_two", "field_three", NULL};
    PyObject *field_one = NULL;
    PyObject *field_two = NULL;
    PyObject *field_three = NULL;
    /* Type initialised dynamically by get__cTransactionType(). */
    static PyTypeObject *static_ExcessNT_Type = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO", kwlist, &field_one, &field_two, &field_three)) {
        return NULL;
    }
    /* The three fields are PyObjects. If your design is that those arguments should be specific types
     * then take the opportunity here to test that they are the expected types.
     */
    if (!static_ExcessNT_Type) {
        static_ExcessNT_Type = PyStructSequence_NewType(&ExcessNT_desc);
        if (!static_ExcessNT_Type) {
            PyErr_SetString(
                    PyExc_MemoryError,
                    "Can not initialise a ExcessNT type with PyStructSequence_NewType()"
            );
            return NULL;
        }
    }
    PyObject *result = PyStructSequence_New(static_ExcessNT_Type);
    if (!result) {
        PyErr_SetString(
                PyExc_MemoryError,
                "Can not create a ExcessNT Struct Sequence with PyStructSequence_New()"
        );
        return NULL;
    }
    /* PyArg_ParseTupleAndKeywords with "O" gives a borrowed reference.
     * https://docs.python.org/3/c-api/arg.html#other-objects
     * "A new strong reference to the object is not created (i.e. its reference count is not increased)."
     * So we increment as PyStructSequence_SetItem seals the reference otherwise if the callers arguments
     * go out of scope we will/may get undefined behaviour when accessing the named tuple fields.
     */
    Py_INCREF(field_one);
    Py_INCREF(field_two);
    Py_INCREF(field_three);
    PyStructSequence_SetItem(result, 0, field_one);
    PyStructSequence_SetItem(result, 1, field_two);
    PyStructSequence_SetItem(result, 2, field_three);
    return result;
}

#pragma mark - A registered Named Tuple with an unnamed field

/* Version as a single 4-byte hex number, e.g. 0x010502B2 == 1.5.2b2.
 * Use this for numeric comparisons, e.g. #if PY_VERSION_HEX >= ...
 *
 * This is Python 3.11+ specific code.
 * Earlier versions give this compile time error:
 * E   ImportError: dlopen(...cStructSequence.cpython-310-darwin.so, 0x0002): \
 *  symbol not found in flat namespace '_PyStructSequence_UnnamedField'
 */
#if PY_VERSION_HEX >= 0x030B0000

static PyStructSequence_Field NTWithUnnamedField_fields[] = {
        {"field_one", "The first field of the named tuple."},
        /* Use NULL then replace with PyStructSequence_UnnamedField
         * otherwise get an error "initializer element is not a compile-time constant" */
        {"field_two", "The second field of the named tuple, not available to Python."},
        {NULL,        "Documentation for an unnamed field."},
        {NULL, NULL}
};

PyDoc_STRVAR(
        NTWithUnnamedField_docstring,
        "A basic named tuple type with an unnamed field."
);

static PyStructSequence_Desc NTWithUnnamedField_desc = {
        "cStructSequence.NTWithUnnamedField",
        NTWithUnnamedField_docstring,
        NTWithUnnamedField_fields,
        1, /* Of three fields only one is available to Python by name. */
};

static PyTypeObject *static_NTWithUnnamedField_Type = NULL;

/**
 * Initialises and returns the \c NTWithUnnamedField_Type.
 * @return The initialised type.
 */
static PyTypeObject *get_NTWithUnnamedField_Type(void) {
    if (!static_NTWithUnnamedField_Type) {
        /* Substitute PyStructSequence_UnnamedField for NULL. */
        NTWithUnnamedField_fields[1].name = PyStructSequence_UnnamedField;
        /* Create and initialise the type. */
        static_NTWithUnnamedField_Type = PyStructSequence_NewType(&NTWithUnnamedField_desc);
        if (!static_NTWithUnnamedField_Type) {
            PyErr_SetString(
                    PyExc_RuntimeError,
                    "Can not initialise a NTWithUnnamedField type with PyStructSequence_NewType()"
            );
            return NULL;
        }
    }
    return static_NTWithUnnamedField_Type;
}

static PyObject *
NTWithUnnamedField_create(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    assert(!PyErr_Occurred());
    static char *kwlist[] = {"field_one", "field_two", "field_three", NULL};
    PyObject *field_one = NULL;
    PyObject *field_two = NULL;
    PyObject *field_three = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO", kwlist, &field_one, &field_two, &field_three)) {
        return NULL;
    }
    /* The three fields are PyObjects. If your design is that those arguments should be specific types
     * then take the opportunity here to test that they are the expected types.
     */

    PyObject *result = PyStructSequence_New(get_NTWithUnnamedField_Type());
    if (!result) {
        PyErr_SetString(
                PyExc_RuntimeError,
                "Can not create a NTWithUnnamedField Struct Sequence with PyStructSequence_New()"
        );
        return NULL;
    }
    /* PyArg_ParseTupleAndKeywords with "O" gives a borrowed reference.
     * https://docs.python.org/3/c-api/arg.html#other-objects
     * "A new strong reference to the object is not created (i.e. its reference count is not increased)."
     * So we increment as PyStructSequence_SetItem seals the reference otherwise if the callers arguments
     * go out of scope we will/may get undefined behaviour when accessing the named tuple fields.
     */
    Py_INCREF(field_one);
    Py_INCREF(field_two);
    Py_INCREF(field_three);
    PyStructSequence_SetItem(result, 0, field_one);
    PyStructSequence_SetItem(result, 1, field_two);
    PyStructSequence_SetItem(result, 2, field_three);
    assert(!PyErr_Occurred());
    return result;
}
#endif

#pragma mark - cStructSequence module methods

static PyMethodDef cStructSequence_methods[] = {
        {"BasicNT_create",            (PyCFunction) BasicNT_create,            METH_VARARGS | METH_KEYWORDS,
                        "Create a BasicNT from the given values."},
        {"NTUnRegistered_create",     (PyCFunction) NTUnRegistered_create,     METH_VARARGS | METH_KEYWORDS,
                        "Create a NTUnRegistered from the given values."},
        {"cTransaction_get",          (PyCFunction) cTransaction_get,          METH_VARARGS | METH_KEYWORDS,
                        "Example of getting a transaction."},
        {"ExcessNT_create",           (PyCFunction) ExcessNT_create,           METH_VARARGS | METH_KEYWORDS,
                        "Create a ExcessNT from the given values."},
/* Python 3.11+ specific code.
 * Earlier versions give this compile time error:
 * E   ImportError: dlopen(...cStructSequence.cpython-310-darwin.so, 0x0002): \
 *  symbol not found in flat namespace '_PyStructSequence_UnnamedField'
 */
#if PY_VERSION_HEX >= 0x030B0000
        {"NTWithUnnamedField_create", (PyCFunction) NTWithUnnamedField_create, METH_VARARGS | METH_KEYWORDS,
                        "Create a NTWithUnnamedField from the given values."},
#endif
        {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef cStructSequence_cmodule = {
        PyModuleDef_HEAD_INIT,
        .m_name = "cStructSequence",
        .m_doc = (
                "Example module that works with Struct Sequence (named tuple) objects."
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
    /* Initialise NTRegisteredType */
    PyObject *NTRegisteredType = (PyObject *) PyStructSequence_NewType(&NTRegistered_desc);
    if (NTRegisteredType == NULL) {
        Py_DECREF(m);
        return NULL;
    }
    Py_INCREF(NTRegisteredType);
    PyModule_AddObject(m, "NTRegisteredType", NTRegisteredType);

    return m;
}

