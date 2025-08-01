//
// Created by Paul Ross on 01/08/2025.
//


#define PPY_SSIZE_T_CLEAN

#include "Python.h"

#include "DebugStructSeq.h"
/* For access to new_unique_string().*/
#include "pyextpatt_util.h"

#ifndef __FILE_NAME__
#define __FILE_NAME__ "DebugStructSeq.c"
#endif

#pragma mark - Struct Sequence

static PyStructSequence_Field struct_sequence_simple_type_fields[] = {
        {"family_name", "Family name."},
        {"given_name", "Given name."},
        {NULL, NULL}
};

static PyStructSequence_Desc struct_sequence_simple_type_desc = {
        "module.struct_sequence_simple",
        ".",
        struct_sequence_simple_type_fields,
        2,
};

static PyTypeObject *static_struct_sequence_simple_type = NULL;

void dbg_PyStructSequence_simple_ctor(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    if (static_struct_sequence_simple_type == NULL) {
        static_struct_sequence_simple_type = PyStructSequence_NewType(&struct_sequence_simple_type_desc);
    }
    assert(static_struct_sequence_simple_type != NULL);
    /* Hmm the refcount is 8. */
//    ref_count = Py_REFCNT(example_type);
//    assert(ref_count == 1);

    PyObject *instance = PyStructSequence_New(static_struct_sequence_simple_type);

    ref_count = Py_REFCNT(instance);
    assert(ref_count == 1);

    /* Get an unset item. */
    PyObject *get_item = NULL;
    get_item = PyStructSequence_GetItem(instance, 0);
    assert(get_item == NULL);

    /* Now set items. */
    PyObject *set_item = NULL;
    set_item = new_unique_string(__FUNCTION__, "NAME");
    PyStructSequence_SetItem(instance, 0, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);
    set_item = new_unique_string(__FUNCTION__, "GENDER");
    PyStructSequence_SetItem(instance, 1, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);

    /* Get items. */
    get_item = PyStructSequence_GetItem(instance, 0);
    assert(get_item != NULL);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);
    get_item = PyStructSequence_GetItem(instance, 1);
    assert(get_item != NULL);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    /* Clean up. */
    Py_DECREF(instance);
    Py_DECREF(static_struct_sequence_simple_type);
}

void dbg_PyStructSequence_setitem_abandons(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    if (static_struct_sequence_simple_type == NULL) {
        static_struct_sequence_simple_type = PyStructSequence_NewType(&struct_sequence_simple_type_desc);
    }
    assert(static_struct_sequence_simple_type != NULL);
    /* Hmm the ref count is 7. */
//    ref_count = Py_REFCNT(example_type);
//    assert(ref_count == 1);

    PyObject *instance = PyStructSequence_New(static_struct_sequence_simple_type);

    ref_count = Py_REFCNT(instance);
    assert(ref_count == 1);

    /* Now set items. */
    PyObject *set_item = NULL;
    set_item = new_unique_string(__FUNCTION__, "NAME");
    PyStructSequence_SetItem(instance, 0, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);
    /* Set it again. */
    PyStructSequence_SetItem(instance, 0, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);

    /* Clean up. */
    Py_DECREF(instance);
    Py_DECREF(static_struct_sequence_simple_type);
}

PyDoc_STRVAR(
        struct_sequence_n_in_sequence_too_large_docstring,
"This uses struct_sequence_simple_type_fields but n_in_sequence is 3 rather than 2."
);

/*
 * This uses struct_sequence_simple_type_fields but n_in_sequence is 3 rather than 2.
 */
static PyStructSequence_Desc struct_sequence_n_in_sequence_too_large_type_desc = {
        "module.struct_sequence_n_in_sequence_too_large",
        struct_sequence_n_in_sequence_too_large_docstring,
        struct_sequence_simple_type_fields,
        3,
};

void dbg_PyStructSequence_n_in_sequence_too_large(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print(); /* Clears error. */
        return;
    }
    assert(!PyErr_Occurred());
//    Py_ssize_t ref_count;
    static PyTypeObject *example_type = NULL;

    if (example_type == NULL) {
        example_type = PyStructSequence_NewType(&struct_sequence_n_in_sequence_too_large_type_desc);
    }
    assert(example_type == NULL);
    assert(PyErr_Occurred());
    /* TypeError: tp_basicsize for type 'module.struct_sequence_n_in_sequence_too_large' (16) is too small for base 'tuple' (24). */
    fprintf(stderr, "%s(): On exit PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
    PyErr_Print(); /* Clears error. */
}


void dbg_PyStructSequence_with_unnamed_field(void) {
    printf("%s():\n", __FUNCTION__);
    if (PyErr_Occurred()) {
        fprintf(stderr, "%s(): On entry PyErr_Print() %s#%d:\n", __FUNCTION__, __FILE_NAME__, __LINE__);
        PyErr_Print();
        return;
    }
    assert(!PyErr_Occurred());
    Py_ssize_t ref_count;

    PyStructSequence_Field struct_sequence_with_unnamed_fields[] = {
            {"family_name", "Family name."},
            /* Use NULL then replace with PyStructSequence_UnnamedField
             * otherwise get an error "initializer element is not a compile-time constant" */
            {"given_name", "Given name."},
            {PyStructSequence_UnnamedField, "Documentation for an unnamed field."},
            {NULL, NULL}
    };
//    struct_sequence_with_unnamed_fields[2].name = PyStructSequence_UnnamedField;

    PyStructSequence_Desc struct_sequence_with_unnamed_field_type_desc = {
            "module.struct_sequence_simple_with_unnamed_field",
            "Documentation.",
            struct_sequence_with_unnamed_fields,
            2,
    };

    PyTypeObject *example_type = NULL;
    if (example_type == NULL) {
        example_type = PyStructSequence_NewType(&struct_sequence_with_unnamed_field_type_desc);
    }
    assert(example_type != NULL);
    /* Hmm. Refcount is 8. */
//    ref_count = Py_REFCNT(example_type);
//    assert(ref_count == 1);

    PyObject *instance = PyStructSequence_New(example_type);

    ref_count = Py_REFCNT(instance);
    assert(ref_count == 1);

    /* Get an unset item. */
    PyObject *get_item = NULL;
    get_item = PyStructSequence_GetItem(instance, 0);
    assert(get_item == NULL);

    /* Now set items. */
    PyObject *set_item = NULL;
    set_item = new_unique_string(__FUNCTION__, "NAME");
    PyStructSequence_SetItem(instance, 0, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);
    set_item = new_unique_string(__FUNCTION__, "GENDER");
    PyStructSequence_SetItem(instance, 1, set_item);
    ref_count = Py_REFCNT(set_item);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());
    fprintf(stdout, "Calling PyObject_Print(instance, stdout, 0);\n");
    PyObject_Print(instance, stdout, 0);
    fprintf(stdout, "\n");
//    if (PyErr_Occurred()) {
//        PyErr_Print();
//    }
    assert(!PyErr_Occurred());
    fprintf(stdout, "Calling PyObject_Print(instance, stdout, Py_PRINT_RAW);\n");
    PyObject_Print(instance, stdout, Py_PRINT_RAW);
    printf("\n");
//    if (PyErr_Occurred()) {
//        PyErr_Print();
//    }
    assert(!PyErr_Occurred());
    fprintf(stdout, "Calling PyObject_Print DONE\n");

    /* Get items. */
    get_item = PyStructSequence_GetItem(instance, 0);
    assert(get_item != NULL);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);
    get_item = PyStructSequence_GetItem(instance, 1);
    assert(get_item != NULL);
    ref_count = Py_REFCNT(get_item);
    assert(ref_count == 1);

    assert(!PyErr_Occurred());

    /* Clean up. */
    Py_DECREF(instance);
    Py_DECREF(example_type);
}
