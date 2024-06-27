.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

====================================
Setting and Getting Module Globals
====================================

This section describes how you create and access module globals from Python C Extensions.

In this module, written as a Python extension in C, we are going to have a string, int, list, tuple and dict in global
scope. In the C code we firstly define names for them:


.. code-block:: c

    const char *NAME_INT = "INT";
    const char *NAME_STR = "STR";
    const char *NAME_LST = "LST";
    const char *NAME_TUP = "TUP";
    const char *NAME_MAP = "MAP";

These are the names of the objects that will appear in the Python module::

    >>> import cModuleGlobals
    >>> dir(cModuleGlobals)
    ['INT', 'LST', 'MAP', 'STR', 'TUP', '__doc__', '__file__', '__loader__', '__name__', '__package__', 'print']

------------------------------------
Initialising Module Globals
------------------------------------

This is the module declaration, it will be called ``cModuleGlobals`` and has just one function; ``print()`` that will
access the module globals from C:

.. code-block:: c

    static PyMethodDef cModuleGlobals_methods[] = {
        {"print", (PyCFunction)print_globals, METH_NOARGS,
            "Access and print out the globals."
        },
        {NULL, NULL, 0, NULL}  /* Sentinel */
    };


    static PyModuleDef cModuleGlobals_module = {
        PyModuleDef_HEAD_INIT,
        "cModuleGlobals",
        "Examples of global values in a module.",
        -1,
        cModuleGlobals_methods, /* cModuleGlobals_methods */
        NULL, /* inquiry m_reload */
        NULL, /* traverseproc m_traverse */
        NULL, /* inquiry m_clear */
        NULL, /* freefunc m_free */
    };

The module initialisation code is next, this uses the Python C API to create the various global objects:

.. code-block:: c

    PyMODINIT_FUNC
    PyInit_cModuleGlobals(void)
    {
        PyObject *m = NULL;
    
        m = PyModule_Create(&cModuleGlobals_module);
    
        if (m == NULL) {
            goto except;
        }
        /* Adding module globals */
        if (PyModule_AddIntConstant(m, NAME_INT, 42)) {
            goto except;
        }
        if (PyModule_AddStringConstant(m, NAME_STR, "String value")) {
            goto except;
        }
        if (PyModule_AddObject(m, NAME_TUP, Py_BuildValue("iii", 66, 68, 73))) {
            goto except;
        }
        if (PyModule_AddObject(m, NAME_LST, Py_BuildValue("[iii]", 66, 68, 73))) {
            goto except;
        }
        /* An invented convenience function for this dict. See below. */
        if (add_map_to_module(m)) {
            goto except;
        }
        goto finally;
    except:
        Py_XDECREF(m);
        m = NULL;
    finally:
        return m;
    }

The dict is added in a separate C function merely for readability:

.. code-block:: c

    /* Add a dict of {str : int, ...}.
     * Returns 0 on success, 1 on failure.
     */
    int add_map_to_module(PyObject *module) {
        int ret = 0;
        PyObject *pMap = NULL;
    
        pMap = PyDict_New();
        if (! pMap) {
            goto except;
        }
        /* Load map. */
        if (PyDict_SetItem(pMap, PyBytes_FromString("66"), PyLong_FromLong(66))) {
            goto except;
        }
        if (PyDict_SetItem(pMap, PyBytes_FromString("123"), PyLong_FromLong(123))) {
            goto except;
        }
        /* Add map to module. */
        if (PyModule_AddObject(module, NAME_MAP, pMap)) {
            goto except;
        }
        ret = 0;
        goto finally;
    except:
        Py_XDECREF(pMap);
        ret = 1;
    finally:
        return ret;
    }

------------------------------------
Getting and Setting Module Globals
------------------------------------

^^^^^^^^^^^^^^^^^^
From Python
^^^^^^^^^^^^^^^^^^

Once the module is built we can access the globals from Python as usual::

    >>> import cModuleGlobals
    >>> dir(cModuleGlobals)
    ['INT', 'LST', 'MAP', 'STR', 'TUP', '__doc__', '__file__', '__loader__', '__name__', '__package__', 'print']
    >>> cModuleGlobals.STR
    'String value'
    >>> cModuleGlobals.STR = 'F'
    >>> cModuleGlobals.STR 
    'F'
    >>> cModuleGlobals.MAP
    {b'123': 123, b'66': 66}
    >>> cModuleGlobals.MAP[b'asd'] = 9
    >>> cModuleGlobals.MAP
    {b'123': 123, b'asd': 9, b'66': 66}

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Getting Module Globals From C
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Accessing Python module globals from C is a little bit more tedious as we are getting borrowed references from the
modules ``__dict__`` and we should increment and decrement them appropriately.
Here we print out the global ``INT`` as both a Python object and a 'C' ``long``:

.. code-block:: c

    static PyObject *print_global_INT(PyObject *pMod) {
        PyObject *ret = NULL;
        PyObject *pItem = NULL;
        long val;
        
        /* Sanity check. */
        assert(pMod);
        assert(PyModule_CheckExact(pMod));
        assert(! PyErr_Occurred());
        
        /* NOTE: PyModule_GetDict(pMod); never fails and returns a borrowed
         * reference. pItem is NULL or a borrowed reference.
         */
        pItem = PyDict_GetItemString(PyModule_GetDict(pMod), NAME_INT);
        if (! pItem) {
            PyErr_Format(PyExc_AttributeError,
                         "Module '%s' has no attibute '%s'.", \
                         PyModule_GetName(pMod), NAME_INT
                         );
            goto except;
        }
        Py_INCREF(pItem);
        fprintf(stdout, "Integer: \"%s\" ", NAME_INT);
        PyObject_Print(pItem, stdout, 0);
        val = PyLong_AsLong(pItem);
        fprintf(stdout, " C long: %ld ", val);
        fprintf(stdout, "\n");

        assert(! PyErr_Occurred());
        Py_INCREF(Py_None);
        ret = Py_None;
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        Py_DECREF(pItem);
        return ret;
    }

From Python we would see this (C's ``print_global_INT()`` is mapped to Python's ``cModuleGlobals.printINT()``):

    >>> import cModuleGlobals
    >>> cModuleGlobals.printINT()
    Module:
    <module 'cModuleGlobals' from './cModuleGlobals.so'>
    Integer: "INT" 42 C long: 42 

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Setting Module Globals From C
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This is similar to the get code above but using ``int PyDict_SetItemString(PyObject *p, const char *key, PyObject *val)``
where val will be a *stolen* reference:

.. code-block:: c

    static PyObject *some_set_function(PyObject *pMod) {
        PyObject *ret = NULL;
        long val = ...; /* Some computed value. */
        
        if (PyDict_SetItemString(PyModule_GetDict(pMod), NAME_INT, PyLong_FromLong(val))) {
            PyErr_Format(PyExc_AttributeError,
                         "Can not set Module '%s' attibute '%s'.", \
                         PyModule_GetName(pMod), NAME_INT
                         );
            goto except;
        }

        assert(! PyErr_Occurred());
        Py_INCREF(Py_None);
        ret = Py_None;
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        return ret;
    }
