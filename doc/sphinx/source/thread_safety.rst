.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

.. index::
    single: Thread Safety

*******************************
Thread Safety
*******************************

This chapter describes various issues when C extensions interact with Python threads [#f1]_.

====================================
When You Need a Lock
====================================

If your Extension is likely to be exposed to a multi-threaded environment then you need to think about thread safety.
I had this problem in a separate project which was a C++ `SkipList <https://github.com/paulross/skiplist/>`_
which could contain an ordered list of arbitrary Python objects.

The problem in a multi-threaded environment that was sharing the same structure was that the following sequence of
events could happen:

* Thread A tries to insert a Python object into the SkipList.
  The C++ code searches for a place to insert it preserving the existing order.
  To do so it must call back into Python code for the user defined comparison function
  (using ``functools.total_ordering`` for example).
* At this point the Python interpreter is free to make a context switch allowing thread B to, say, remove an element
  from the same SkipList. This removal may well invalidate C++ pointers held by thread A.
* When the interpreter switches back to thread A it accesses an invalid pointer and a segfault happens.

The solution is to use a lock to prevent a Python context switch until A has completed its insertion, but how?

I found the existing Python documentation misleading and I couldn't get it to work reliably, if at all.
It was only when I stumbled upon the `source code <https://github.com/python/cpython/blob/master/Modules/_bz2module.c>`_
for the `bz module <https://docs.python.org/3/library/bz2.html#module-bz2>`_ that I realised there was a whole other,
low level way of doing this, largely undocumented.

Here is a version that concentrates on those essentials.
As an example, here is a subclass of a list that has a ``max()`` method that returns the maximum value in the list.
To do the comparison it must call
`PyObject_RichCompareBool <https://docs.python.org/3/c-api/object.html#c.PyObject_RichCompareBool>`_
to decide which of two objects is the maximum.

So during that call to ``max()`` the Python interpreter is free too switch to another thread that might alter the
list we are inspecting.
What we need to do is to block that thread with a lock so that can't happen.
Then once the result of ``max()`` is known we can relase that lock.
This class deliberately has ``sleep()`` calls to allow a thread switch to take place.

The code (C and C++) is in ``src/cpy/Threads`` and the tests are in ``tests/unit/test_c_threads.py``.

Lets walk through it.

.. index::
    single: Thread Safety; Creating a Lock

====================================
Coding up the Lock
====================================

First we need to include `pythread.h <https://github.com/python/cpython/blob/master/Include/pythread.h>`_
as well as the usual includes:

.. code-block:: c
    :emphasize-lines: 4-6

    #include <Python.h>
    #include "structmember.h"
    
    #ifdef WITH_THREAD
    #include "pythread.h"
    #endif

.. note::

    Your Python may have been compiled without thread support in which case we don't have to concern ourselves with
    thread locking.
    We can discover this from the presence of the macro ``WITH_THREAD`` so all our thread support code is conditional
    on the definition of this macro.

--------------------------------------------------
Adding a ``PyThread_type_lock`` to our object
--------------------------------------------------

Then we add a ``PyThread_type_lock`` (an opaque pointer) to the Python structure we are intending to protect.
Here is the object declaration:

.. code-block:: c

    typedef struct {
        PyListObject list;
    #ifdef WITH_THREAD
        PyThread_type_lock lock;
    #endif
    } SubListObject;

--------------------------------------------------
Initialising and Deallocating the Lock
--------------------------------------------------

If you have a ``__new__`` method then set the lock pointer to ``NULL``.
The lock needs to be initialised only in the ``__init__`` method.
In the ``__init__`` method we allocate the lock by calling ``PyThread_allocate_lock()`` [#f2]_:

.. code-block:: c

    static int
    SubList_init(SubListObject *self, PyObject *args, PyObject *kwds) {
        if (PyList_Type.tp_init((PyObject *) self, args, kwds) < 0) {
            return -1;
        }
    #ifdef WITH_THREAD
        self->lock = PyThread_allocate_lock();
        if (self->lock == NULL) {
            PyErr_SetString(PyExc_MemoryError, "Unable to allocate thread lock.");
            return -2;
        }
    #endif
        return 0;
    }

When deallocating the object we should free the lock pointer with ``PyThread_free_lock`` [#f3]_:

.. code-block:: c

    static void
    SubList_dealloc(SubListObject *self) {
        /* Deallocate other fields here. */
        #ifdef WITH_THREAD
            if (self->lock) {
                PyThread_free_lock(self->lock);
                self->lock = NULL:
            }
        #endif
        Py_TYPE(self)->tp_free((PyObject *)self);
    }

.. index::
    single: Thread Safety; Using a Lock

====================================
Using the Lock
====================================

So now our object has a lock but we need to acquire it and release it.

-------------------------------------
From C Code
-------------------------------------

It is useful to declare a couple of macros.
These are from the `bz module <https://docs.python.org/3/library/bz2.html#module-bz2>`_:

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

Before any critical section we need to use ``ACQUIRE_LOCK(self);`` (which blocks) then ``RELEASE_LOCK(self);``
when done.
Failure to call ``RELEASE_LOCK(self);`` in any code path *will* lead to deadlocking.

Example
-------------------------------------

Here is an example of out sublist ``append()``.

In the body of the function it makes a ``super()`` call and then introduces a ``sleep()`` which allows the
Python interpreter to switch threads (it should not because of the lock).

.. code-block:: c

    static PyObject *
    SubList_append(SubListObject *self, PyObject *args) {
        ACQUIRE_LOCK(self);
        PyObject *result = call_super_name(
                (PyObject *) self, "append", args, NULL
        );
        // 0.25s delay to demonstrate holding on to the thread.
        sleep_milliseconds(250L);
        RELEASE_LOCK(self);
        return result;
    }

-------------------------------------
From C++ Code
-------------------------------------

We can make this a little smoother in C++ by creating a class that will lock and unlock.

Creating a class to Acquire and Release the Lock
----------------------------------------------------

We can acquire and release the lock in a RAII fashion in C++ where the constructor blocks until the lock is acquired
and the destructor releases the lock.
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

Using the AcquireLock class
----------------------------------------------------

Before any critical section we create an ``AcquireLock`` object which blocks until we have the lock.
Once the lock is obtained we can make any calls, including calls into the Python interpreter without preemption.
The lock is automatically freed when we exit the code block:

.. code-block:: c

    /** append with a thread lock. */
    static PyObject *
    SubList_append(SubListObject *self, PyObject *args) {
        AcquireLock<SubListObject> local_lock((SubListObject *)self);
        PyObject *result = call_super_name(
                (PyObject *) self, "append", args, NULL
        );
        // 0.25s delay to demonstrate holding on to the thread.
        sleep_milliseconds(250L);
        return result;
    }

.. index::
    single: Thread Safety; Examples

====================================
Example Code and Tests
====================================

The code (C and C++) is in ``src/cpy/Threads`` and the tests are in ``tests/unit/test_c_threads.py``.

----------------------------------------------------
Example Code
----------------------------------------------------

The example code is here:

- ``C``: ``src/cpy/Threads/csublist.c``
- ``C++``: ``src/cpy/Threads/cThreadLock.h`` and ``src/cpy/Threads/cppsublist.cpp``.

``setup.py`` creates two extensions; ``cPyExtPatt.Threads.csublist`` (in C) and ``cPyExtPatt.Threads.cppsublist``
(in C++):

.. code-block:: python

        Extension(name=f"{PACKAGE_NAME}.Threads.csublist",
                  include_dirs=[
                      '/usr/local/include',
                      'src/cpy/Util',
                      "src/cpy/Threads",
                  ],
                  sources=[
                      "src/cpy/Threads/csublist.c",
                      'src/cpy/Util/py_call_super.c',
                  ],
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),
        Extension(name=f"{PACKAGE_NAME}.Threads.cppsublist",
                  include_dirs=[
                      '/usr/local/include',
                      'src/cpy/Util',
                      "src/cpy/Threads",
                  ],
                  sources=[
                      "src/cpy/Threads/cppsublist.cpp",
                      'src/cpy/Util/py_call_super.c',
                  ],
                  language='c++11',
                  ),


The individual C and C++ modules can be accessed with:

.. code-block:: python

    from cPyExtPatt.Threads import cppsublist
    from cPyExtPatt.Threads import csublist


----------------------------------------------------
Example Tests
----------------------------------------------------

The tests are in ``tests/unit/test_c_threads.py``.
Here are some examples:

Tests in C
----------------------------------------------------

First create two function to call ``max()`` and ``append()``.
These functions print out their progress and which thread they are running in:

.. code-block:: python

    def csublist_max(obj, count):
        print(
            f'sublist_max(): Thread name {threading.current_thread().name}',
            flush=True
        )
        for _i in range(count):
            print(
                f'sublist_max(): Thread name {threading.current_thread().name}'
                f' Result: {obj.max()}',
                flush=True
            )
            time.sleep(0.25)
        print(
            f'sublist_max(): Thread name {threading.current_thread().name} DONE',
            flush=True
        )


    def csublist_append(obj, count):
        print(
            f'sublist_append(): Thread name {threading.current_thread().name}',
            flush=True
        )
        for _i in range(count):
            print(
                f'sublist_append(): Thread name {threading.current_thread().name}',
                flush=True
            )
            obj.append(len(obj))
            time.sleep(0.25)
        print(
            f'sublist_append(): Thread name {threading.current_thread().name} DONE',
            flush=True
        )


Now a test that creates a single shared sub-list and four threads for each of the ``max()`` and ``append()``
functions:

.. code-block:: python

    def test_threaded_c():
        print()
        print('test_threaded_c() START', flush=True)
        obj = csublist.cSubList(range(128))
        threads = []
        for i in range(4):
            threads.append(
                threading.Thread(
                    name=f'sublist_max[{i:2d}]',
                    target=csublist_max,
                    args=(obj, 2),
                )
            )
            threads.append(
                threading.Thread(
                    name=f'sublist_append[{i:2d}]',
                    target=csublist_append,
                    args=(obj, 2),
                )
            )
        for thread in threads:
            thread.start()
        print('Waiting for worker threads', flush=True)
        main_thread = threading.current_thread()
        for t in threading.enumerate():
            if t is not main_thread:
                t.join()
        print('Worker threads DONE', flush=True)

Running this test gives this output, typically:

.. code-block:: text

    test_threaded_c() START
    sublist_max(): Thread name sublist_max[ 0]
    sublist_append(): Thread name sublist_append[ 0]
    sublist_append(): Thread name sublist_append[ 0]
    sublist_max(): Thread name sublist_max[ 1]
    sublist_append(): Thread name sublist_append[ 1]
    sublist_max(): Thread name sublist_max[ 2]
    sublist_max(): Thread name sublist_max[ 1] Result: 127
    sublist_append(): Thread name sublist_append[ 2]
    sublist_max(): Thread name sublist_max[ 3]
    sublist_append(): Thread name sublist_append[ 3]
    Waiting for worker threads
    sublist_append(): Thread name sublist_append[ 1]
    sublist_max(): Thread name sublist_max[ 0] Result: 128
    sublist_max(): Thread name sublist_max[ 3] Result: 128
    sublist_append(): Thread name sublist_append[ 2]
    sublist_append(): Thread name sublist_append[ 3]
    sublist_append(): Thread name sublist_append[ 0]
    sublist_max(): Thread name sublist_max[ 2] Result: 128
    sublist_append(): Thread name sublist_append[ 2]
    sublist_append(): Thread name sublist_append[ 1]
    sublist_max(): Thread name sublist_max[ 1] Result: 131
    sublist_max(): Thread name sublist_max[ 0] Result: 132
    sublist_append(): Thread name sublist_append[ 0] DONE
    sublist_max(): Thread name sublist_max[ 1] DONE
    sublist_max(): Thread name sublist_max[ 3] Result: 134
    sublist_append(): Thread name sublist_append[ 3]
    sublist_append(): Thread name sublist_append[ 1] DONE
    sublist_max(): Thread name sublist_max[ 2] Result: 134
    sublist_max(): Thread name sublist_max[ 0] DONE
    sublist_append(): Thread name sublist_append[ 2] DONE
    sublist_max(): Thread name sublist_max[ 3] DONE
    sublist_append(): Thread name sublist_append[ 3] DONE
    sublist_max(): Thread name sublist_max[ 2] DONE
    Worker threads DONE

Tests in C++
----------------------------------------------------

A very similar example in C++ is in ``tests/unit/test_c_threads.py``.

.. rubric:: Footnotes

.. [#f1] I don't cover 'pure' C threads (those that do not ``#include <Python.h>``) here as they are not relevant.
   If your C extension code creates/calls pure C threads this does not affect the CPython state for the *current*
   thread.
   Of course *that* C thread could embed another Python interpreter but that would have an entirely different state
   than the current interpreter.

.. [#f2] The order has to be: set the lock pointer NULL in ``_new``, allocate it in ``_init``, free it in ``_dealloc``.
   If you don't do this then the lock does not get initialised and segfaults, typically in
   ``_pthread_mutex_check_init``.

.. [#f3] A potential weakness of this code is that we might be deallocating the lock *whilst the lock is acquired*
   which could lead to deadlock.

   This is very much implementation defined in ``pythreads`` and may vary from platform to platform.
   There is no obvious API in ``pythreads`` that allows us to determine if a lock is held so we can release it before
   deallocation.

   I notice that in the Python threading module (*Modules/_threadmodule.c*) there is an additional ``char`` field that
   acts as a flag to say when the lock is held so that the ``lock_dealloc()`` function in that module can release the
   lock before freeing the lock.
