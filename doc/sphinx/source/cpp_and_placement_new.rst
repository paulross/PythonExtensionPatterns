.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

.. _cpp_and_placement_new:

==========================================
A CPython Extension containing C++ Objects
==========================================

Here is an example of using C++ classes within a CPython extension type (Python object).
It shows the various stages of construction and destruction.

Only the important code is shown here. The complete code is in ``src/cpy/cpp/placement_new.cpp`` and the tests are
in ``tests/unit/test_c_cpp.py``

-----------------------------------------------
Allocation of C++ Objects and Placement new
-----------------------------------------------

In ``src/cpy/cpp/placement_new.cpp`` there is a C++ class ``Verbose`` which:

- Reports on ``stdout`` construction and destruction events.
- Allocates an in-memory buffer of 256MB so that the memory usage, and any leaks, will show up in the process RSS.

We are going to create a Python extension that has a Python class that contains the C++ ``Verbose`` objects in two
ways:

- Directly.
- With a dynamically allocated pointer.

This will illustrate the different techniques needed for both.
Here is the CPython structure:

.. code-block:: cpp

    typedef struct {
        PyObject_HEAD
        Verbose Attr;
        Verbose *pAttr;
    } CppCtorDtorInPyObject;

Here is the function to allocate a new CPython object.
The important point here is that the line below:

.. code-block:: cpp

    self = (CppCtorDtorInPyObject *) type->tp_alloc(type, 0);

Allocates sufficient, uninitialised, space for the ``CppCtorDtorInPyObject`` object.
This will mean that both the ``Verbose Attr;`` and ``Verbose *pAttr;`` are uninitialised.
To initialise them two different techniques must be used:

- For ``Verbose Attr;`` this must be initialised with *placement new* :index:`placement new`: ``new(&self->Attr) Verbose;``.
- For ``Verbose *pAttr;`` this must be initialised with a dynamic new: ``self->pAttr = new Verbose("pAttr");``.

Here is the complete code:

.. code-block:: cpp

    static PyObject *
    CppCtorDtorInPyObject_new(PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)) {
        printf("-- %s()\n", __FUNCTION__);
        CppCtorDtorInPyObject *self;
        self = (CppCtorDtorInPyObject *) type->tp_alloc(type, 0);
        if (self != NULL) {
            // Placement new used for direct allocation.
            new(&self->Attr) Verbose;
            self->Attr.print("Initial self->Attr");
            // Dynamically allocated new.
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

The complimentary de-allocation function uses different deletion techniques for the two objects.

.. code-block:: cpp

    static void
    CppCtorDtorInPyObject_dealloc(CppCtorDtorInPyObject *self) {
        printf("-- %s()\n", __FUNCTION__);
        self->Attr.print("self->Attr before delete");
        // For self->Attr call the destructor directly.
        self->Attr.~Verbose();
        self->pAttr->print("self->pAttr before delete");
        // For self->pAttr use delete.
        delete self->pAttr;
        Py_TYPE(self)->tp_free((PyObject *) self);
    }

The C++ ``Verbose`` class writes to ``stdout`` the stages of construction and deletion, typically the output is:

.. code-block:: bash

    RSS start: 35,586,048
    -- CppCtorDtorInPyObject_new()
    Constructor at 0x102afa840 with argument "Default" buffer len: 268435456
    Default constructor at 0x102afa840 with argument "Default"
    Initial self->Attr: Verbose object at 0x102afa840 m_str: "Default"
    Constructor at 0x600003158000 with argument "pAttr" buffer len: 268435456
    Initial self->pAttr: Verbose object at 0x600003158000 m_str: "pAttr"
    -- CppCtorDtorInPyObject_buffer_size()
    Buffer size: 536,871,116
      RSS new: 572,506,112 +536,920,064
    -- CppCtorDtorInPyObject_dealloc()
    self->Attr before delete: Verbose object at 0x102afa840 m_str: "Default"
    Destructor at 0x102afa840 m_str: "Default"
    self->pAttr before delete: Verbose object at 0x600003158000 m_str: "pAttr"
    Destructor at 0x600003158000 m_str: "pAttr"
      RSS del: 35,602,432 +16,384
      RSS end: 35,602,432 +16,384
