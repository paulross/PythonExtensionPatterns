.. highlight:: python
    :linenothreshold: 10

.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _debug-version-of-python-label:

===============================================
Building and Using a Debug Version of Python
===============================================

There is a spectrum of debug builds of Python that you can create. This chapter describes how to create them.


--------------------------------------------
Building a Standard Debug Version of Python
--------------------------------------------

Download and unpack the Python source. Then in the source directory create a debug directory for the debug build:

.. code-block:: bash

    mkdir debug
    cd debug
    ../configure --with-pydebug
    make
    make test

-----------------------
Specifying Macros
-----------------------

They can be specified at the configure stage, this works:

.. code-block:: bash

    ../configure CFLAGS='-DPy_DEBUG -DPy_TRACE_REFS' --with-pydebug
    make


However the python documentation suggests the alternative way of specifying them when invoking make:

.. code-block:: bash

    ../configure --with-pydebug
    make EXTRA_CFLAGS="-DPy_REF_DEBUG"

I don't know why one way would be regarded as better than the other.

---------------------------
The Debug Builds
---------------------------

The builds are controlled by the following macros:

                                                                            
=================== ======================================================= ==============
Macro               Description                                             Must Rebuild
                                                                            Extensions?
=================== ======================================================= ==============
``Py_DEBUG``        A standard debug build. ``Py_DEBUG`` implies            Yes
                    ``LLTRACE``, ``Py_REF_DEBUG``, ``Py_TRACE_REFS``, and
                    ``PYMALLOC_DEBUG`` (if ``WITH_PYMALLOC`` is enabled).
``Py_REF_DEBUG``    Turn on aggregate reference counting which will be      No
                    displayed in the interactive interpreter when
                    invoked with ``-X showrefcount`` on the command line.
                    If you are not keeping references to objects and the
                    count is increasing there is probably a leak.
                    Also adds ``sys.gettotalrefcount()`` to the ``sys``
                    module and this returns the total number of references.
``Py_TRACE_REFS``   Turns on reference tracing.                             Yes
                    Implies ``Py_REF_DEBUG``.
``COUNT_ALLOCS``    Keeps track of the number of objects of each type have  Yes
                    been allocated and how many freed.
                    See: :ref:`debug-version-of-python-COUNT_ALLOCS-label`
``WITH_PYMALLOC``   Enables Pythons small memory allocator. For Valgrind    No
                    this must be disabled, if using Pythons malloc
                    debugger (using ``PYMALLOC_DEBUG``) this must be
                    enabled.
                    See: :ref:`debug-version-of-python-memory_alloc-label`
``PYMALLOC_DEBUG``  Enables Python's malloc debugger that annotates         No
                    memory blocks. Requires ``WITH_PYMALLOC``.
                    See: :ref:`debug-version-of-python-memory_alloc-label`
=================== ======================================================= ==============



In the source directory:

.. code-block:: bash

    mkdir debug
    cd debug
    ../configure --with-pydebug
    make
    make test


.. _debug-version-of-python-memory_alloc-label:

---------------------------
Python's Memory Allocator
---------------------------

A normal build of Python gives CPython a special memory allocator 'PyMalloc'. When enabled this mallocs largish chunks of memory from the OS and then uses this pool for the actual PyObjects. With PyMalloc active Valgrind can not see all allocations and deallocations.

There are two Python builds of interest to help solve memory problems:

* Disable PyMalloc so that Valgrind can analyse the memory usage.
* Enable PyMalloc in debug mode, this creates memory blocks with special bit patterns and adds debugging information on each end of any dynamically allocated memory. This pattern is checked on every alloc/free and if found to be corrupt a diagnostic is printed and the process terminated.

To make a version of Python with its memory allocator suitable for use with Valgrind:

.. code-block:: bash

    ../configure --with-pydebug --without-pymalloc
    make

See :ref:`valgrind-label` for using Valgrind.

To make a version of Python with its memory allocator using Python's malloc debugger either:

.. code-block:: bash

    ../configure CFLAGS='-DPYMALLOC_DEBUG' --with-pydebug
    make

Or:

.. code-block:: bash

    ../configure --with-pydebug
    make EXTRA_CFLAGS="-DPYMALLOC_DEBUG"

This builds Python with the ``WITH_PYMALLOC`` and ``PYMALLOC_DEBUG`` macros defined.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Finding Access after Free With ``PYMALLOC_DEBUG``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Python built with ``PYMALLOC_DEBUG`` is the most effective way of detecting access after free. For example if we have this CPython code:

.. code-block:: c

    static PyObject *access_after_free(PyObject *pModule) {
        PyObject *pA = PyLong_FromLong(1024L);
        Py_DECREF(pA);
        PyObject_Print(pA, stdout, 0);
        Py_RETURN_NONE;
    }

And we call this from the interpreter we get a diagnostic:

.. code-block:: python

    Python 3.4.3 (default, Sep 16 2015, 16:56:10) 
    [GCC 4.2.1 Compatible Apple LLVM 6.0 (clang-600.0.51)] on darwin
    Type "help", "copyright", "credits" or "license" for more information.
    >>> import cPyRefs
    >>> cPyRefs.afterFree()
    <refcnt -2604246222170760229 at 0x10a474130>
    >>> 

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Getting Statistics on PyMalloc
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the environment variable ``PYTHONMALLOCSTATS`` exists when running Python built with ``WITH_PYMALLOC``+``PYMALLOC_DEBUG`` then a (detailed) report of pymalloc activity is output on stderr whenever a new 'arena' is allocated.

.. code-block:: bash

    PYTHONMALLOCSTATS=1 python.exe
    
I have no special knowledge about the output you see when running Python this way which looks like this::

    >>> cPyRefs.leakNewRefs(1000, 10000)
    loose_new_reference: value=1000 count=10000
    Small block threshold = 512, in 64 size classes.

    class   size   num pools   blocks in use  avail blocks
    -----   ----   ---------   -------------  ------------
        4     40           2             139            63
        5     48           1               2            82
      ...
       62    504           3              21             3
       63    512           3              18             3

    # times object malloc called       =            2,042,125
    # arenas allocated total           =                  636
    # arenas reclaimed                 =                    1
    # arenas highwater mark            =                  635
    # arenas allocated current         =                  635
    635 arenas * 262144 bytes/arena    =          166,461,440

    # bytes in allocated blocks        =          162,432,624
    # bytes in available blocks        =              116,824
    0 unused pools * 4096 bytes        =                    0
    # bytes lost to pool headers       =            1,950,720
    # bytes lost to quantization       =            1,961,272
    # bytes lost to arena alignment    =                    0
    Total                              =          166,461,440
    Small block threshold = 512, in 64 size classes.

    class   size   num pools   blocks in use  avail blocks
    -----   ----   ---------   -------------  ------------
        4     40           2             139            63
        5     48           1               2            82
      ...
       62    504           3              21             3
       63    512           3              18             3

    # times object malloc called       =            2,045,325
    # arenas allocated total           =                  637
    # arenas reclaimed                 =                    1
    # arenas highwater mark            =                  636
    # arenas allocated current         =                  636
    636 arenas * 262144 bytes/arena    =          166,723,584

    # bytes in allocated blocks        =          162,688,624
    # bytes in available blocks        =              116,824
    0 unused pools * 4096 bytes        =                    0
    # bytes lost to pool headers       =            1,953,792
    # bytes lost to quantization       =            1,964,344
    # bytes lost to arena alignment    =                    0
    Total                              =          166,723,584
    Small block threshold = 512, in 64 size classes.

    class   size   num pools   blocks in use  avail blocks
    -----   ----   ---------   -------------  ------------
        4     40           2             139            63
        5     48           1               2            82
      ...
       62    504           3              21             3
       63    512           3              18             3

    # times object malloc called       =            2,048,525
    # arenas allocated total           =                  638
    # arenas reclaimed                 =                    1
    # arenas highwater mark            =                  637
    # arenas allocated current         =                  637
    637 arenas * 262144 bytes/arena    =          166,985,728

    # bytes in allocated blocks        =          162,944,624
    # bytes in available blocks        =              116,824
    0 unused pools * 4096 bytes        =                    0
    # bytes lost to pool headers       =            1,956,864
    # bytes lost to quantization       =            1,967,416
    # bytes lost to arena alignment    =                    0
    Total                              =          166,985,728
    loose_new_reference: DONE


.. _debug-version-of-python-COUNT_ALLOCS-label:

-----------------------------------------------
Python Debug build with ``COUNT_ALLOCS``
-----------------------------------------------

A Python debug build with ``COUNT_ALLOCS`` give some additional information about each object *type* (not the individual objects themselves). A ``PyObject`` grows some extra fields that track the reference counts for that type. The fields are:

=============== ====================================================================
Field           Description
=============== ====================================================================
``tp_allocs``   The number of times an object of this type was allocated.
``tp_frees``    The number of times an object of this type was freed.
``tp_maxalloc`` The maximum seen value of ``tp_allocs - tp_frees`` so this is the
                maximum count of this type allocated at the same time.
=============== ====================================================================

The ``sys`` module also gets an extra function ``sys.getcounts()`` that returns a list of tuples: ``[(tp_typename, tp_allocs, tp_frees, tp_maxalloc), ...]``.


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Building the Python Executable with ``COUNT_ALLOCS``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Either:

.. code-block:: bash

    ../configure CFLAGS='-DCOUNT_ALLOCS' --with-pydebug
    make

Or:

.. code-block:: bash

    ../configure --with-pydebug
    make EXTRA_CFLAGS="-DCOUNT_ALLOCS"

.. warning::

    When using ``COUNT_ALLOCS`` any Python extensions now need to be rebuilt with this Python executable as it fundementally changes the structure of a ``PyObject``.
    
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Using the Python Executable with ``COUNT_ALLOCS``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

An example of using this build is here: :ref:`leaked-new-references-usingCOUNT_ALLOCS-label`

-----------------------------------------------------------
Identifying the Python Build Configuration from the Runtime
-----------------------------------------------------------

The module ``sysconfig`` allows you access to the configuration of the Python runtime. At its simplest, and most verbose, this can be used thus:

.. code-block:: bash

    $ ./python.exe -m sysconfig
    Platform: "macosx-10.9-x86_64"
    Python version: "3.4"
    Current installation scheme: "posix_prefix"

    Paths: 
        data = "/usr/local"
        ...
        stdlib = "/usr/local/lib/python3.4"

    Variables: 
        ABIFLAGS = "dm"
        AC_APPLE_UNIVERSAL_BUILD = "0"
        AIX_GENUINE_CPLUSPLUS = "0"
        AR = "ar"
        ...
        py_version = "3.4.3"
        py_version_nodot = "34"
        py_version_short = "3.4"


Importing ``sysconfig`` into an interpreter session gives two useful functions are ``get_config_var(...)`` which gets the setting for a particular macro and ``get_config_vars()`` which gets a dict of ``{macro : value, ...}``. For example:

.. code-block:: python

    >>> import sysconfig
    >>> sysconfig.get_config_var('Py_DEBUG')
    1

For advanced usage you can parse any ``pyconfig.h`` into a dict by opening that file and passing it to ``sysconfig.parse_config_h(f)`` as a file object. ``sysconfig.get_config_h_filename()`` will give you the configuration file for the runtime (assuming it still exists). So:
    
.. code-block:: python

    >>> with open(sysconfig.get_config_h_filename()) as f:
          cfg = sysconfig.parse_config_h(f)
          
