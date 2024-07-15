.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

=================================
Calling ``super()`` from C 
=================================

I needed to call super() from a C extension and I couldn't find a good description of how to do this online so I am
including this here.

Suppose we wanted to subclass a list and record how many times ``append()`` was called. This is simple enough in pure
Python:

.. code-block:: python

    class SubList(list):
        def __init__(self, *args, **kwargs):
            self.appends = 0
            super().__init__(*args, **kwargs)

        def append(self, v):
            self.appends += 1
            return super().append(v)

To do it in C is a bit trickier. Taking as our starting point the `example of sub-classing a list <https://docs.python.org/3/extending/newtypes.html#subclassing-other-types>`_ in the Python documentation, amended a little bit for our example.

Our type contains an integer count of the number of appends. That is set to zero on construction and can be accesssed like a normal member. 

.. code-block:: c

    typedef struct {
        PyListObject list;
        int appends;
    } Shoddy;


    static int
    Shoddy_init(Shoddy *self, PyObject *args, PyObject *kwds)
    {
        if (PyList_Type.tp_init((PyObject *)self, args, kwds) < 0) {
            return -1;
        }
        self->appends = 0;
        return 0;
    }

    static PyMemberDef Shoddy_members[] = {
        ...
        {"appends", T_INT, offsetof(Shoddy, appends), 0,
            "Number of append operations."},
        ...
        {NULL, 0, 0, 0, NULL}  /* Sentinel */
    };

We now need to create the ``append()`` function, this function will call the superclass ``append()`` and increment the ``appends`` counter:

.. code-block:: c

    static PyMethodDef Shoddy_methods[] = {
        ...
        {"append", (PyCFunction)Shoddy_append, METH_VARARGS,
            PyDoc_STR("Append to the list")},
        ...
        {NULL,	NULL, 0, NULL},
    };

This is where it gets tricky, how do we implement ``Shoddy_append``?

--------------------------
The Obvious Way is Wrong
--------------------------

A first attempt might do something like a method call on the ``PyListObject``:
 
.. code-block:: c

    typedef struct {
        PyListObject list;
        int appends;
    } Shoddy;

    /* Other stuff here. */
    
    static PyObject *
    Shoddy_append(Shoddy *self, PyObject *args) {
        PyObject *result = PyObject_CallMethod((PyObject *)&self->list, "append", "O", args);
        if (result) {
            self->appends++;
        }
        return result;
    }

This leads to infinite recursion as the address of the first element of a C struct (``list``) is the address of the struct so ``self`` is the same as ``&self->list``. This function is recursive with no base case.

--------------------------
Doing it Right
--------------------------

Our append method needs to use `super` to search our super-classes for the "append" method and call that.

Here are a couple of ways of calling ``super()`` correctly:

* Construct a ``super`` object directly and call that.
* Extract the ``super`` object from the builtins module and call that.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Construct a ``super`` object directly
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The plan is to do this:

* Create the arguments to initialise an instance of the class ``super``.
* Call ``super.__new__`` with those arguments.
* Call ``super.__init__`` with those arguments.
* With that ``super`` object then search for the method we want to call. This is ``append`` in our case. This calls the ``super_getattro`` method that performs the search and returns the Python function.
* Call that Python function and return the result.

Our function is defined thus, for simplicity there is no error checking here. For the full function see below:

.. code-block:: c

    PyObject *
    call_super_pyname(PyObject *self, PyObject *func_name, PyObject *args, PyObject *kwargs) {
        PyObject *super      = NULL;
        PyObject *super_args = NULL;
        PyObject *func       = NULL;
        PyObject *result     = NULL;

        // Create the arguments for super()
        super_args = PyTuple_New(2);
        Py_INCREF(self->ob_type); // Py_INCREF(&ShoddyType); in our specific case
        PyTuple_SetItem(super_args, 0, (PyObject*)self->ob_type)); // PyTuple_SetItem(super_args, 0, (PyObject*)&ShoddyType) in our specific case
        Py_INCREF(self);
        PyTuple_SetItem(super_args, 1, self));
        // Creat the class super()
        super = PyType_GenericNew(&PySuper_Type, super_args, NULL);
        // Instantiate it with the tuple as first arg, no kwargs passed to super() so NULL
        super->ob_type->tp_init(super, super_args, NULL);
        // Use super to find the 'append' method
        func = PyObject_GetAttr(super, func_name);
        // Call that method
        result = PyObject_Call(func, args, kwargs);
        Py_XDECREF(super);
        Py_XDECREF(super_args);
        Py_XDECREF(func);
        return result;
    }

We can make this function quite general to be used in the CPython type system. For convenience we can create two functions, one calls the super function by a C NTS, the other by a PyObject string. The following code is essentially the same as above but with error checking.

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

    PyObject *
    call_super_pyname(PyObject *self, PyObject *func_name,
                      PyObject *args, PyObject *kwargs) {
        PyObject *super      = NULL;
        PyObject *super_args = NULL;
        PyObject *func       = NULL;
        PyObject *result     = NULL;
    
        if (! PyUnicode_Check(func_name)) {
            PyErr_Format(PyExc_TypeError,
                         "super() must be called with unicode attribute not %s",
                         func_name->ob_type->tp_name);
        }
    
        super_args = PyTuple_New(2);
        //    Py_INCREF(&ShoddyType);
        Py_INCREF(self->ob_type);
        //    if (PyTuple_SetItem(super_args, 0, (PyObject*)&ShoddyType)) {
        if (PyTuple_SetItem(super_args, 0, (PyObject*)self->ob_type)) {
            assert(PyErr_Occurred());
            goto except;
        }
        Py_INCREF(self);
        if (PyTuple_SetItem(super_args, 1, self)) {
            assert(PyErr_Occurred());
            goto except;
        }
    
        super = PyType_GenericNew(&PySuper_Type, super_args, NULL);
        if (! super) {
            PyErr_SetString(PyExc_RuntimeError, "Could not create super().");
            goto except;
        }
        // Make tuple as first arg, second arg (i.e. kwargs) should be NULL
        super->ob_type->tp_init(super, super_args, NULL);
        if (PyErr_Occurred()) {
            goto except;
        }
        func = PyObject_GetAttr(super, func_name);
        if (! func) {
            assert(PyErr_Occurred());
            goto except;
        }
        if (! PyCallable_Check(func)) {
            PyErr_Format(PyExc_AttributeError,
                         "super() attribute \"%S\" is not callable.", func_name);
            goto except;
        }
        result = PyObject_Call(func, args, kwargs);
        assert(! PyErr_Occurred());
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(result);
        result = NULL;
    finally:
        Py_XDECREF(super);
        Py_XDECREF(super_args);
        Py_XDECREF(func);
        return result;
    }

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Extract the ``super`` object from the builtins
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Another way to do this is to fish out the `super` class from the builtins module and use that. Incidentially this is how Cython does it.

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
        PyObject *builtins = PyImport_AddModule("builtins");
        // Borrowed reference
        Py_INCREF(builtins);
        PyObject *super_type = PyObject_GetAttrString(builtins, "super");
        PyObject *super_args = PyTuple_New(2);
        Py_INCREF(self->ob_type);
        PyTuple_SetItem(super_args, 0, (PyObject*)self->ob_type);
        Py_INCREF(self);
        PyTuple_SetItem(super_args, 1, self);
        PyObject *super = PyObject_Call(super_type, super_args, NULL);
        PyObject *func = PyObject_GetAttr(super, func_name);
        PyObject *result = PyObject_Call(func, args, kwargs);
        Py_XDECREF(builtins);
        Py_XDECREF(super_args);
        Py_XDECREF(super_type);
        Py_XDECREF(super);
        Py_XDECREF(func);
        return result;
    }

Here is the function with full error checking:

.. code-block:: c

    extern PyObject *
    call_super_pyname_lookup(PyObject *self, PyObject *func_name,
                             PyObject *args, PyObject *kwargs) {
        PyObject *result        = NULL;
        PyObject *builtins      = NULL;
        PyObject *super_type    = NULL;
        PyObject *super         = NULL;
        PyObject *super_args    = NULL;
        PyObject *func          = NULL;
    
        builtins = PyImport_AddModule("builtins");
        if (! builtins) {
            assert(PyErr_Occurred());
            goto except;
        }
        // Borrowed reference
        Py_INCREF(builtins);
        super_type = PyObject_GetAttrString(builtins, "super");
        if (! super_type) {
            assert(PyErr_Occurred());
            goto except;
        }
        super_args = PyTuple_New(2);
        Py_INCREF(self->ob_type);
        if (PyTuple_SetItem(super_args, 0, (PyObject*)self->ob_type)) {
            assert(PyErr_Occurred());
            goto except;
        }
        Py_INCREF(self);
        if (PyTuple_SetItem(super_args, 1, self)) {
            assert(PyErr_Occurred());
            goto except;
        }
        super = PyObject_Call(super_type, super_args, NULL);
        if (! super) {
            assert(PyErr_Occurred());
            goto except;
        }
        func = PyObject_GetAttr(super, func_name);
        if (! func) {
            assert(PyErr_Occurred());
            goto except;
        }
        if (! PyCallable_Check(func)) {
            PyErr_Format(PyExc_AttributeError,
                         "super() attribute \"%S\" is not callable.", func_name);
            goto except;
        }
        result = PyObject_Call(func, args, kwargs);
        assert(! PyErr_Occurred());
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
