.. toctree::
    :maxdepth: 3

.. index::
    single: C Functions; Coding Pattern

.. _chapter_function_pattern:

===========================================
A Pythonic Coding Pattern for C Functions
===========================================

Principle
===============

To avoid all the errors we have seen, particularly in :ref:`chapter_refcount` it is useful to have a C coding pattern
for handling ``PyObjects`` that does the following:

* No early returns and a single place for clean up code.
* Borrowed references incref'd and decref'd correctly.
* No stale Exception from a previous execution path.
* Exceptions set and tested.
* NULL is returned when an exception is set.
* Non-NULL is returned when no exception is set.

The basic pattern in C is similar to Python's try/except/finally pattern:

.. code-block:: c

    try:
        /* Do fabulous stuff here. */
    except:
        /* Handle every abnormal condition and clean up. */
    finally:
        /* Clean up under normal conditions and return an appropriate value. */


Coding the Function
=====================

Firstly we set any local ``PyObject`` (s) and the return value to ``NULL``:

.. code-block:: c

    static PyObject *function(PyObject *arg_1) {
        PyObject *obj_a    = NULL;
        PyObject *ret      = NULL;
    
Then we have a little bit of Pythonic C - this can be omitted:

.. code-block:: c

        goto try; /* Pythonic 'C' ;-) */
    try:

Check that there are no lingering Exceptions:

.. code-block:: c

        assert(! PyErr_Occurred());

An alternative check for no lingering Exceptions with non-debug builds:

.. code-block:: c

        if(PyErr_Occurred()) {
            PyObject_Print(PyErr_Occurred(), stdout, Py_PRINT_RAW);
            goto except;
        }

Now we assume that any argument is a "Borrowed" reference so we increment it (we need a matching ``Py_DECREF`` before
function exit, see below). The first pattern assumes a non-NULL argument.

.. code-block:: c

        assert(arg_1);
        Py_INCREF(arg_1);

If you are willing to accept NULL arguments then this pattern would be more suitable:

.. code-block:: c
    
    if (arg_1) {
        Py_INCREF(arg_1);
    }
    
Or just use ``Py_XINCREF``.

Now we create any local objects, if they are "Borrowed" references we need to incref them.
With any abnormal behaviour we do a local jump straight to the cleanup code.

.. code-block:: c

        /* Local object creation. */
        /* obj_a = ...; */
        if (! obj_a) {
            PyErr_SetString(PyExc_ValueError, "Ooops.");
            goto except;
        }
        /* If obj_a is a borrowed reference rather than a new reference. */
        Py_INCREF(obj_a);

Create the return value and deal with abnormal behaviour in the same way:
 
.. code-block:: c

        /* More of your code to do stuff with arg_1 and obj_a. */    
        /* Return object creation, ret should be a new reference otherwise you are in trouble. */
        /* ret = ...; */
        if (! ret) {
            PyErr_SetString(PyExc_ValueError, "Ooops again.");
            goto except;
        }

You might want to check the contents of the return value here. On error jump to ``except:`` otherwise jump to ``finally:``.

.. code-block:: c

        /* Any return value checking here. */
    
        /* If success then check exception is clear,
         * then goto finally; with non-NULL return value. */
        assert(! PyErr_Occurred());
        assert(ret);
        goto finally;

This is the except block where we cleanup any local objects and set the return value to NULL.

.. code-block:: c

    except:
        /* Failure so Py_XDECREF the return value here. */
        Py_XDECREF(ret);
        /* Check a Python error is set somewhere above. */
        assert(PyErr_Occurred());
        /* Signal failure. */
        ret = NULL;

Notice the ``except:`` block falls through to the ``finally:`` block.

.. code-block:: c

    finally:
        /* All _local_ PyObjects declared at the entry point are Py_XDECREF'd here.
         * For new references this will free them. For borrowed references this
         * will return them to their previous refcount.
         */
        Py_XDECREF(obj_a);
        /* Decrement the ref count of externally supplied the arguments here.
         * If you allow arg_1 == NULL then Py_XDECREF(arg_1). */
        Py_DECREF(arg_1);
        /* And return...*/
        return ret;
    }

.. index::
    single: C Functions; Full Coding Pattern

.. raw:: latex

    [Continued on the next page]

    \pagebreak

The Function Code as One
========================

Here is the complete code with minimal comments:

.. code-block:: c

    static PyObject *function(PyObject *arg_1) {
        PyObject *obj_a    = NULL;
        PyObject *ret      = NULL;
    
        goto try;
    try:
        assert(! PyErr_Occurred());
        assert(arg_1);
        Py_INCREF(arg_1);
    
        /* Create obj_a = ...; */

        if (! obj_a) {
            PyErr_SetString(PyExc_ValueError, "Ooops.");
            goto except;
        }
        /* Only do this if obj_a is a borrowed reference. */
        Py_INCREF(obj_a);
    
        /* More of your code to do stuff with obj_a. */
    
        /* Return object creation, ret must be a new reference. */
        /* ret = ...; */
        if (! ret) {
            PyErr_SetString(PyExc_ValueError, "Ooops again.");
            goto except;
        }
        assert(! PyErr_Occurred());
        assert(ret);
        goto finally;
    except:
        Py_XDECREF(ret);
        assert(PyErr_Occurred());
        ret = NULL;
    finally:
        /* Only do this if obj_a is a borrowed reference. */
        Py_XDECREF(obj_a);
        Py_DECREF(arg_1);
        return ret;
    }
