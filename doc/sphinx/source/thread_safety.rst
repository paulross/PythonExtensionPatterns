.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

====================================
Thread Safety
====================================

If your Extension is likely to be exposed to a multi-threaded environment then you need to think about thread safety.
I had this problem in a separate project which was a C++ `SkipList <https://github.com/paulross/skiplist/>`_
which could contain an ordered list of arbitrary Python objects.
The problem in a multi-threaded environment was that the following sequence of events could happen:

* Thread A tries to insert a Python object into the SkipList.
  The C++ code searches for a place to insert it preserving the existing order.
  To do so it must call back into Python code for the user defined comparison function
  (using ``functools.total_ordering`` for example).
* At this point the Python interpreter is free to make a context switch allowing thread B to, say, remove an element
  from the SkipList. This removal may well invalidate C++ pointers held by thread A.
* When the interpreter switches back to thread A it accesses an invalid pointer and a segfault happens.

The solution is to use a lock to prevent a Python context switch until A has completed its insertion, but how?

I found the existing Python documentation misleading and I couldn't get it to work reliably, if at all.
It was only when I stumbled upon the `source code <https://github.com/python/cpython/blob/master/Modules/_bz2module.c>`_
for the `bz module <https://docs.python.org/3/library/bz2.html#module-bz2>`_ that I realised there was a whole other,
low level way of doing this, largely undocumented.

Here is a version that concentrates on those essentials.
As an example here is a subclass of a list that has a ``max()`` method that returns the maximum value in the list.
To do the comparison it must call
`PyObject_RichCompareBool <https://docs.python.org/3/c-api/object.html#c.PyObject_RichCompareBool>`_
to decide which of two objects is the maximum.
This class deliberately has ``sleep()`` calls to allow a thread switch to take place.
So during that call the Python interpreter is free too switch to another thread that might alter the list we are
inspecting.
What we need to do is to block that thread with a lock so that can't happen.
Then once the result of ``max()`` is known we can relases that lock.

Lets walk through it.

.. note::

    Your Python may have been compiled without thread support in which case we don't have to concern ourselves with
    thread locking.
    We can discover this from the presence of the macro ``WITH_THREAD`` so all our thread support code is conditional
    on the definition of this macro.

Coding up the Lock
----------------------------

First we need to include `pythread.h <https://github.com/python/cpython/blob/master/Include/pythread.h>`_
as well as the usual includes:

.. code-block:: c
    :emphasize-lines: 4-6

    #include <Python.h>
    #include "structmember.h"
    
    #ifdef WITH_THREAD
    #include "pythread.h"
    #endif

Adding a ``PyThread_type_lock`` to our object
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Then we add a ``PyThread_type_lock`` (an opaque pointer) to the Python structure we are intending to protect.
I'll use the example of the
`SkipList source code <https://github.com/paulross/skiplist/blob/master/src/cpy/cSkipList.cpp>`_.
Here is a fragment with the important lines highlighted:

.. code-block:: c

    typedef struct {
        PyListObject list;
    #ifdef WITH_THREAD
        PyThread_type_lock lock;
    #endif
    } SubListObject;



Initialising and Deallocating the Lock
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Now we need to set the lock pointer to ``NULL`` in the ``__new__`` function:

.. code-block:: c
    :linenos:
    :emphasize-lines: 10-12

    static PyObject *
    SkipList_new(PyTypeObject *type, PyObject * /* args */, PyObject * /* kwargs */) {
        SkipList *self = NULL;
        
        self = (SkipList *)type->tp_alloc(type, 0);
        if (self != NULL) {
            /*
             * Initialise other struct SkipList fields...
             */
    #ifdef WITH_THREAD
            self->lock = NULL;
    #endif
        }
        return (PyObject *)self;
    }

In the ``__init__`` method we allocate the lock by calling ``PyThread_allocate_lock()`` [#f1]_ A lot of this code is specific to the SkipList but the lock allocation code is highlighted:

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

When deallocating the object we should free the lock pointer with ``PyThread_free_lock`` [#f2]_:

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
------------------------


The Lock in C
^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

    #define ACQUIRE_LOCK(obj) do { \
        if (!PyThread_acquire_lock((obj)->lock, 0)) { \
            Py_BEGIN_ALLOW_THREADS \
            PyThread_acquire_lock((obj)->lock, 1); \
            Py_END_ALLOW_THREADS \
        } } while (0)
    #define RELEASE_LOCK(obj) PyThread_release_lock((obj)->lock)

The code that acquires the lock is slightly clearer if the
`Py_BEGIN_ALLOW_THREADS <https://docs.python.org/3/c-api/init.html#c.Py_BEGIN_ALLOW_THREADS>`_
and `Py_END_ALLOW_THREADS <https://docs.python.org/3/c-api/init.html#c.Py_END_ALLOW_THREADS>`_
macros are fully expanded:

.. code-block:: c

    if (! PyThread_acquire_lock(_pSL->lock, NOWAIT_LOCK)) {
        {
            PyThreadState *_save;
            _save = PyEval_SaveThread();
            PyThread_acquire_lock(_pSL->lock, WAIT_LOCK);
            PyEval_RestoreThread(_save);
        }
    }







The Lock In C++
^^^^^^^^^^^^^^^^^^^^^^^^


Creating a class to Acquire and Release the Lock
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Now we add some code to acquire and release the lock.
We can do this in a RAII fashion in C++ where the constructor blocks until the lock is acquired and the destructor
releases the lock.
This is a template class for generality.

The code is in ``src/cpy/Threads/cThreadLock.h``

.. code-block:: c++

    #include <Python.h>
    #include "structmember.h"

    #ifdef WITH_THREAD
    #include "pythread.h"
    #endif

    #ifdef WITH_THREAD
        /* A RAII wrapper around the PyThread_type_lock. */
        template<typename T>
        class AcquireLock {
        public:
            AcquireLock(T *pObject) : m_pObject(pObject) {
                assert(m_pObject);
                assert(m_pObject->lock);
                Py_INCREF(m_pObject);
                if (!PyThread_acquire_lock(m_pObject->lock, NOWAIT_LOCK)) {
                    Py_BEGIN_ALLOW_THREADS
                        PyThread_acquire_lock(m_pObject->lock, WAIT_LOCK);
                    Py_END_ALLOW_THREADS
                }
            }
            ~AcquireLock() {
                assert(m_pObject);
                assert(m_pObject->lock);
                PyThread_release_lock(m_pObject->lock);
                Py_DECREF(m_pObject);
            }
        private:
            T *m_pObject;
        };

    #else
        /* Make the class a NOP which should get optimised out. */
        template<typename T>
        class AcquireLock {
        public:
            AcquireLock(T *) {}
        };
    #endif



Before any critical code we create an ``AcquireLock`` object which blocks until we have the lock. Once the lock is obtained we can make any calls, including calls into the Python interpreter without preemption. The lock is automatically freed when we exit the code block:

.. code-block:: c
    :linenos:
    :emphasize-lines: 7,21

    static PyObject *
    SkipList_insert(SkipList *self, PyObject *arg) {
        assert(self && self->pSl_void);
        /* Lots of stuff here...
         */
        {
            AcquireLock _lock(self);
            /* We can make calls here, including calls back into the Python
             * interpreter knowing that the interpreter will not preempt us.
             */
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
        /* More stuff here...
         */
         Py_RETURN_NONE;
    }

And that is pretty much it.

.. rubric:: Footnotes

.. [#f1] The order has to be: set the lock pointer NULL in ``_new``, allocate it in ``_init``, free it in ``_dealloc``.
   If you don't do this then the lock does not get initialised and segfaults typically in ``_pthread_mutex_check_init``.
.. [#f2] A potential weakness of this code is that we might be deallocating the lock *whilst the lock is acquired*
   which could lead to deadlock.

   This is very much implementation defined in ``pythreads`` and may vary from platform to platform.
   There is no obvious API in ``pythreads`` that allows us to determine if a lock is held so we can release it before
   deallocation.

   I notice that in the Python threading module (*Modules/_threadmodule.c*) there is an additional ``char`` field that
   acts as a flag to say when the lock is held so that the ``lock_dealloc()`` function in that module can release the
   lock before freeing the lock.
