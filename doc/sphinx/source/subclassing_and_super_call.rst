.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _chapter_subclassing_and_using_super:

.. index::
    single: Subclassing
    single: Subclassing; Using super()

**************************************
Subclassing and Using ``super()``
**************************************

This chapter describes how to subclass existing types and how to call ``super()`` in C where necessary.

.. index::
    single: Subclassing; Basic

=================================
Basic Subclassing
=================================

In this example we are going to subclass the built in ``list`` object and just add a new attribute ``state``.
The code is based on
`Python documentation on subclassing <https://docs.python.org/3/extending/newtypes_tutorial.html#subclassing-other-types>`_
The full code is in ``src/cpy/SubClass/sublist.c`` and the tests are in ``tests/unit/test_c_subclass.py``.

-----------------------------
Writing the C Extension
-----------------------------

First the declaration of the ``SubListObject``:

.. code-block:: c

    #define PY_SSIZE_T_CLEAN
    #include <Python.h>
    #include "structmember.h"

    typedef struct {
        PyListObject list; // The superclass.
        int state;         // Our new attribute.
    } SubListObject;

The ``__init__`` method:

.. code-block:: c

    static int
    SubList_init(SubListObject *self, PyObject *args, PyObject *kwds) {
        if (PyList_Type.tp_init((PyObject *) self, args, kwds) < 0) {
            return -1;
        }
        self->state = 0;
        return 0;
    }

Now add an ``increment()`` method and the method table:

.. code-block:: c

    static PyObject *
    SubList_increment(SubListObject *self, PyObject *Py_UNUSED(unused)) {
        self->state++;
        return PyLong_FromLong(self->state);
    }

    static PyMethodDef SubList_methods[] = {
            {"increment", (PyCFunction) SubList_increment, METH_NOARGS,
                    PyDoc_STR("increment state counter")},
            {NULL, NULL, 0, NULL},
    };

Add the ``state`` attribute:

.. code-block:: c

    static PyMemberDef SubList_members[] = {
            {"state", T_INT, offsetof(SubListObject, state), 0,
                    "Value of the state."},
            {NULL, 0, 0, 0, NULL}  /* Sentinel */
    };

Declare the type.

Note that we do not initialise ``tp_base`` just yet.
The reason is that C99 requires the initializers to be “address constants”.
This is best described in the
`tp_base documentation <https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_base>`_.

.. code-block:: c

    static PyTypeObject SubListType = {
            PyVarObject_HEAD_INIT(NULL, 0)
            .tp_name = "sublist.SubList",
            .tp_doc = PyDoc_STR("SubList objects"),
            .tp_basicsize = sizeof(SubListObject),
            .tp_itemsize = 0,
            .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
            .tp_init = (initproc) SubList_init,
            .tp_methods = SubList_methods,
            .tp_members = SubList_members,
    };

Declare the module:

.. code-block:: c

    static PyModuleDef sublistmodule = {
            PyModuleDef_HEAD_INIT,
            .m_name = "sublist",
            .m_doc = "Module that contains a subclass of a list.",
            .m_size = -1,
    };

Initialise the module, this is where we set ``tp_base``:

.. code-block:: c

    PyMODINIT_FUNC
    PyInit_sublist(void) {
        PyObject *m;
        SubListType.tp_base = &PyList_Type;
        if (PyType_Ready(&SubListType) < 0) {
            return NULL;
        }
        m = PyModule_Create(&sublistmodule);
        if (m == NULL) {
            return NULL;
        }
        Py_INCREF(&SubListType);
        if (PyModule_AddObject(m, "SubList", (PyObject *) &SubListType) < 0) {
            Py_DECREF(&SubListType);
            Py_DECREF(m);
            return NULL;
        }
        return m;
    }

-----------------------------
Setup and Build
-----------------------------

In the ``setup.py`` add an Extension such as:

.. code-block:: python

    Extension(name=f"cPyExtPatt.SubClass.sublist",
              include_dirs=[
                  '/usr/local/include',
              ],
              sources=[
                  "src/cpy/SubClass/sublist.c",
              ],
              extra_compile_args=extra_compile_args_c,
              language='c',
              ),

The extension can be built with ``python setup.py develop``.

-----------------------------
Test
-----------------------------

The extension can used like this:

.. code-block:: python

    from cPyExtPatt.SubClass import sublist

    obj = sublist.SubList()
    assert obj.state == 0
    obj.increment()
    assert obj.state == 1

This is fine for subclasses that just add some additional functionality however if you want to overload the super class
you need to be able to call ``super()`` from C which is described next.

.. index::
    single: Subclassing; Calling super() from C

=================================
Calling ``super()`` from C
=================================

I needed to call super() from a C extension and I couldn't find a good description of how to do this online so I am
including this here.
The ability to call ``super()`` is needed when you want to modify the behaviour of inherited classes.

Suppose we wanted to subclass a list and record how many times ``append()`` was called.
This is simple enough in pure Python:

.. code-block:: python

    class SubList(list):
        def __init__(self, *args, **kwargs):
            self.appends = 0
            super().__init__(*args, **kwargs)

        def append(self, v):
            self.appends += 1
            return super().append(v)

To do it in C is a bit trickier. Taking as our starting point the
`example of sub-classing a list <https://docs.python.org/3/extending/newtypes.html#subclassing-other-types>`_
in the Python documentation (amended a little bit for our example).

Our type contains an integer count of the number of appends.
That is set to zero on construction and can be accessed like a normal member.

.. code-block:: c

    typedef struct {
        PyListObject list;
        int appends;
    } SubListObject;


    static int
    SubListObject_init(SubListObject *self, PyObject *args, PyObject *kwds)
    {
        if (PyList_Type.tp_init((PyObject *)self, args, kwds) < 0) {
            return -1;
        }
        self->appends = 0;
        return 0;
    }

    static PyMemberDef SubListObject_members[] = {
        ...
        {"appends", T_INT, offsetof(SubListObject, appends), 0,
            "Number of append operations."},
        ...
        {NULL, 0, 0, 0, NULL}  /* Sentinel */
    };

We now need to create the ``append()`` function, which will call the superclass ``append()`` and then increment the
``appends`` counter:

.. code-block:: c

    static PyMethodDef SubListObject_methods[] = {
        ...
        {"append", (PyCFunction)SubListObject_append, METH_VARARGS,
            PyDoc_STR("Append to the list")},
        ...
        {NULL,	NULL, 0, NULL},
    };

This is where it gets tricky, how do we implement ``SubListObject_append``?

--------------------------
The Obvious Way is Wrong
--------------------------

A first attempt might do something like a method call on the ``PyListObject``:
 
.. code-block:: c

    typedef struct {
        PyListObject list;
        int appends;
    } SubListObject;

    /* Other stuff here. */
    
    static PyObject *
    SubListObject_append(SubListObject *self, PyObject *args) {
        PyObject *result = PyObject_CallMethod((PyObject *)&self->list, "append", "O", args);
        if (result) {
            self->appends++;
        }
        return result;
    }

This leads to infinite recursion as the address of the first element of a C struct (``list``) is the address of the
struct so ``self`` is the same as ``&self->list``. This function is recursive with no base case.

--------------------------
Doing it Right
--------------------------

Our append method needs to use `super` to search our super-classes for the "append" method and call that.

Here are a couple of ways of calling ``super()`` correctly:

* Construct a ``super`` object directly and call that.
* Extract the ``super`` object from the builtins module and call that.

The full code is in ``src/cpy/Util/py_call_super.h`` and ``src/cpy/Util/py_call_super.c``.

.. index::
    single: Subclassing; Directly Calling super() from C

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Construct a ``super`` object directly
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The plan is to do this:

* Create the arguments to initialise an instance of the class ``super``.
* Call ``super.__new__`` with those arguments.
* Call ``super.__init__`` with those arguments.
* With that ``super`` object then search for the method we want to call.
  This is ``append`` in our case.
  This calls the ``super_getattro`` method that performs the search and returns the Python function.
* Call that Python function and return the result.

Our function is defined thus, for simplicity there is no error checking here. For the full function see below:

.. code-block:: c

    /* Call func_name on the super classes of self with the arguments and
     * keyword arguments.
     *
     * Equivalent to getattr(super(type(self), self), func_name)(*args, **kwargs)
     *
     * func_name is a Python string.
     * The implementation creates a new super object on each call.
     */
    PyObject *
    call_super_pyname(PyObject *self, PyObject *func_name,
                      PyObject *args, PyObject *kwargs) {
        PyObject *super_func = NULL;
        PyObject *super_args = NULL;
        PyObject *func = NULL;
        PyObject *result = NULL;

        // Will be decremented when super_args is decremented if Py_BuildValue succeeds.
        Py_INCREF(self->ob_type);
        Py_INCREF(self);
        super_args = Py_BuildValue("OO", (PyObject *) self->ob_type, self);
        super_func = PyType_GenericNew(&PySuper_Type, super_args, NULL);
        // Use tuple as first arg, super() second arg (i.e. kwargs) should be NULL
        super_func->ob_type->tp_init(super_func, super_args, NULL);
        func = PyObject_GetAttr(super_func, func_name);
        result = PyObject_Call(func, args, kwargs);
        Py_XDECREF(super_func);
        Py_XDECREF(super_args);
        Py_XDECREF(func);
        return result;
    }

We can make this function quite general to be used in the CPython type system.
For convenience we can create two functions, one calls the super function by a C NTS, the other by a PyObject string.
The following code is essentially the same as above but with error checking.

The header file might be py_call_super.h which just declares our two functions:

.. code-block:: c

    #ifndef __PythonSubclassList__py_call_super__
    #define __PythonSubclassList__py_call_super__

    #include <Python.h>

    extern PyObject *
    call_super_pyname(PyObject *self, PyObject *func_name,
                      PyObject *args, PyObject *kwargs);
    extern PyObject *
    call_super_name(PyObject *self, const char *func_cname,
                    PyObject *args, PyObject *kwargs);

    #endif /* defined(__PythonSubclassList__py_call_super__) */

And the implementation file would be py_call_super.c, this is the code above with full error checking:

.. code-block:: c

    /* Call func_name on the super classes of self with the arguments and
     * keyword arguments.
     *
     * Equivalent to getattr(super(type(self), self), func_name)(*args, **kwargs)
     *
     * func_name is a Python string.
     * The implementation creates a new super object on each call.
     */
    PyObject *
    call_super_pyname(PyObject *self, PyObject *func_name,
                      PyObject *args, PyObject *kwargs) {
        PyObject *super_func = NULL;
        PyObject *super_args = NULL;
        PyObject *func = NULL;
        PyObject *result = NULL;

        // Error check input
        if (!PyUnicode_Check(func_name)) {
            PyErr_Format(PyExc_TypeError,
                         "super() must be called with unicode attribute not %s",
                         Py_TYPE(func_name)->tp_name);
        }
        // Will be decremented when super_args is decremented if Py_BuildValue succeeds.
        Py_INCREF(self->ob_type);
        Py_INCREF(self);
        super_args = Py_BuildValue("OO", (PyObject *) self->ob_type, self);
        if (!super_args) {
            Py_DECREF(self->ob_type);
            Py_DECREF(self);
            PyErr_SetString(PyExc_RuntimeError, "Could not create arguments for super().");
            goto except;
        }
        super_func = PyType_GenericNew(&PySuper_Type, super_args, NULL);
        if (!super_func) {
            PyErr_SetString(PyExc_RuntimeError, "Could not create super().");
            goto except;
        }
        // Use tuple as first arg, super() second arg (i.e. kwargs) should be NULL
        super_func->ob_type->tp_init(super_func, super_args, NULL);
        if (PyErr_Occurred()) {
            goto except;
        }
        func = PyObject_GetAttr(super_func, func_name);
        if (!func) {
            assert(PyErr_Occurred());
            goto except;
        }
        if (!PyCallable_Check(func)) {
            PyErr_Format(PyExc_AttributeError,
                         "super() attribute \"%S\" is not callable.", func_name);
            goto except;
        }
        result = PyObject_Call(func, args, kwargs);
        if (!result) {
            assert(PyErr_Occurred());
            goto except;
        }
        assert(!PyErr_Occurred());
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(result);
        result = NULL;
    finally:
        Py_XDECREF(super_func);
        Py_XDECREF(super_args);
        Py_XDECREF(func);
        return result;
    }


.. index::
    single: Subclassing; Calling super() From builtins

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Extract the ``super`` object from the builtins
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Another way to do this is to fish out the `super` class from the builtins module and use that.
Incidentally this is how Cython does it.

The steps are:

#. Get the `builtins` module.
#. Get the `super` class from the `builtins` module.
#. Create a tuple of the arguments to pass to the super class.
#. Create the `super` object with the arguments.
#. Use this `super` object to call the function with the appropriate function arguments.

Again this code has no error checking for simplicity:

.. code-block:: c

    extern PyObject *
    call_super_pyname_lookup(PyObject *self, PyObject *func_name,
                             PyObject *args, PyObject *kwargs) {
        PyObject *result = NULL;
        PyObject *builtins = NULL;
        PyObject *super_type = NULL;
        PyObject *super = NULL;
        PyObject *super_args = NULL;
        PyObject *func = NULL;

        builtins = PyImport_AddModule("builtins");
        // Borrowed reference
        Py_INCREF(builtins);
        super_type = PyObject_GetAttrString(builtins, "super");
        // Will be decremented when super_args is decremented if Py_BuildValue succeeds.
        Py_INCREF(self->ob_type);
        Py_INCREF(self);
        super_args = Py_BuildValue("OO", (PyObject *) self->ob_type, self);
        super = PyObject_Call(super_type, super_args, NULL);
        // The following code is the same as call_super_pyname()
        func = PyObject_GetAttr(super, func_name);
        result = PyObject_Call(func, args, kwargs);
        Py_XDECREF(builtins);
        Py_XDECREF(super_args);
        Py_XDECREF(super_type);
        Py_XDECREF(super);
        Py_XDECREF(func);
        return result;
    }

Here is the function with full error checking:

.. code-block:: c

    /* Call func_name on the super classes of self with the arguments and
     * keyword arguments.
     *
     * Equivalent to getattr(super(type(self), self), func_name)(*args, **kwargs)
     *
     * func_name is a Python string.
     * The implementation uses the builtin super().
     */
    extern PyObject *
    call_super_pyname_lookup(PyObject *self, PyObject *func_name,
                             PyObject *args, PyObject *kwargs) {
        PyObject *result = NULL;
        PyObject *builtins = NULL;
        PyObject *super_type = NULL;
        PyObject *super = NULL;
        PyObject *super_args = NULL;
        PyObject *func = NULL;

        builtins = PyImport_AddModule("builtins");
        if (!builtins) {
            assert(PyErr_Occurred());
            goto except;
        }
        // Borrowed reference
        Py_INCREF(builtins);
        super_type = PyObject_GetAttrString(builtins, "super");
        if (!super_type) {
            assert(PyErr_Occurred());
            goto except;
        }
        // Will be decremented when super_args is decremented if Py_BuildValue succeeds.
        Py_INCREF(self->ob_type);
        Py_INCREF(self);
        super_args = Py_BuildValue("OO", (PyObject *) self->ob_type, self);
        if (!super_args) {
            Py_DECREF(self->ob_type);
            Py_DECREF(self);
            PyErr_SetString(PyExc_RuntimeError, "Could not create arguments for super().");
            goto except;
        }
        super = PyObject_Call(super_type, super_args, NULL);
        if (!super) {
            assert(PyErr_Occurred());
            goto except;
        }
        // The following code is the same as call_super_pyname()
        func = PyObject_GetAttr(super, func_name);
        if (!func) {
            assert(PyErr_Occurred());
            goto except;
        }
        if (!PyCallable_Check(func)) {
            PyErr_Format(PyExc_AttributeError,
                         "super() attribute \"%S\" is not callable.", func_name);
            goto except;
        }
        result = PyObject_Call(func, args, kwargs);
        if (!result) {
            assert(PyErr_Occurred());
            goto except;
        }
        assert(!PyErr_Occurred());
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(result);
        result = NULL;
    finally:
        Py_XDECREF(builtins);
        Py_XDECREF(super_args);
        Py_XDECREF(super_type);
        Py_XDECREF(super);
        Py_XDECREF(func);
        return result;
    }


.. index::
    single: Subclassing; With Overloading

=====================================
Subclassing with Overloading
=====================================

Lets revisit our subclass of the builtin ``list`` that counts how many time ``append()`` is called and we will use
the C ``super()`` API described above.

The full code is in ``src/cpy/SubClass/sublist.c`` and the tests are in ``tests/unit/test_c_subclass.py``.

-----------------------------
Writing the C Extension
-----------------------------

First the declaration and initialisation of ``SubListObject`` that has an ``appends`` counter.
Note the inclusion of ``py_call_super.h``:

.. code-block:: c

    #define PY_SSIZE_T_CLEAN
    #include <Python.h>
    #include "structmember.h"

    #include "py_call_super.h"

    typedef struct {
        PyListObject list;
        int appends;
    } SubListObject;

    static int
    SubList_init(SubListObject *self, PyObject *args, PyObject *kwds) {
        if (PyList_Type.tp_init((PyObject *) self, args, kwds) < 0) {
            return -1;
        }
        self->appends = 0;
        return 0;
    }

Now the implementation of ``append`` that makes the ``super()`` call and then increments the ``appends`` attribute
and returning the value of the ``super()`` call:

.. code-block:: c

    static PyObject *
    SubList_append(SubListObject *self, PyObject *args) {
        PyObject *result = call_super_name((PyObject *)self, "append",
                                           args, NULL);
        if (result) {
            self->appends++;
        }
        return result;
    }

The declaration of methods, members and the type:

.. code-block:: c

    static PyMethodDef SubList_methods[] = {
            {"append", (PyCFunction) SubList_append, METH_VARARGS,
                    PyDoc_STR("append an item")},
            {NULL, NULL, 0, NULL},
    };

    static PyMemberDef SubList_members[] = {
            {"appends", T_INT, offsetof(SubListObject, appends), 0,
                    "Number of append operations."},
            {NULL, 0, 0, 0, NULL}  /* Sentinel */
    };

    static PyTypeObject SubListType = {
            PyVarObject_HEAD_INIT(NULL, 0)
            .tp_name = "sublist.SubList",
            .tp_doc = PyDoc_STR("SubList objects"),
            .tp_basicsize = sizeof(SubListObject),
            .tp_itemsize = 0,
            .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
            .tp_init = (initproc) SubList_init,
            .tp_methods = SubList_methods,
            .tp_members = SubList_members,
    };

Finally the module definition which is very similar to before:

.. code-block:: c

    static PyModuleDef sublistmodule = {
            PyModuleDef_HEAD_INIT,
            .m_name = "sublist",
            .m_doc = "Module that contains a subclass of a list.",
            .m_size = -1,
    };

    PyMODINIT_FUNC
    PyInit_sublist(void) {
        PyObject *m;
        SubListType.tp_base = &PyList_Type;
        if (PyType_Ready(&SubListType) < 0) {
            return NULL;
        }
        m = PyModule_Create(&sublistmodule);
        if (m == NULL) {
            return NULL;
        }
        Py_INCREF(&SubListType);
        if (PyModule_AddObject(m, "SubList", (PyObject *) &SubListType) < 0) {
            Py_DECREF(&SubListType);
            Py_DECREF(m);
            return NULL;
        }
        return m;
    }

-----------------------------
Setup and Build
-----------------------------

In the ``setup.py`` add an Extension such as:

.. code-block:: python

    Extension(name=f"cPyExtPatt.SubClass.sublist",
              include_dirs=[
                  '/usr/local/include',
              ],
              sources=[
                  "src/cpy/SubClass/sublist.c",
              ],
              extra_compile_args=extra_compile_args_c,
              language='c',
              ),

The extension can be built with ``python setup.py develop``.

-----------------------------
Test
-----------------------------

The extension can used like this:

.. code-block:: python

    from cPyExtPatt.SubClass import sublist

    obj = sublist.SubList()
    assert obj.appends == 0
    obj.append(42)
    assert obj.appends == 1
    assert obj == [42, ]


.. index::
    single: Subclassing; datetime Example

--------------------------------------
Another Example of Using this API
--------------------------------------

Here is a real example of using this see overloading ``replace()`` when subclassing a ``datetime`` in
:ref:`chapter_capsules_using_an_existing_capsule` from the chapter :ref:`chapter_capsules`.

The code here calls the ``super()`` function then raises if the given arguments are unacceptable (trying to set the
``tzinfo`` property to ``None``):

.. code-block:: c

    static PyObject *
    DateTimeTZ_replace(PyObject *self, PyObject *args, PyObject *kwargs) {
        PyObject *result = call_super_name(self, "replace", args, kwargs);
        if (result) {
            result = (PyObject *) raise_if_no_tzinfo((DateTimeTZ *) result);
        }
        return result;
    }
