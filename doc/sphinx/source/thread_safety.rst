.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

====================================
Thread Safety
====================================

If your Extension is likely to be exposed to a multi-threaded environment then you need to think aout thread safety. I had this problem with a separate project which was a `SkipList <https://github.com/paulross/skiplist/>`_ which could contain an ordered list of arbitrary Python objects. The problem in a multi-threaded environment was that the following sequence of events could happen:

* Thread A tries to insert a Python object. The C++ code searches for a place to insert it preserving the existing order. To do so it must call back into Python code for the user defined comparison (using ``functools.total_ordering`` for example).
* At this point the Python interpreter might do a context switch allowing thread B to, say, remove an element from the SkipList. This removal may well invalidate C++ pointers held by thread A.
* When the interpreter switches back to thread A it accesses an invalid pointer and a segfault happens.

The solution, of course, is to use a lock to prevent a context switch until A has completed its insertion, but how? I found the existing Python documentation misleading and I couldn't get it to work reliably, if at all. It was only when I stumbled upon the `source code <https://github.com/python/cpython/blob/master/Modules/_bz2module.c>`_ for the `bz module <https://docs.python.org/3/library/bz2.html#module-bz2>`_ that I realised there was a whole other, low level and undocumented way of doing this.

Python may have been compiled without thread support in which case we don't have to concern ourselves with thread locking. We can discover this from the macro ``WITH_THREADS`` so all our thread support code is conditional on the definition of this macro.

Coding up the Lock
----------------------------

First we need to include ``pthread.h``:

.. code-block:: c

    #include <Python.h>
    #include "structmember.h"
    
    #ifdef WITH_THREAD
    #include "pythread.h"
    #endif

Adding a ``PyThread_type_lock`` to our object
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Then we add a ``PyThread_type_lock`` to our Python structure, I'll use the example of the `SkipList source code <https://github.com/paulross/skiplist/blob/master/src/cpy/cSkipList.cpp>`_:

.. code-block:: c

    typedef struct {
        PyObject_HEAD
        /* Other stuff here... */
    #ifdef WITH_THREAD
        PyThread_type_lock lock;
    #endif
    } SkipList;

Creating a class to Acquire and Release the Lock
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Now we add some code to acquire and release the lock. We can do this in a RAII fashion in C++ [#f1]_:

.. code-block:: c

    #ifdef WITH_THREAD
    /* A RAII wrapper around the PyThread_type_lock. */
    class AcquireLock {
    public:
        AcquireLock(SkipList *pSL) : _pSL(pSL) {
            assert(pSL);
            assert(pSL->lock);
            if (! PyThread_acquire_lock(_pSL->lock, NOWAIT_LOCK)) {
                Py_BEGIN_ALLOW_THREADS
                PyThread_acquire_lock(_pSL->lock, WAIT_LOCK);
                Py_END_ALLOW_THREADS
            }
        }
        ~AcquireLock() {
            assert(_pSL);
            assert(_pSL->lock);
            PyThread_release_lock(_pSL->lock);
        }
    private:
        SkipList *_pSL;
    };
    #else
    /* Make the class a NOP which should get optimised out. */
    class AcquireLock {
    public:
        AcquireLock(SkipList *) {}
    };
    #endif

Initialising and Deallocating the Lock
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Now we need to set the lock pointer to ``NULL`` in the ``_new`` function:

.. code-block:: c
    :linenos:
    :emphasize-lines: 8-10

    static PyObject *
    SkipList_new(PyTypeObject *type, PyObject * /* args */, PyObject * /* kwargs */) {
        SkipList *self = NULL;
        
        self = (SkipList *)type->tp_alloc(type, 0);
        if (self != NULL) {
            /* Initialise struct SkipList fields... */
    #ifdef WITH_THREAD
            self->lock = NULL;
    #endif
        }
        return (PyObject *)self;
    }

In the ``__init__`` method we allocate the lock by calling ``PyThread_allocate_lock()`` [#f2]_:

.. code-block:: c
    :linenos:
    :emphasize-lines: 12-18

    static int
    SkipList_init(SkipList *self, PyObject *args, PyObject *kwargs) {
        int ret_val = -1;
        PyObject *value_type    = NULL;
        PyObject *cmp_func      = NULL;
        static char *kwlist[] = {
            (char *)"value_type",
            (char *)"cmp_func",
            NULL
        };
        assert(self);
    #ifdef WITH_THREAD
        self->lock = PyThread_allocate_lock();
        if (self->lock == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Unable to allocate thread lock.");
            goto except;
        }
    #endif
        /* 
         * Much more stuff here...
         */
        assert(! PyErr_Occurred());
        assert(self);
        assert(self->pSl_void);
        ret_val = 0;
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(self);
        ret_val = -1;
    finally:
        return ret_val;
    }

When deallocating the object we should free the lock pointer:

.. code-block:: c
    :linenos:
    :emphasize-lines: 6-11

    static void
    SkipList_dealloc(SkipList *self) {
        /*
         * Deallocate other fields here...
         */
    #ifdef WITH_THREAD
            if (self->lock) {
                PyThread_free_lock(self->lock);
                self->lock = NULL;
            }
    #endif
            Py_TYPE(self)->tp_free((PyObject*)self);
        }
    }

Using the Lock
^^^^^^^^^^^^^^^^^^^^^^^^

Before any critical code we create an ``AcquireLock`` object:

.. code-block:: c
    :linenos:
    :emphasize-lines: 8,19

    static PyObject *
    SkipList_insert(SkipList *self, PyObject *arg) {
        assert(self && self->pSl_void);
        /*
         * Lots of stuff here...
         */
        {
            AcquireLock _lock(self);
            try {
                self->pSl_object->insert(arg);
            } catch (std::invalid_argument &err) {
                // Thrown if PyObject_RichCompareBool returns -1
                // A TypeError should be set
                if (! PyErr_Occurred()) {
                    PyErr_SetString(PyExc_TypeError, err.what());
                }
                return NULL;
            }
            /* Lock automatically released here. */
        }
        /*
         * More stuff here...
         */
         Py_RETURN_NONE;
    }

.. rubric:: Footnotes

.. [#f1] I won't pretend to understand all that is going on here, it does work however.
.. [#f2] What I don't understand is why putting this code in the ``SkipList_new`` function does not work, the lock is not initialised and segfaults in ``_pthread_mutex_check_init``. The order has to be: set the lock pointer NULL in ``_new``, allocate it in ``_init``.
