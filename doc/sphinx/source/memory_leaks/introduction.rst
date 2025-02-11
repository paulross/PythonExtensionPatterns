Introduction
====================

This describes tools and techniques that can identify memory leaks in Long running Python programs.

Is it a Leak?
------------------

Rising memory is not necessarily a leak, it can be every internal data structures that grow naturally.
A common strategy for hash tables, arrays and the like is that when they reach capacity they reallocate themselves
into twice the original memory and this can look, superficially like a memory leak.

Python data structures are not particularly efficient, an ``int`` is typically 24 bytes, a ``datetime`` 48 bytes and so on.

A further source of 'leaks' are caches.

Sources of Leaks
------------------

Here is a non-exhaustive list in rough order of popularity:

* Classic C/C++ leaks:
    * ``malloc`` without corresponding ``free``.
    * ``new`` without corresponding ``delete``.
    * Static data structures that use ever increasing heap storage.
* Reference counting errors in C/C++ extensions used by Python.
* Bugs in C/C++ wrappers such as Cython or pybind11.
* Bugs in Python.


A Bit About (C)Python Memory Management
------------------------------------------

Python objects are allocated on the heap with their parent references on the stack.
When the stack unwinds the reference goes out of scope and, without any other action, the heap allocated would be leaked.
Python uses a couple of techniques to prevent this; reference counting and Garbage Collection.
Bear in mind that Python is quite old and the Garbage Collector reflects that.

Reference Counts
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The primary technique that Python uses is reference counting, when an object is created it is given a reference count of 1.
When another object refers to it the reference count increases by one.
When an object goes out of scope that refers to the object the reference count is decremented by one.
When the reference count becomes zero the object is de-allocated and the memory can be re-used immediately.

In C/C++ extensions you have to manage these reference counts manually and correctly.
An important point about reference counts in C/C++ extensions is that:

* If they are too low the object might get de-allocated prematurely whilst there are still valid references. Then those
  references might try to access the deleted object and that may, or may not, result in a segfault.
* If the reference count is incremented unnecessarily the object will never get de-allocated and there will be a memory leak.

The latter is often regarded as the lesser of the two problems and the temptation is to err on the side of increasing
reference counts for 'safety'.
This swaps an easy to solve probem (segfault) for a harder to solve one (memory leak).

You can find the reference count of any object by calling :py:func:`sys.getrefcount()` with the Python object as the argument.
The count is one higher than you might expect as it includes the (temporary) reference to the :py:func:`sys.getrefcount()`.

Reference counting is always switched on in Python.

.. note::

    Some objects are *interned*, that is their reference count never goes to zero so that they are, in effect, permanent.
    This is done for performance and includes most builtins and the integers -5 to 255.

    For example:

    .. code-block:: python

        >>> sys.getrefcount(None)
        14254
        >>> sys.getrefcount(0)
        2777
        >>> sys.getrefcount(400)
        2

Reference counts have one major problem, cyclic references. Consider this:

.. code-block:: python

    class A: pass
    a = A()
    b = A()
    a.next = b
    b.next = a

``a`` references ``b`` and ``b`` references ``a`` so you can not delete either without deleting the other.
To get round this problem Python uses a simple garbage collector.

Garbage Collection
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The only job of Python's garbage collector (GC) is to discover unreachable objects that have cyclic references.
The Python garbage collector is fairly simple and rather old.
You can use the :py:mod:`gc` module to inspect and control the garbage collector.
The garbage collector can be switched off and this is often done in high performance systems.

In particular:

* The GC will not reclaim objects that are not tracked.
  This includes many objects created in C/C++ extensions.
  See :py:func:`gc.is_tracked` to see if an object is being tracked by the GC.
* The GC only looks at unreachable objects.
* The GC only deals with cyclic references.
* The GC is easily defeated, even inadvertently, for example if objects implement ``__del__``.
* A real restriction on the GC is due to C/C++ extensions.]
  An unreachable C/C++ object from Python code with a zero reference count can not be deleted as there is no way of
  knowing if some C/C++ code might have a reference to it.
  In Java this is easier as the VM controls the whole estate and can safely delete unreachable objects.


The Big Picture
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Here is a visualisation of memory allocators from top to bottom (from the Python source ``Objects/obmalloc.c``):

.. code-block:: text

        _____   ______   ______       ________
       [ int ] [ dict ] [ list ] ... [ string ]       Python core         |
    +3 | <----- Object-specific memory -----> | <-- Non-object memory --> |
        _______________________________       |                           |
       [   Python's object allocator   ]      |                           |
    +2 | ####### Object memory ####### | <------ Internal buffers ------> |
        ______________________________________________________________    |
       [          Python's raw memory allocator (PyMem_ API)          ]   |
    +1 | <----- Python memory (under PyMem manager's control) ------> |   |
        __________________________________________________________________
       [    Underlying general-purpose allocator (ex: C library malloc)   ]
     0 | <------ Virtual memory allocated for the python process -------> |

       =========================================================================
        _______________________________________________________________________
       [                OS-specific Virtual Memory Manager (VMM)               ]
    -1 | <--- Kernel dynamic storage allocation & management (page-based) ---> |
        __________________________________   __________________________________
       [                                  ] [                                  ]
    -2 | <-- Physical memory: ROM/RAM --> | | <-- Secondary storage (swap) --> |


Layer +2 is significant, it is the CPython's Object Allocator (``pymalloc``).

CPython's Object Allocator (``pymalloc``)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Standard CPython uses an in-memory pool for small items (<=512 bytes) to reduce the cost of going to the OS for memory allocations.
One consequence of this is that small memory leaks will be hidden when observing the *overall* memory usage of a precess.
Another consequence is that tools such as Valgrind are rendered nearly useless for detecting memory leaks when the ``pymalloc`` is in use.
``pymalloc`` can be disabled with a special build of Python.
Requests >512 bytes are allocated without ``pymalloc`` and routed to the platform's allocator (usually the C ``malloc()`` function).

A summary of ``pymalloc``:

* ``pymalloc`` consists of a set of *Arena*'s.
* An *Arena* is a 256kB (262,144 bytes) chunk of memory divided up into *Pool*'s.
* A *Pool* is a chunk of memory the size of a OS page, usually 4096 bytes.
* A *Pool* is subdivided into *Block*'s which all have the same size for that Pool.
* A *Block* is memory sized between 8 and 512 (modulo 8).

To understand this better try:

.. code-block:: python

    import sys
    sys._debugmallocstats()

An you will get something like:

.. code-block:: text

    Small block threshold = 512, in 64 size classes.

    class   size   num pools   blocks in use  avail blocks
    -----   ----   ---------   -------------  ------------
        0      8           2             551           461
        1     16           1              82           171
        2     24           2             186           150
    ...
       62    504          10              73             7
       63    512          19             132             1

    # arenas allocated total           =                   95
    # arenas reclaimed                 =                   46
    # arenas highwater mark            =                   49
    # arenas allocated current         =                   49
    49 arenas * 262144 bytes/arena     =           12,845,056

    # bytes in allocated blocks        =           12,129,080
    # bytes in available blocks        =              174,784
    59 unused pools * 4096 bytes       =              241,664
    # bytes lost to pool headers       =              147,696
    # bytes lost to quantization       =              151,832
    # bytes lost to arena alignment    =                    0
    Total                              =           12,845,056

          14 free PyCFunctionObjects * 48 bytes each =                  672
               78 free PyDictObjects * 48 bytes each =                3,744
               7 free PyFloatObjects * 24 bytes each =                  168
              3 free PyFrameObjects * 384 bytes each =                1,152
               80 free PyListObjects * 40 bytes each =                3,200
             17 free PyMethodObjects * 40 bytes each =                  680
      25 free 1-sized PyTupleObjects * 32 bytes each =                  800
    1446 free 2-sized PyTupleObjects * 40 bytes each =               57,840
    ...
     1 free 19-sized PyTupleObjects * 176 bytes each =                  176

There are five sections:

* The first line states the small object limit (512) and how this is divided: 512 / 8 = 64 'class's.
  Each of these 'class's handle memory allocations of a specific size.
* The second section shows how many pools and blocks are in use for each 'class' (specific size of memory allocation).
* The third section is about *Arena*'s, there are currently 49 at 262,144 bytes each.
* The fourth section summarises the total memory usage, in particular the amount of memory consumed by the ``pymalloc`` administration.
* The fifth section is a summary of the memory consumed by particular Python type.
  NOTE: This is not an exclusive list, many types such as ``int``, ``set`` are absent.

In summary:

.. code-block:: text

    - 49 Arenas of 256kB (262,144 bytes) is 12,845,056 in total.
        - Each Arena is divided into 64 pools of 4096 bytes each,
          thus 49 x 64 = 3136 pools (the sum of 'num pools') above.
            - Each Pool of 4096 bytes is allocated a fixed size
              between 8 and 512 bytes and divided by that into Blocks.
              So there are between 512 x 8 byte blocks and 8 x 512 byte blocks in a Pool.

See :ref:`examples-debug_malloc_stats` for examples of ``pymemtrace.debug_malloc_stats`` that can make this information
much more useful.

Memory De-allocation
"""""""""""""""""""""

* If the object is >512 bytes it is not under control of ``pymalloc`` and the memory is returned to the OS immediately.
* A *Pool* is free'd when all the blocks are empty.
* An *Arena* is free'd when all the *Pool*'s are empty.
* There is no attempt to reorganise ``pymalloc`` periodically reduce the memory use such as a copying garbage collector might do.

This means that pools and arenas can exist for a very long time.
