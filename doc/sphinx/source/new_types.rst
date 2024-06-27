.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

====================================
Creating New Types
====================================

The creation of new extension types (AKA 'classes') is pretty well described in the Python documentation `tutorial <https://docs.python.org/extending/newtypes_tutorial.html>`_ and
`reference <https://docs.python.org/extending/newtypes.html>`_. This section just describes a rag bag of tricks and examples.

------------------------------------
Properties
------------------------------------

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Referencing Existing Properties
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the property is part of the extension type then it is fairly easy to make it directly accessible as 
`described here <https://docs.python.org/extending/newtypes.html#adding-data-and-methods-to-the-basic-example>`_ 

For example the ``Noddy`` struct has a Python object (a string) and a C object (an int):

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
Examples
---------------

See ``src/cpy/cObject.c`` for some examples.