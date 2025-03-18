.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

.. index::
    single: New Types; Creating

************************************
Creating New Types
************************************

The creation of new extension types (AKA 'classes') is pretty well described in the Python documentation
`tutorial <https://docs.python.org/extending/newtypes_tutorial.html>`_ and
`reference <https://docs.python.org/extending/newtypes.html>`_.
This section is a cookbook of tricks and examples.

====================================
Properties
====================================

.. index::
    single: New Types; Existing Python Properties
    single: New Types; Existing C Properties

------------------------------------
Referencing Existing Properties
------------------------------------

If the property is part of the extension type then it is fairly easy to make it directly accessible as 
`described here <https://docs.python.org/extending/newtypes.html#adding-data-and-methods-to-the-basic-example>`_

.. note:: Terminology

    In this section "property", "attribute" and "field" are used interchangeably.

For example the ``Noddy`` struct has a Python object (a Python string) and a C object (an C int):

.. code-block:: c

    typedef struct {
        PyObject_HEAD
        PyObject *first; /* first name */
        /* ... */
        int number;
    } Noddy;

These can be exposed by identifying them as members with an array of ``PyMemberDef`` like this:

.. code-block:: c

    static PyMemberDef Noddy_members[] = {
        {"first", T_OBJECT_EX, offsetof(Noddy, first), 0,
         "first name"},
        /* ... */
        {"number", T_INT, offsetof(Noddy, number), 0,
         "noddy number"},
        {NULL}  /* Sentinel */
    };

And the type struct must reference this array of ``PyMemberDef`` thus:

.. code-block:: c

    static PyTypeObject NoddyType = {
        /* ... */
        Noddy_members,             /* tp_members */
        /* ... */
    };

`Reference to PyMemberdef. <https://docs.python.org/3/c-api/structures.html#c.PyMemberDef>`_

.. index::
    single: New Types; Dynamic Python Properties
    single: New Types; Created Python Properties

--------------------------
Created Properties
--------------------------

If the properties are not directly accessible, for example they might need to be created, then an array of ``PyGetSetDef`` structures is used in the `PyTypeObject.tp_getset <https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_getset>`_ slot.


.. code-block:: c

    static PyObject*
    Foo_property_getter(Foo* self, void * /* closure */) {
        return /* ... */;
    }

    int
    Foo_property_setter(Foo* self, PyObject *value) {
        return /* 0 on success, -1 on failure with error set. */;
    }

    static PyGetSetDef Foo_properties[] = {
        {"id", (getter) Foo_property_getter, (setter) Foo_property_setter,
            "The property documentation.", NULL },
        {NULL}  /* Sentinel */
    };

And the type struct must reference this array of ``PyMemberDef`` thus:

.. code-block:: c

    static PyTypeObject FooType = {
        /* ... */
        Foo_properties,             /* tp_getset */
        /* ... */
    };


`Reference to PyGetSetDef. <https://docs.python.org/3/c-api/structures.html#c.PyGetSetDef>`_

====================================
Subclassing
====================================

This large subject gets it own chapter: :ref:`chapter_subclassing_and_using_super`.


.. index::
    single: New Types; Examples

====================================
Examples
====================================

See ``src/cpy/cObject.c`` for some examples, the tests for these are in ``tests/unit/test_c_object.py``:

- ``Null`` is a basic class that does nothing.
- ``Str`` is a subclass of the builtin ``str`` class.


.. index::
    single: New Types; Set Attributes Dynamically
    single: New Types; Get Attributes Dynamically
    single: New Types; Delete Attributes Dynamically

----------------------------------------------------
Setting, Getting and Deleting Attributes Dynamically
----------------------------------------------------

In ``src/cpy/cObject.c`` there is an example, ``ObjectWithAttributes``, which is a class that can set, get, delete
attributes dynamically.

Firstly the object declaration:

.. code-block:: c

    typedef struct {
        PyObject_HEAD
        /* Attributes dictionary, NULL on construction,
         8 will be populated by MyObj_getattro. */
        PyObject *x_attr;
    } ObjectWithAttributes;

Then some type checking that requires a forward declaration:

.. code-block:: c

    /** Forward declaration. */
    static PyTypeObject ObjectWithAttributes_Type;
    #define ObjectWithAttributes_Check(v) (Py_TYPE(v) == &ObjectWithAttributes_Type)

The ``__new__`` and ``__del__`` methods:

.. code-block:: c

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

Add an empty method for demonstration:

.. code-block:: c

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

Now the methods to get and set attribute:

.. code-block:: c

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
            /* v is a borrowed reference,
             * then PyDict_SetItemString() does NOT steal it
             * so nothing to do. */
            return PyDict_SetItemString(self->x_attr, name, v);
    }

Finally the type declaration. Note this is the complete type declaration with compiler declarations to suit
Python versions 3.6 to 3.13:

.. code-block:: c

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

Finally add this to the module (partial code):

.. code-block:: c

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
        if (PyModule_AddObject(m, "ObjectWithAttributes",
                               (PyObject *) &ObjectWithAttributes_Type)) {
            goto fail;
        }
        /* More here ... */
        return m;
        fail:
        Py_XDECREF(m);
        return NULL;
    }

This can be tested thus, in ``tests/unit/test_c_object.py``:

.. code-block:: python

    import pytest

    from cPyExtPatt import cObject

    def test_ObjectWithAttributes_set_and_get():
        obj = cObject.ObjectWithAttributes()
        obj.some_attr = 'Some attribute'
        assert hasattr(obj, 'some_attr')
        assert obj.some_attr == 'Some attribute'


    def test_ObjectWithAttributes_set_and_del():
        obj = cObject.ObjectWithAttributes()
        obj.some_attr = 'Some attribute'
        assert hasattr(obj, 'some_attr')
        delattr(obj, 'some_attr')
        assert not hasattr(obj, 'some_attr')
        with pytest.raises(AttributeError) as err:
            obj.some_attr
        assert err.value.args[0] == "'cObject.ObjectWithAttributes' object has no attribute 'some_attr'"


.. _PySequence_Check(): https://docs.python.org/3/c-api/sequence.html#c.PySequence_Check
.. _PySequence_GetItem(): https://docs.python.org/3/c-api/sequence.html#c.PySequence_GetItem
.. _PySequence_SetItem(): https://docs.python.org/3/c-api/sequence.html#c.PySequence_SetItem
.. _PySequence_Contains(): https://docs.python.org/3/c-api/sequence.html#c.PySequence_Contains
.. _PySequence_Concat(): https://docs.python.org/3/c-api/sequence.html#c.PySequence_Concat
.. _PySequence_InPlaceConcat(): https://docs.python.org/3/c-api/sequence.html#c.PySequence_InPlaceConcat
.. _PySequence_Repeat(): https://docs.python.org/3/c-api/sequence.html#c.PySequence_Repeat
.. _PySequence_InPlaceRepeat(): https://docs.python.org/3/c-api/sequence.html#c.PySequence_InPlaceRepeat

.. _PyObject_SetItem(): https://docs.python.org/3/c-api/object.html#c.PyObject_SetItem
.. _PyObject_DelItem(): https://docs.python.org/3/c-api/object.html#c.PyObject_DelItem

.. _sq_length: https://docs.python.org/3/c-api/typeobj.html#c.PySequenceMethods.sq_length
.. _sq_concat: https://docs.python.org/3/c-api/typeobj.html#c.PySequenceMethods.sq_concat
.. _sq_repeat: https://docs.python.org/3/c-api/typeobj.html#c.PySequenceMethods.sq_repeat
.. _sq_item: https://docs.python.org/3/c-api/typeobj.html#c.PySequenceMethods.sq_item
.. _sq_ass_item: https://docs.python.org/3/c-api/typeobj.html#c.PySequenceMethods.sq_ass_item
.. _sq_ass_contains: https://docs.python.org/3/c-api/typeobj.html#c.PySequenceMethods.sq_ass_contains
.. _sq_inplace_concat: https://docs.python.org/3/c-api/typeobj.html#c.PySequenceMethods.sq_inplace_concat
.. _sq_inplace_repeat: https://docs.python.org/3/c-api/typeobj.html#c.PySequenceMethods.sq_inplace_repeat

.. _lenfunc: https://docs.python.org/3/c-api/typeobj.html#c.lenfunc
.. _binaryfunc: https://docs.python.org/3/c-api/typeobj.html#c.binaryfunc
.. _ssizeargfunc: https://docs.python.org/3/c-api/typeobj.html#c.ssizeargfunc
.. _ssizeobjargproc: https://docs.python.org/3/c-api/typeobj.html#c.ssizeobjargproc
.. _objobjproc: https://docs.python.org/3/c-api/typeobj.html#c.objobjproc

=========================
Emulating Sequence Types
=========================

This section describes how to make an object act like a sequence using
`tp_as_sequence <https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_as_sequence>`_.
See also `Sequence Object Structures <https://docs.python.org/3/c-api/typeobj.html#sequence-structs>`_

As an example here is an extension that can represent a sequence of longs in C with a CPython sequence interface.
The code is in ``src/cpy/Object/cSeqObject.c``.

First the class declaration:

.. code-block:: c

    #define PY_SSIZE_T_CLEAN
    #include <Python.h>
    #include "structmember.h"

    typedef struct {
        PyObject_HEAD
        long *array_long;
        ssize_t size;
    } SequenceLongObject;


Then the equivalent of ``__new__``:

.. code-block:: c

    static PyObject *
    SequenceLongObject_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
        SequenceLongObject *self;
        self = (SequenceLongObject *) type->tp_alloc(type, 0);
        if (self != NULL) {
            assert(!PyErr_Occurred());
            self->size = 0;
            self->array_long = NULL;
        }
        return (PyObject *) self;
    }

And the equivalent of ``__init__`` that takes a Python sequence of ints:

.. code-block:: c

    static int
    SequenceLongObject_init(SequenceLongObject *self, PyObject *args, PyObject *kwds) {
        static char *kwlist[] = {"sequence", NULL};
        PyObject *sequence = NULL;

        if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &sequence)) {
            return -1;
        }
        if (!PySequence_Check(sequence)) {
            return -2;
        }
        self->size = PySequence_Length(sequence);
        self->array_long = malloc(self->size * sizeof(long));
        if (!self->array_long) {
            return -3;
        }
        for (Py_ssize_t i = 0; i < self->size; ++i) {
            // New reference.
            PyObject *py_value = PySequence_GetItem(sequence, i);
            if (PyLong_Check(py_value)) {
                self->array_long[i] = PyLong_AsLong(py_value);
                Py_DECREF(py_value);
            } else {
                PyErr_Format(
                        PyExc_TypeError,
                        "Argument [%zd] must be a int, not type %s",
                        i,
                        Py_TYPE(sequence)->tp_name
                );
                // Clean up on error.
                free(self->array_long);
                self->array_long = NULL;
                Py_DECREF(py_value);
                return -4;
            }
        }
        return 0;
    }

And de-allocation:

.. code-block:: c

    static void
    SequenceLongObject_dealloc(SequenceLongObject *self) {
        free(self->array_long);
        Py_TYPE(self)->tp_free((PyObject *) self);
    }

---------------------------
The Sequence Function Table
---------------------------



.. list-table:: Sequence Methods
   :widths: 30 25 50 70
   :header-rows: 1

   * - Member
     - Function Type
     - Function Signature
     - Description
   * - `sq_length`_
     - `lenfunc`_
     - ``Py_ssize_t (*lenfunc)(PyObject*)``
     - Returns the length of the sequence.
   * - `sq_concat`_
     - `binaryfunc`_
     - ``PyObject *(*binaryfunc)(PyObject*, PyObject*)``
     - Takes two sequences and returns a new third one with the first and second concatenated.
   * - `sq_repeat`_
     - `ssizeargfunc`_
     - ``PyObject *(*ssizeargfunc)(PyObject*, Py_ssize_t)``
     - Returns a new sequence with the old one repeated n times.
   * - `sq_item`_
     - `ssizeargfunc`_
     - ``PyObject *(*ssizeargfunc)(PyObject*, Py_ssize_t)``
     - Returns a *new* reference to the n'th item in the sequence.
       Negative indexes are handled appropriately.
       Used by `PySequence_GetItem()`_.
       This is a fairly crucial implementation for a sequence as `PySequence_Check()`_ detects this to decide if the
       object is a sequence.
   * - `sq_ass_item`_
     - `ssizeobjargproc`_
     - ``int (*ssizeobjargproc)(PyObject*, Py_ssize_t, PyObject*)``
     - Sets the the n'th item in the sequence.
       If the value is NULL the item is deleted and the sequence concatenated (thus called by `PyObject_DelItem()`_).
       Negative indexes are handled appropriately.
       Used by `PyObject_SetItem()`_.
   * - `sq_ass_contains`_
     - `objobjproc`_
     - ``int (*objobjproc)(PyObject*, PyObject*)``
     - Returns non-zero if the sequence contains the given object.
       Used by `PySequence_Contains()`_.
       This slot may be left to NULL, in this case PySequence_Contains() simply traverses the sequence until it finds a match.
   * - `sq_inplace_concat`_
     - `binaryfunc`_
     - ``PyObject *(*binaryfunc)(PyObject*, PyObject*)``
     - Provides in-place concatenation, for example ``+=``.
       If this slot is ``NULL`` then `PySequence_Concat()`_ will be used returning a new object.
   * - `sq_inplace_repeat`_
     - `ssizeargfunc`_
     - ``PyObject *(*ssizeargfunc)(PyObject*, Py_ssize_t)``
     - Provides in-place concatenation, for example ``+=``.
       Returns the existing sequence repeated n times.
       If this slot is ``NULL`` then `PySequence_Repeat()`_ will be used returning a new object.

---------------
``sq_length``
---------------

.. list-table:: Sequence Methods: ``sq_length``
   :widths: 20 80
   :header-rows: 0

   * - Member
     - `sq_length`_
   * - Function type
     - `lenfunc`_
   * - Function signature
     - ``Py_ssize_t (*lenfunc)(PyObject*)``
   * - Description
     - Returns the length of the sequence.

Implementation
--------------

In ``src/cpy/Object/cSeqObject.c``:

.. code-block:: c

    static Py_ssize_t
    SequenceLongObject_sq_length(PyObject *self) {
        return ((SequenceLongObject *) self)->size;
    }

Tests
--------------

Tests are in ``tests/unit/test_c_seqobject.py``:

.. code-block:: python

    from cPyExtPatt import cSeqObject

    def test_SequenceLongObject_len():
        obj = cSeqObject.SequenceLongObject([7, 4, 1, ])
        assert len(obj) == 3

---------------
``sq_concat``
---------------

.. list-table:: Sequence Methods: ``sq_concat``
   :widths: 20 80
   :header-rows: 0

   * - Member
     - `sq_concat`_
   * - Function type
     - `binaryfunc`_
   * - Function signature
     - ``PyObject *(*binaryfunc)(PyObject*, PyObject*)``
   * - Description
     - Takes two sequences and returns a new third one with the first and second concatenated.
       This is used by the ``+`` Python operator and the `PySequence_Concat()`_ C API.

Implementation
--------------

Note the use of forward references for the type checker as we need to check of the second argument is of the
correct type.
In ``src/cpy/Object/cSeqObject.c``:

.. code-block:: c

    // Forward references
    static PyTypeObject SequenceLongObjectType;
    static int is_sequence_of_long_type(PyObject *op);

    /** Returns a new SequenceLongObject composed of self + other. */
    static PyObject *
    SequenceLongObject_sq_concat(PyObject *self, PyObject *other) {
        if (!is_sequence_of_long_type(other)) {
            PyErr_Format(
                    PyExc_TypeError,
                    "%s(): argument 1 must have type \"SequenceLongObject\" not %s",
                    Py_TYPE(other)->tp_name
            );
            return NULL;
        }
        PyObject *ret = SequenceLongObject_new(&SequenceLongObjectType, NULL, NULL);
        if (!ret) {
            assert(PyErr_Occurred());
            return NULL;
        }
        /* For convenience. */
        SequenceLongObject *ret_as_slo = (SequenceLongObject *) ret;
        ret_as_slo->size = ((SequenceLongObject *) self)->size + ((SequenceLongObject *) other)->size;
        ret_as_slo->array_long = malloc(ret_as_slo->size * sizeof(long));
        if (!ret_as_slo->array_long) {
            PyErr_Format(PyExc_MemoryError, "%s(): Can not create new object.", __FUNCTION__);
            Py_DECREF(ret);
            return NULL;
        }

        ssize_t i = 0;
        ssize_t ub = ((SequenceLongObject *) self)->size;
        while (i < ub) {
            ret_as_slo->array_long[i] = ((SequenceLongObject *) self)->array_long[i];
            i++;
        }
        ssize_t j = 0;
        ub = ((SequenceLongObject *) other)->size;
        while (j < ub) {
            ret_as_slo->array_long[i] = ((SequenceLongObject *) other)->array_long[j];
            i++;
            j++;
        }
        return ret;
    }

Tests
--------------

Tests are in ``tests/unit/test_c_seqobject.py``:

.. code-block:: python

    def test_SequenceLongObject_concat():
        obj_a = cSeqObject.SequenceLongObject([7, 4, 1, ])
        obj_b = cSeqObject.SequenceLongObject([70, 40, 100, ])
        assert id(obj_a) != id(obj_b)
        obj = obj_a + obj_b
        assert id(obj) != id(obj_a)
        assert id(obj) != id(obj_b)
        assert len(obj) == 6
        assert list(obj) == [7, 4, 1, ] + [70, 40, 100, ]

---------------
``sq_repeat``
---------------

.. list-table:: Sequence Methods: ``sq_concat``
   :widths: 20 80
   :header-rows: 0

   * - Member
     - `sq_repeat`_
   * - Function type
     - `ssizeargfunc`_
   * - Function signature
     - ``PyObject *(*ssizeargfunc)(PyObject*, Py_ssize_t)``
   * - Description
     - Returns a new sequence with the old one repeated the given number of times times.
       This is used by the ``*`` Python operator and the `PySequence_Repeat()`_ C API.

Implementation
--------------

The implementation is fairly straightforward in ``src/cpy/Object/cSeqObject.c``.
Note that ``count`` can be zero or negative:

.. code-block:: c

    /** Return a new sequence which contains the old one repeated count times. */
    static PyObject *
    SequenceLongObject_sq_repeat(PyObject *self, Py_ssize_t count) {
        PyObject *ret = SequenceLongObject_new(&SequenceLongObjectType, NULL, NULL);
        if (!ret) {
            assert(PyErr_Occurred());
            return NULL;
        }
        assert(ret != self);
        if (((SequenceLongObject *) self)->size > 0 && count > 0) {
            /* For convenience. */
            SequenceLongObject *self_as_slo = (SequenceLongObject *) self;
            SequenceLongObject *ret_as_slo = (SequenceLongObject *) ret;
            ret_as_slo->size = self_as_slo->size * count;
            assert(ret_as_slo->size > 0);
            ret_as_slo->array_long = malloc(ret_as_slo->size * sizeof(long));
            if (!ret_as_slo->array_long) {
                PyErr_Format(PyExc_MemoryError, "%s(): Can not create new object.", __FUNCTION__);
                Py_DECREF(ret);
                return NULL;
            }
            Py_ssize_t ret_index = 0;
            for (Py_ssize_t i = 0; i < count; ++i) {
                for (Py_ssize_t j = 0; j < self_as_slo->size; ++j) {
                    ret_as_slo->array_long[ret_index] = self_as_slo->array_long[j];
                    ++ret_index;
                }
            }
        } else {
            /* Empty sequence. */
        }
        return ret;
    }

Tests
--------------

Tests are in ``tests/unit/test_c_seqobject.py``:

.. code-block:: python

    @pytest.mark.parametrize(
        'initial_sequence, count, expected',
        (
            (
                [], 1, [],
            ),
            (
                [7, 4, 1, ], 0, [],
            ),
            (
                [7, 4, 1, ], -1, [],
            ),
            (
                [7, 4, 1, ], 1, [7, 4, 1, ],
            ),
            (
                [7, 4, 1, ], 2, [7, 4, 1, 7, 4, 1, ],
            ),
            (
                [7, 4, 1, ], 3, [7, 4, 1, 7, 4, 1, 7, 4, 1, ],
            ),
        )
    )
    def test_SequenceLongObject_repeat(initial_sequence, count, expected):
        obj_a = cSeqObject.SequenceLongObject(initial_sequence)
        obj = obj_a * count
        print()
        assert id(obj_a) != id(obj)
        assert list(obj) == expected
        assert list(obj) == (list(obj_a) * count)


---------------
``sq_item``
---------------

`sq_item`_ gives read access to an indexed member.

.. list-table:: Sequence Methods: ``sq_item``
   :widths: 20 80
   :header-rows: 0

   * - Member
     - `sq_item`_
   * - Function type
     - `ssizeargfunc`_
   * - Function signature
     - ``PyObject *(*ssizeargfunc)(PyObject*, Py_ssize_t)``
   * - Description
     - Returns a *new* reference to the n'th item in the sequence.
       Negative indexes are handled appropriately.
       Used by `PySequence_GetItem()`_.
       This is a fairly crucial implementation for a sequence as `PySequence_Check()`_ detects this to decide if the
       object is a sequence.

Implementation
--------------

In ``src/cpy/Object/cSeqObject.c``:

.. code-block:: c

    /** Returns a new reference to an indexed item in a sequence. */
    static PyObject *
    SequenceLongObject_sq_item(PyObject *self, Py_ssize_t index) {
        Py_ssize_t my_index = index;
        if (my_index < 0) {
            my_index += SequenceLongObject_sq_length(self);
        }
        // Corner case example: len(self) == 0 and index < 0
        if (my_index < 0 || my_index >= SequenceLongObject_sq_length(self)) {
            PyErr_Format(
                    PyExc_IndexError,
                    "Index %ld is out of range for length %ld",
                    index,
                    SequenceLongObject_sq_length(self)
            );
            return NULL;
        }
        return PyLong_FromLong(((SequenceLongObject *) self)->array_long[my_index]);
    }

Tests
--------------

Tests are in ``tests/unit/test_c_seqobject.py`` which includes failure modes:

.. code-block:: python

    from cPyExtPatt import cSeqObject

    @pytest.mark.parametrize(
        'initial_sequence, index, expected',
        (
                (
                        [7, 4, 1, ], 0, 7,
                ),
                (
                        [7, 4, 1, ], 1, 4,
                ),
                (
                        [7, 4, 1, ], 2, 1,
                ),
                (
                        [7, 4, 1, ], -1, 1,
                ),
                (
                        [7, 4, 1, ], -2, 4,
                ),
                (
                        [7, 4, 1, ], -3, 7,
                ),
        )
    )
    def test_SequenceLongObject_item(initial_sequence, index, expected):
        obj = cSeqObject.SequenceLongObject(initial_sequence)
        assert obj[index] == expected

    @pytest.mark.parametrize(
        'initial_sequence, index, expected',
        (
                (
                        [], 0, 'Index 0 is out of range for length 0',
                ),
                (
                        [], -1, 'Index -1 is out of range for length 0',
                ),
                (
                        [1, ], 2, 'Index 2 is out of range for length 1',
                ),
        )
    )
    def test_SequenceLongObject_item_raises(initial_sequence, index, expected):
        obj = cSeqObject.SequenceLongObject(initial_sequence)
        with pytest.raises(IndexError) as err:
            obj[index]
        assert err.value.args[0] == expected


---------------
``sq_ass_item``
---------------

`sq_ass_item`_ gives write and delete access to an indexed member.

.. list-table:: Sequence Methods: ``sq_ass_item``
   :widths: 20 80
   :header-rows: 0

   * - Member
     - `sq_ass_item`_
   * - Function type
     - `ssizeobjargproc`_
   * - Function signature
     - ``int (*ssizeobjargproc)(PyObject*, Py_ssize_t, PyObject*)``
   * - Description
     - Sets the the n'th item in the sequence.
       If the value is NULL the item is deleted and the sequence concatenated (thus called by `PyObject_DelItem()`_).
       Negative indexes are handled appropriately.
       Used by `PyObject_SetItem()`_.

Implementation
--------------

In ``src/cpy/Object/cSeqObject.c``:

.. code-block:: c

    static int
    SequenceLongObject_sq_ass_item(PyObject *self, Py_ssize_t index, PyObject *value) {
        fprintf(
            stdout, "%s()#%d: self=%p index=%zd value=%p\n",
            __FUNCTION__, __LINE__, (void *) self, index, (void *) value
        );
        /* This is very weird. */
        if (index < 0) {
            fprintf(
                stdout, "%s()#%d: Fixing index index=%zd to %zd\n", __FUNCTION__, __LINE__,
                index, index - SequenceLongObject_sq_length(self)
            );
            index -= SequenceLongObject_sq_length(self);
        }
        /* Isn't it? */
        Py_ssize_t my_index = index;
        if (my_index < 0) {
            my_index += SequenceLongObject_sq_length(self);
        }
        // Corner case example: len(self) == 0 and index < 0
        fprintf(
            stdout, "%s()#%d: len=%zd index=%zd my_index=%zd\n", __FUNCTION__, __LINE__,
            SequenceLongObject_sq_length(self), index, my_index
        );
        if (my_index < 0 || my_index >= SequenceLongObject_sq_length(self)) {
            PyErr_Format(
                    PyExc_IndexError,
                    "Index %ld is out of range for length %ld",
                    index,
                    SequenceLongObject_sq_length(self)
            );
            return -1;
        }
        if (value != NULL) {
            /* Just set the value. */
            if (!PyLong_Check(value)) {
                PyErr_Format(
                        PyExc_TypeError,
                        "sq_ass_item value needs to be an int, not type %s",
                        Py_TYPE(value)->tp_name
                );
                return -1;
            }
            ((SequenceLongObject *) self)->array_long[my_index] = PyLong_AsLong(value);
        } else {
            /* Delete the value. */
            /* For convenience. */
            SequenceLongObject *self_as_slo = (SequenceLongObject *) self;
            /* Special case: deleting the only item in the array. */
            if (self_as_slo->size == 1) {
                fprintf(stdout, "%s()#%d: deleting empty index\n", __FUNCTION__, __LINE__);
                free(self_as_slo->array_long);
                self_as_slo->array_long = NULL;
                self_as_slo->size = 0;
            } else {
                /* Delete the value and re-compose the array. */
                fprintf(stdout, "%s()#%d: deleting index=%zd\n", __FUNCTION__, __LINE__, index);
                long *new_array = malloc((self_as_slo->size - 1) * sizeof(long));
                if (!new_array) {
                    PyErr_Format(
                            PyExc_MemoryError,
                            "sq_ass_item can not allocate new array. %s#%d",
                            __FILE__, __LINE__
                    );
                    return -1;
                }
                /* Copy up to the index. */
                Py_ssize_t index_new_array = 0;
                for (Py_ssize_t i = 0; i < my_index; ++i, ++index_new_array) {
                    new_array[index_new_array] = self_as_slo->array_long[i];
                }
                /* Copy past the index. */
                for (Py_ssize_t i = my_index + 1; i < self_as_slo->size; ++i, ++index_new_array) {
                    new_array[index_new_array] = self_as_slo->array_long[i];
                }

                free(self_as_slo->array_long);
                self_as_slo->array_long = new_array;
                --self_as_slo->size;
            }
        }
        return 0;
    }

Tests
--------------

Tests are in ``tests/unit/test_c_seqobject.py`` which includes failure modes.
First setting a value:

.. code-block:: python

    from cPyExtPatt import cSeqObject

    @pytest.mark.parametrize(
        'initial_sequence, index, value, expected',
        (
                (
                        [7, 4, 1, ], 0, 14, [14, 4, 1, ],
                ),
                (
                        [7, 4, 1, ], -1, 14, [7, 4, 14, ],
                ),
                (
                        [7, 4, 1, ], -2, 14, [7, 14, 1, ],
                ),
                (
                        [7, 4, 1, ], -3, 14, [14, 4, 1, ],
                ),
        )
    )
    def test_SequenceLongObject_setitem(initial_sequence, index, value, expected):
        obj = cSeqObject.SequenceLongObject(initial_sequence)
        obj[index] = value
        assert list(obj) == expected


Setting a value with an out of range index:

.. code-block:: python

    from cPyExtPatt import cSeqObject

    @pytest.mark.parametrize(
        'initial_sequence, index, expected',
        (
                (
                        [7, 4, 1, ], 3, 'Index 3 is out of range for length 3',
                ),
                (
                        [7, 4, 1, ], -4, 'Index -4 is out of range for length 3',
                ),
        )
    )
    def test_SequenceLongObject_setitem_raises(initial_sequence, index, expected):
        print()
        print(initial_sequence, index, expected)
        obj = cSeqObject.SequenceLongObject(initial_sequence)
        with pytest.raises(IndexError) as err:
            obj[index] = 100
            print(list(obj))
        assert err.value.args[0] == expected


Deleting a value:

.. code-block:: python

    from cPyExtPatt import cSeqObject

    @pytest.mark.parametrize(
        'initial_sequence, index, expected',
        (
                (
                        [7, ], 0, [],
                ),
                (
                        [7, ], -1, [],
                ),
                (
                        [7, 4, 1, ], 1, [7, 1, ],
                ),
                (
                        [7, 4, ], 0, [4, ],
                ),
                (
                        [7, 4, 1, ], -1, [7, 4, ],
                ),
                (
                        [7, 4, 1, ], -2, [7, 1, ],
                ),
                (
                        [7, 4, 1, ], -3, [4, 1, ],
                ),
        )
    )
    def test_SequenceLongObject_delitem(initial_sequence, index, expected):
        obj = cSeqObject.SequenceLongObject(initial_sequence)
        del obj[index]
        assert list(obj) == expected


Deleting a value with an out of range index:

.. code-block:: python

    from cPyExtPatt import cSeqObject

    @pytest.mark.parametrize(
        'initial_sequence, index, expected',
        (
                (
                        [], 0, 'Index 0 is out of range for length 0',
                ),
                (
                        [], -1, 'Index -1 is out of range for length 0',
                ),
                (
                        [7, ], 1, 'Index 1 is out of range for length 1',
                ),
                (
                        [7, ], -3, 'Index -3 is out of range for length 1',
                ),
        )
    )
    def test_SequenceLongObject_delitem_raises(initial_sequence, index, expected):
        print()
        print(initial_sequence, index, expected)
        obj = cSeqObject.SequenceLongObject(initial_sequence)
        print(list(obj))
        with pytest.raises(IndexError) as err:
            del obj[index]
        assert err.value.args[0] == expected





TODO:

====================================
TODOs:
====================================

.. todo::

    "Creating New Types": Add a section on making an object act like a number using
    `tp_as_number <https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_as_number>`_.
    See also `Number Object Structures <https://docs.python.org/3/c-api/typeobj.html#number-structs>`_

.. todo::

    "Creating New Types": Add a section on making an object act like a mapping object (like a ``dict``) using
    `tp_as_mapping <https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_as_mapping>`_.
    See also `Mapping Object Structures <https://docs.python.org/3/c-api/typeobj.html#mapping-structs>`_
