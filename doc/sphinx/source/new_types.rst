.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

.. index::
    single: New Types; Creating

====================================
Creating New Types
====================================

The creation of new extension types (AKA 'classes') is pretty well described in the Python documentation
`tutorial <https://docs.python.org/extending/newtypes_tutorial.html>`_ and
`reference <https://docs.python.org/extending/newtypes.html>`_.
This section is a cookbook of tricks and examples.

------------------------------------
Properties
------------------------------------

.. index::
    single: New Types; Existing Python Properties
    single: New Types; Existing C Properties

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Referencing Existing Properties
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Created Properties
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

---------------
Subclassing
---------------

This large subject gets it own chapter: :ref:`chapter_subclassing_and_using_super`.


.. index::
    single: New Types; Examples

---------------
Examples
---------------

See ``src/cpy/cObject.c`` for some examples, the tests for these are in ``tests/unit/test_c_object.py``:

- ``Null`` is a basic class that does nothing.
- ``Str`` is a subclass of the builtin ``str`` class.


.. index::
    single: New Types; Set Attributes Dynamically
    single: New Types; Get Attributes Dynamically
    single: New Types; Delete Attributes Dynamically

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Setting, Getting and Deleting Attributes Dynamically
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

---------------
Sequence Types
---------------

.. todo::

    "Creating New Types": Add a section on making an object act like a sequence using
    `tp_as_sequence <https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_as_sequence>`_.
    See also `Sequence Object Structures <https://docs.python.org/3/c-api/typeobj.html#sequence-structs>`_


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

TOOD:

---------------
TODOs:
---------------

.. todo::

    "Creating New Types": Add a section on making an object act like a number using
    `tp_as_number <https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_as_number>`_.
    See also `Number Object Structures <https://docs.python.org/3/c-api/typeobj.html#number-structs>`_

.. todo::

    "Creating New Types": Add a section on making an object act like a mapping object (like a ``dict``) using
    `tp_as_mapping <https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_as_mapping>`_.
    See also `Mapping Object Structures <https://docs.python.org/3/c-api/typeobj.html#mapping-structs>`_
