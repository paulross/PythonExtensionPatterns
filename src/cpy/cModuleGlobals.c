//
//  cModuleGlobals.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 09/05/2014.
//  Copyright (c) 2014 Paul Ross. All rights reserved.
//

/* This is the code used for the documentation at:
 * https://github.com/paulross/PythonExtensionPatterns/doc/sphinx/source/module_globals.rst
 * Or:
 * http://pythonextensionpatterns.readthedocs.io/en/latest/module_globals.html
 *
 */

#include "Python.h"

const char *NAME_INT = "INT";
const char *NAME_STR = "STR";
const char *NAME_LST = "LST";
const char *NAME_TUP = "TUP";
const char *NAME_MAP = "MAP";

static PyObject *_print_global_INT(PyObject *pMod) {
	PyObject *ret = NULL;
    PyObject *pItem = NULL;
    long val;
	
    /* Returns a new reference. */
    pItem = PyObject_GetAttrString(pMod, NAME_INT);
    if (! pItem) {
        PyErr_Format(PyExc_AttributeError,
                     "Module '%s' has no attibute '%s'.", \
                     PyModule_GetName(pMod), NAME_INT
                     );
        goto except;
    }
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
    Py_XDECREF(pItem);
	Py_XDECREF(ret);
	ret = NULL;
finally:
	return ret;
}

static PyObject *_print_global_INT_borrowed_ref(PyObject *pMod) {
	PyObject *ret = NULL;
    PyObject *pItem = NULL;
    long val;

    assert(pMod);
    assert(PyModule_CheckExact(pMod));
    assert(! PyErr_Occurred());

    fprintf(stdout, "Module:\n");
    PyObject_Print(pMod, stdout, 0);
    fprintf(stdout, "\n");

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

static PyObject *_print_globals(PyObject *pMod) {
	PyObject *ret = NULL;
    PyObject *pItem = NULL;

    assert(pMod);
    assert(PyModule_CheckExact(pMod));
    assert(! PyErr_Occurred());

    fprintf(stdout, "cModuleGlobals:\n");
    PyObject_Print(pMod, stdout, 0);
    fprintf(stdout, "\n");

    /* Your code here...*/
    if (! _print_global_INT(pMod)) {
    	goto except;
    }

    if (! _print_global_INT_borrowed_ref(pMod)) {
    	goto except;
    }

    pItem = PyObject_GetAttrString(pMod, NAME_STR);
    if (! pItem) {
        PyErr_Format(PyExc_AttributeError,
                     "Module '%s' has no attibute '%s'.", \
                     PyModule_GetName(pMod), NAME_STR
                     );
        goto except;
    }
    fprintf(stdout, " String: \"%s\" ", NAME_STR);
    PyObject_Print(pItem, stdout, 0);
    fprintf(stdout, "\n");
    Py_DECREF(pItem);
    pItem = NULL;
    
    pItem = PyObject_GetAttrString(pMod, NAME_LST);
    if (! pItem) {
        PyErr_Format(PyExc_AttributeError,
                     "Module '%s' has no attibute '%s'.", \
                     PyModule_GetName(pMod), NAME_LST
                     );
        goto except;
    }
    fprintf(stdout, "   List: \"%s\" ", NAME_LST);
    PyObject_Print(pItem, stdout, 0);
    fprintf(stdout, "\n");
    Py_DECREF(pItem);
    pItem = NULL;
    
    pItem = PyObject_GetAttrString(pMod, NAME_MAP);
    if (! pItem) {
        PyErr_Format(PyExc_AttributeError,
                     "Module '%s' has no attibute '%s'.", \
                     PyModule_GetName(pMod), NAME_MAP
                     );
        goto except;
    }
    fprintf(stdout, "    Map: \"%s\" ", NAME_MAP);
    PyObject_Print(pItem, stdout, 0);
    fprintf(stdout, "\n");
    Py_DECREF(pItem);
    pItem = NULL;
    
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



static PyMethodDef cModuleGlobals_methods[] = {
    {"print", (PyCFunction)_print_globals, METH_NOARGS,
		"Access and print out th globals."
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

/* Add a dict of {str : int, ...}.
 * Returns 0 on success, 1 on failure.
 */
int _add_map_to_module(PyObject *module) {
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
    /* An invented convenience function for this dict. */
    if (_add_map_to_module(m)) {
        goto except;
    }
	
//    if (PyType_Ready(&cPhysRecType)) {
//        return NULL;
//    }
//    Py_INCREF(&cPhysRecType);
//    PyModule_AddObject(m, "cPhysRec", (PyObject *)&cPhysRecType);
    goto finally;
except:
    Py_XDECREF(m);
    m = NULL;
finally:
    return m;
}
