.. moduleauthor:: Paul Ross <apaulross@gmail.com>
.. sectionauthor:: Paul Ross <apaulross@gmail.com>

.. raw:: latex

    \pagebreak

.. _memory-leaks.trace_allocs:

.. index::
    single: Tracing Allocations
    single: Memory Leaks; Tracing Allocations

======================================================
A Simple Way of Tracing Allocations and De-allocations
======================================================

Here is a simple way of instrumenting a CPython module to trace the allocations and de-allocations of
objects in that module.

A Tracking Class
======================================================

First the class declaration, this keeps a record of where each ``PyObject`` was allocated,
it also hold some allocation summary statistics.
The declaration and definition are in ``src/cpy/MemLeaks/TrackAllocs.h``
and ``src/cpy/MemLeaks/TrackAllocs.cpp``.

First we have a simple struct to record the place of allocation:

.. code-block:: c++

    /** POD class that contains the location of the new instance creation. */
    struct NewAndDeallocTrackedValue {
        std::string function;
        std::string file;
        int line;
    };

Then the tracker, essentially this contains a map of ``<PyObject *, struct NewAndDeallocTrackedValue>`` of
all the currently live objects along with some overall statistics:

.. code-block:: c++

    class NewAndDeallocTracker {
    public:
        /** Add a new instance of an object to the map. */
        int add_new(PyObject *op, const char *function, const char *file, int line);

        /** Remove an instance of an object from the map. */
        int add_dealloc(PyObject *op);

        /** Returns a summary string of all the currently live objects. */
        std::string dump_remaining();

        /** Access methods. */
        /** Return the total number of currently live objects. */
        size_t len() const { return m_tracker_map.size(); }

        /** Return the total number of new objects ever created. */
        size_t total_new() const { return m_total_new; }

        /** Return the total tp_basicsize of new objects ever created. */
        size_t total_tp_basicsize() const { return m_total_tp_basicsize; }

        /** Return the total number of objects ever de-allocated. */
        size_t total_dealloc() const { return m_total_dealloc; }

        /** Return the maximum number of allocations at any time. */
        size_t max_allocs() const { return m_max_allocs; }

    protected:
        std::map<PyObject *, struct NewAndDeallocTrackedValue> m_tracker_map;
        size_t m_total_new = 0;
        size_t m_total_tp_basicsize = 0;
        size_t m_total_dealloc = 0;
        size_t m_max_allocs = 0;
    };

The implementation of ``add_new()`` is:

.. code-block:: c++

    int NewAndDeallocTracker::add_new(
            PyObject *op, const char *function, const char *file, int line
    ) {
        if(!op) {
            return -1;
        }
        if (m_tracker_map.find(op) != m_tracker_map.end()) {
            return -2;
        }
        m_tracker_map[op] = {function, file, line};
        m_total_new++;
        m_total_tp_basicsize += op->ob_type->tp_basicsize;
        if (m_max_allocs < m_tracker_map.size()) {
            m_max_allocs = m_tracker_map.size();
        }
        return 0;
    }

The implementation of ``add_dealloc()`` is as follows:

.. code-block:: c++

    int NewAndDeallocTracker::add_dealloc(PyObject *op) {
        if(!op) {
            return -1;
        }
        if (m_tracker_map.find(op) == m_tracker_map.end()) {
            return -2;
        }
        m_tracker_map.erase(op);
        m_total_dealloc++;
        return 0;
    }

Finally the implementation of ``dump_remaining()`` which creates a string summarising the data:

.. code-block:: c++

    std::string NewAndDeallocTracker::dump_remaining() {
        std::ostringstream os;
        int count = 0;
        os << "NewAndDeallocTracker.dump_remaining() [";
        os << m_tracker_map.size() << "]" << std::endl;
        os << "Count new: " << m_total_new;
        os << " Sum tp_basicsize: " << m_total_tp_basicsize;
        os << " Count dealloc: " << m_total_dealloc << " Max: " << m_max_allocs;
        os << std::endl;
        size_t total_tp_basicsize = 0;
        for (const auto &iter: m_tracker_map) {
            total_tp_basicsize += iter.first->ob_type->tp_basicsize;
        }
        os << "Total tp_basicsize remaining: " << total_tp_basicsize;
        os << std::endl;
        for (const auto &iter: m_tracker_map) {
            os << "[" << std::setw(4) << count << "]";
            os << " PyObject: " << iter.first;
            os << " Type: " << Py_TYPE(iter.first)->tp_name;
            os << " Size: " << Py_TYPE(iter.first)->tp_basicsize;
            os << " From: " << iter.second.function;
            os << " " << iter.second.file;
            os << " " << iter.second.line;
            os << std::endl;
            count++;
        }
        os << "NewAndDeallocTracker.dump_remaining(): DONE";
        return os.str();
    }

Instrumenting a Module
======================================================

As an example in ``src/cpy/MemLeaks/cTrackAllocs.cpp`` a module ``cTrackAllocs`` is defined with a single class
``cTrackAllocs.ObjectWithBytes`` which just contains a ``bytes`` object of a given size.
We want to track all the allocations and de-allocations of this class.
Only the code essential to this task is shown here, the complete code is in ``src/cpy/MemLeaks/cTrackAllocs.cpp``.

Of course, if you have multiple classes in the module the tracker can track all of them.

Adding a Static Allocation Tracker
----------------------------------

First we have a macro ``TRACK_ALLOCS_AND_DEALLOCS`` that allows us to switch this tracking on and off.
If this macro is non-zero then the following code becomes active:

- A statically allocated instance of the ``NewAndDeallocTracker``.
- A couple of module level methods that allow us to extract the information the allocation tracker holds.
- Registering a function that is called when the Python interpreter is torn down.
  This function reports all relevant objects that have not been de-allocated, in other words, leaked.
  See :ref:`memory-leaks.trace_allocs.Py_AtExit`.

Here is that code:

.. code-block:: c++

    #define TRACK_ALLOCS_AND_DEALLOCS 1

    #if TRACK_ALLOCS_AND_DEALLOCS
    #include "TrackAllocs.h"

    static NewAndDeallocTracker s_NewAndDeallocTracker;

    static PyObject *
    cTrackAllocs_dump_remaining(PyObject *Py_UNUSED(module), PyObject *Py_UNUSED(args)) {
        std::string result = s_NewAndDeallocTracker.dump_remaining();
        return PyUnicode_FromStringAndSize(result.c_str(), result.size());

    }

    static PyObject *
    cTrackAllocs_statistics(PyObject *Py_UNUSED(module), PyObject *Py_UNUSED(args)) {
        return Py_BuildValue(
                "nnnn",
                s_NewAndDeallocTracker.total_new(),
                s_NewAndDeallocTracker.total_tp_basicsize(),
                s_NewAndDeallocTracker.total_dealloc(),
                s_NewAndDeallocTracker.max_allocs()
                );
    }
    #endif

These are conditionally included in the module method table:

.. code-block:: c

    static PyMethodDef cTrackAllocs_functions[] = {
    #if TRACK_ALLOCS_AND_DEALLOCS
        {
            "dump_remaining", (PyCFunction) cTrackAllocs_dump_remaining, METH_NOARGS,
             PyDoc_STR("Returns a string describing the remaining allocations.")
        },
        {
            "statistics", (PyCFunction) cTrackAllocs_statistics, METH_NOARGS,
             PyDoc_STR(
                "A tuple of (total_new, total_tp_basicsize, total_dealloc, max_allocs)."
             )
        },
    #endif
            /* Other module level functions here... */
            {NULL, NULL, 0, NULL}           /* sentinel */
    };

Instrumenting the ``__new__`` Method on the Class
-------------------------------------------------

Here is the code for the ``__new__`` method implementation.
Conditionally it calls ``s_NewAndDeallocTracker.add_new()`` depending on ``TRACK_ALLOCS_AND_DEALLOCS``.
This function is mapped to the ``.tp_new = ObjectWithBytes_new,`` slot.
Here it is:

.. code-block:: c

    static PyObject *
    ObjectWithBytes_new(
            PyTypeObject *type, PyObject *Py_UNUSED(args), PyObject *Py_UNUSED(kwds)
    ) {
        ObjectWithBytes *self;
        self = (ObjectWithBytes *) type->tp_alloc(type, 0);
        if (self == NULL) {
            return NULL;
        }
        self->pBytes = NULL;
    #if TRACK_ALLOCS_AND_DEALLOCS
        s_NewAndDeallocTracker.add_new((PyObject*)self, __FUNCTION__, __FILE__, __LINE__);
    #endif
        return (PyObject *) self;
    }

Instrumenting the De-allocation Method on the Class
---------------------------------------------------

And the de-allocation method when the object is destroyed, conditionally it calls
``s_NewAndDeallocTracker.add_dealloc()`` depending on ``TRACK_ALLOCS_AND_DEALLOCS``.
This function is mapped to the ``.tp_dealloc = (destructor) ObjectWithBytes_dealloc,`` slot.
Here it is:

.. code-block:: c

    static void
    ObjectWithBytes_dealloc(ObjectWithBytes *self) {
    #if TRACK_ALLOCS_AND_DEALLOCS
        s_NewAndDeallocTracker.add_dealloc((PyObject*)self);
    #endif
        Py_XDECREF(self->pBytes);
        PyObject_Del(self);
    }

Test Example
======================================================

There is some code in ``tests/unit/test_c_track_allocs.py`` that exercises this module.
In this example we create a list of these objects of varying sizes and we print out the
summary information from the tracker after each ``append()``:

.. code-block:: python

    from cPyExtPatt import cTrackAllocs

    list_objects = []
    for size in (8, 4, 7):
        list_objects.append(cTrackAllocs.ObjectWithBytes(length=size))
        print()
        print(cTrackAllocs.dump_remaining())

.. raw:: latex

    [Continued on the next page]

    \pagebreak
    \begin{landscape}

And this might produce the output (lightly edited):

.. code-block:: text

    NewAndDeallocTracker.dump_remaining() [1]
    Count new: 1 Sum tp_basicsize: 24 Count dealloc: 0 Max: 1
    Total tp_basicsize remaining: 24
    [   0] PyObject: 0x10e550c70 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    NewAndDeallocTracker.dump_remaining(): DONE

    NewAndDeallocTracker.dump_remaining() [2]
    Count new: 2 Sum tp_basicsize: 48 Count dealloc: 0 Max: 2
    Total tp_basicsize remaining: 48
    [   0] PyObject: 0x10e550b30 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    [   1] PyObject: 0x10e550c70 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    NewAndDeallocTracker.dump_remaining(): DONE

    NewAndDeallocTracker.dump_remaining() [3]
    Count new: 3 Sum tp_basicsize: 72 Count dealloc: 0 Max: 3
    Total tp_basicsize remaining: 72
    [   0] PyObject: 0x10e550b30 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    [   1] PyObject: 0x10e550c70 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    [   2] PyObject: 0x10e550db0 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    NewAndDeallocTracker.dump_remaining(): DONE

.. note:: The result of ``dump_remaining()`` is in address order, not the order of the objects in ``list_objects``.

.. raw:: latex

    \end{landscape}

If we now remove objects:

.. code-block:: python

    print(cTrackAllocs.dump_remaining())
    while len(list_objects):
        list_objects.pop()
        print()
        print(cTrackAllocs.dump_remaining())

.. raw:: latex

    [Continued on the next page]

    \pagebreak
    \begin{landscape}

We would get something like:

.. code-block:: text

    NewAndDeallocTracker.dump_remaining() [3]
    Count new: 3 Sum tp_basicsize: 72 Count dealloc: 0 Max: 3
    Total tp_basicsize remaining: 72
    [   0] PyObject: 0x10e550b30 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    [   1] PyObject: 0x10e550c70 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    [   2] PyObject: 0x10e550db0 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    NewAndDeallocTracker.dump_remaining(): DONE

    NewAndDeallocTracker.dump_remaining() [2]
    Count new: 3 Sum tp_basicsize: 72 Count dealloc: 1 Max: 3
    Total tp_basicsize remaining: 48
    [   0] PyObject: 0x10e550b30 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    [   1] PyObject: 0x10e550c70 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    NewAndDeallocTracker.dump_remaining(): DONE

    NewAndDeallocTracker.dump_remaining() [1]
    Count new: 3 Sum tp_basicsize: 72 Count dealloc: 2 Max: 3
    Total tp_basicsize remaining: 24
    [   0] PyObject: 0x10e550c70 Type: cTrackAllocs.ObjectWithBytes Size: 24 From: ObjectWithBytes_new cTrackAllocs.cpp 61
    NewAndDeallocTracker.dump_remaining(): DONE

    NewAndDeallocTracker.dump_remaining() [0]
    Count new: 3 Sum tp_basicsize: 72 Count dealloc: 3 Max: 3
    Total tp_basicsize remaining: 0
    NewAndDeallocTracker.dump_remaining(): DONE

.. raw:: latex

    \end{landscape}

Of course this allocation tracker is a basic example, you can make them as sophisticated as you wish.
The main disadvantages are:

- Code clutter.
- If there are memory leaks this will identify where allocations were made with out the corresponding de-allocation.
  That helps somewhat but it does not tell you where that pesky ``Py_DECREF`` *actually* should be!

.. _Py_AtExit(): https://docs.python.org/3/c-api/sys.html#c.Py_AtExit

.. _memory-leaks.trace_allocs.Py_AtExit:

.. index::
    single: Py_AtExit()
    single: Debugging; Py_AtExit()
    single: Memory Leaks; Py_AtExit()

Using ``Py_AtExit``
==========================

This tracing can be of great use when using `Py_AtExit()`_ when it can dump the allocations of objects that have
not been de-allocated.

If we add the following function to our module code at ``src/cpy/MemLeaks/cTrackAllocs.cpp``:

.. code-block:: c

    #if TRACK_ALLOCS_AND_DEALLOCS
    /* Other stuff here, see above. */

    /**
     * A Function that can be registered with Py_AtExit() that will dump the tracker state
     * when the Python interpreter is torn down.
     * NOTE: This should not call any CPython APIs as the interpreter is in an uncertain state.
     */
    static void cTrackAllocs_dump_remaining_atexit(void) {
        std::cout << __FUNCTION__ << "() AT EXIT START:" << std::endl;
        std::cout << "File: " << __FILE__ << " Line: " << __LINE__ << std::endl;
        std::cout << s_NewAndDeallocTracker.dump_remaining() << std::endl;
        std::cout << __FUNCTION__ << "() AT EXIT DONE" << std::endl;
    }

    #endif

Then register this function when creating (importing) the module:

.. code-block:: c

    PyMODINIT_FUNC
    PyInit_cTrackAllocs(void) {
        /* Other stuff here, see above. */

        #if TRACK_ALLOCS_AND_DEALLOCS
            if (Py_AtExit(&cTrackAllocs_dump_remaining_atexit)) {
                goto fail;
            }
        #endif

        /* Other stuff here, see above. */
    }

.. note::

     `Py_AtExit()`_ can only register 32 functions.

Then when the interpreter exits we will see something like this:

.. code-block:: text

    cTrackAllocs_dump_remaining_atexit() AT EXIT START:
    File: src/cpy/MemLeaks/cTrackAllocs.cpp Line: 38
    NewAndDeallocTracker.dump_remaining() [0]
    Count new: 6 Sum tp_basicsize: 144 Count dealloc: 6 Max: 3
    Total tp_basicsize remaining: 0
    NewAndDeallocTracker.dump_remaining(): DONE
    cTrackAllocs_dump_remaining_atexit() AT EXIT DONE

Any live objects will be listed here, these a re worthy if inspection as the have not been de-allocated
in the normal way.
