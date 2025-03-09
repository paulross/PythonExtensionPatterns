.. highlight:: python
    :linenothreshold: 10

.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _debug-version-of-python-label:

.. index::
    single: Debugging; Debug Version of Python

===============================================
Building and Using a Debug Version of Python
===============================================

There is a spectrum of debug builds of Python that you can create. This chapter describes how to create them.


.. index::
    single: Debugging; Building Python

--------------------------------------------
Building a Standard Debug Version of Python
--------------------------------------------

Download check and and unpack the Python source into the directory of your choice:

.. code-block:: bash

    $ curl -o Python-3.13.2.tgz https://www.python.org/ftp/python/3.13.2/Python-3.13.2.tgz
    # Get the Gzipped source tarball md5 from:
    # https://www.python.org/downloads/release/python-3132/ "6192ce4725d9c9fc0e8a1cd38410b417"
    $ md5 Python-3.13.2.tgz | grep 6192ce4725d9c9fc0e8a1cd38410b417
    MD5 (Python-3.13.2.tgz) = 6192ce4725d9c9fc0e8a1cd38410b417
    # No output would be a md5 missmatch.
    $ tmp echo $?
    0
    # 1 would be a md5 missmatch.
    $ tar -xzf Python-3.13.2.tgz
    $ cd Python-3.13.2


Then in the source directory create a debug directory for the debug build:

.. code-block:: bash

    $ mkdir debug
    $ cd debug
    $ ../configure --with-pydebug
    $ make
    $ make test

.. index::
    single: Debugging; Python Build Macros

-----------------------
Specifying Macros
-----------------------

They can be specified at the configure stage, this works:

.. code-block:: bash

    $ ../configure CFLAGS='-DPy_DEBUG -DPy_TRACE_REFS' --with-pydebug
    $ make


However the python documentation suggests the alternative way of specifying them when invoking make:

.. code-block:: bash

    $ ../configure --with-pydebug
    $ make EXTRA_CFLAGS="-DPy_REF_DEBUG"

I don't know why one way would be regarded as better than the other.

.. index::
    pair: Debugging; Py_DEBUG
    pair: Debugging; Py_REF_DEBUG
    pair: Debugging; Py_TRACE_REFS
    pair: Debugging; WITH_PYMALLOC
    pair: Debugging; PYMALLOC_DEBUG
    pair: Debugging; LLTRACE

---------------------------
The Debug Builds
---------------------------

The builds are controlled by the following macros.
The third column shows if CPython C extensions have to be rebuilt against that version of Python:

.. list-table:: Debug Macros
   :widths: 20 70 10
   :header-rows: 1

   * - Macro
     - Description
     - Rebuild EXT?
   * - ``Py_DEBUG``
     - A standard debug build. ``Py_DEBUG`` sets ``LLTRACE``, ``Py_REF_DEBUG``, ``Py_TRACE_REFS``, and
       ``PYMALLOC_DEBUG`` (if ``WITH_PYMALLOC`` is enabled).
     - Yes
   * - ``Py_REF_DEBUG``
     - Turn on aggregate reference counting which will be
       displayed in the interactive interpreter when
       invoked with ``-X showrefcount`` on the command line.
       If you are not keeping references to objects and the
       count is increasing there is probably a leak.
       Also adds ``sys.gettotalrefcount()`` to the ``sys``
       module and this returns the total number of references.
     - No
   * - ``Py_TRACE_REFS``
     - Turns on reference tracing. Sets ``Py_REF_DEBUG``.
     - Yes
   * - ``WITH_PYMALLOC``
     - Enables Pythons small memory allocator. For Valgrind
       this must be disabled, if using Pythons malloc
       debugger (using ``PYMALLOC_DEBUG``) this must be
       enabled.
       See: :ref:`debug-version-of-python-memory_alloc-label`
     - No
   * - ``PYMALLOC_DEBUG``
     - Enables Python's malloc debugger that annotates
       memory blocks. Requires ``WITH_PYMALLOC``.
       See: :ref:`debug-version-of-python-memory_alloc-label`
     - No

Here is the description of other debug macros that are set by one of the macros above:

=================== =======================================================
Macro               Description
=================== =======================================================
``LLTRACE``         Low level tracing. See ``Python/ceval.c``.
=================== =======================================================


.. _debug-version-of-python-memory_alloc-label:

.. index::
    single: Debugging; Python's Memory Allocator

---------------------------
Python's Memory Allocator
---------------------------

A normal build of Python gives CPython a special memory allocator 'PyMalloc'. When enabled this mallocs largish chunks of memory from the OS and then uses this pool for the actual PyObjects. With PyMalloc active Valgrind can not see all allocations and deallocations.

There are two Python builds of interest to help solve memory problems:

* Disable PyMalloc so that Valgrind can analyse the memory usage.
* Enable PyMalloc in debug mode, this creates memory blocks with special bit patterns and adds debugging information on each end of any dynamically allocated memory. This pattern is checked on every alloc/free and if found to be corrupt a diagnostic is printed and the process terminated.

To make a version of Python with its memory allocator suitable for use with Valgrind:

.. code-block:: bash

    $ ../configure --with-pydebug --without-pymalloc
    $ make

See :ref:`using-valgrind-label` for using Valgrind.

To make a version of Python with its memory allocator using Python's malloc debugger either:

.. code-block:: bash

    $ ../configure CFLAGS='-DPYMALLOC_DEBUG' --with-pydebug
    $ make

Or:

.. code-block:: bash

    $ ../configure --with-pydebug
    $ make EXTRA_CFLAGS="-DPYMALLOC_DEBUG"

This builds Python with the ``WITH_PYMALLOC`` and ``PYMALLOC_DEBUG`` macros defined.

.. index::
    pair: Debugging; Access After Free

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Finding Access after Free With ``PYMALLOC_DEBUG``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TODO:

Python built with ``PYMALLOC_DEBUG`` is the most effective way of detecting access after free.
For example if we have this CPython code:

.. code-block:: c

    static PyObject *access_after_free(PyObject *Py_UNUSED(module)) {
        PyObject *pA = PyLong_FromLong(1024L * 1024L);
        fprintf(
            stdout,
            "%s(): Before Py_DECREF(0x%p) Ref count: %zd\n",
            __FUNCTION__, (void *)pA, Py_REFCNT(pA)
        );
        PyObject_Print(pA, stdout, Py_PRINT_RAW);
        fprintf(stdout, "\n");

        Py_DECREF(pA);

        fprintf(
                stdout,
                "%s(): After Py_DECREF(0x%p) Ref count: %zd\n",
                __FUNCTION__, (void *)pA, Py_REFCNT(pA)
        );
        PyObject_Print(pA, stdout, Py_PRINT_RAW);
        fprintf(stdout, "\n");

        Py_RETURN_NONE;
    }

And we call this from the interpreter what we get is undefined and may vary from Python version to version.
Here is Python 3.9:

.. code-block:: python

    # $ python
    # Python 3.9.7 (v3.9.7:1016ef3790, Aug 30 2021, 16:39:15)
    # [Clang 6.0 (clang-600.0.57)] on darwin
    # Type "help", "copyright", "credits" or "license" for more information.
    >>> from cPyExtPatt import cPyRefs
    >>> cPyRefs.access_after_free()
    access_after_free(): Before Py_DECREF(0x0x7fe55f1e2fb0) Ref count: 1
    1048576
    access_after_free(): After Py_DECREF(0x0x7fe55f1e2fb0) Ref count: 140623120051984
    1048576
    >>>

And Python 3.13:

.. code-block:: python

    # $ python
    # Python 3.13.1 (v3.13.1:06714517797, Dec  3 2024, 14:00:22) [Clang 15.0.0 (clang-1500.3.9.4)] on darwin
    # Type "help", "copyright", "credits" or "license" for more information.
    >>> from cPyExtPatt import cPyRefs
    >>> cPyRefs.access_after_free()
    access_after_free(): Before Py_DECREF(0x0x102465890) Ref count: 1
    1048576
    access_after_free(): After Py_DECREF(0x0x102465890) Ref count: 4333131856
    0
    >>>

And we call this from the (debug) Python 3.13 interpreter we get a diagnostic:

.. code-block:: python

    # (PyExtPatt_3.13.2_Debug) PythonExtensionPatterns git:(develop) $ python
    # Python 3.13.2 (main, Mar  9 2025, 11:01:02) [Clang 14.0.3 (clang-1403.0.22.14.1)] on darwin
    # Type "help", "copyright", "credits" or "license" for more information.
    >>> from cPyExtPatt import cPyRefs
    >>> cPyRefs.access_after_free()
    access_after_free(): Before Py_DECREF(0x0x10a984e00) Ref count: 1
    1048576
    access_after_free(): After Py_DECREF(0x0x10a984e00) Ref count: -2459565876494606883
    <refcnt -2459565876494606883 at 0x10a984e00>
    >>>

.. index::
    single: Debugging; PyMalloc Statistics

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Getting Statistics on PyMalloc
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the environment variable ``PYTHONMALLOCSTATS`` exists when running Python built with
``WITH_PYMALLOC``+``PYMALLOC_DEBUG`` then a (detailed) report of pymalloc activity is output on stderr whenever
a new 'arena' is allocated.

.. code-block:: bash

    $ PYTHONMALLOCSTATS=1 python

I have no special knowledge about the output you see when running Python this way which looks like this::

    >>> from cPyExtPatt import cPyRefs
    >>> cPyRefs.leak_new_reference(1000, 10000)
    loose_new_reference: value=1000 count=10000
    Small block threshold = 512, in 32 size classes.

    class   size   num pools   blocks in use  avail blocks
    -----   ----   ---------   -------------  ------------
        1     32           1              83           427
        2     48           1             146           194
        3     64          48           12240             0
        4     80          82           16690            38
        5     96          74           12409           171
        6    112          22            3099            91
        7    128          10            1156           114
        8    144          12            1346            10
        9    160           4             389            19
       10    176           4             366             2
       11    192          32            2649            71
       12    208           3             173            61
       13    224           2             132            12
       14    240          13             860            24
       15    256           5             281            34
       16    272           6             344            16
       17    288           7             348            44
       18    304           8             403            21
       19    320           4             198             6
       20    336           4             183             9
       21    352           5             198            32
       22    368           3             119            13
       23    384           3             104            22
       24    400           3             106            14
       25    416           3              98            19
       26    432           8             295             1
       27    448           2              64             8
       28    464           3              70            35
       29    480           2              44            24
       30    496           2              59             5
       31    512           2              45            17

    # arenas allocated total           =                    6
    # arenas reclaimed                 =                    0
    # arenas highwater mark            =                    6
    # arenas allocated current         =                    6
    6 arenas * 1048576 bytes/arena     =            6,291,456

    # bytes in allocated blocks        =            5,927,280
    # bytes in available blocks        =              224,832
    0 unused pools * 16384 bytes       =                    0
    # bytes lost to pool headers       =               18,144
    # bytes lost to quantization       =               22,896
    # bytes lost to arena alignment    =               98,304
    Total                              =            6,291,456

    arena map counts
    # arena map mid nodes              =                    1
    # arena map bot nodes              =                    1

    # bytes lost to arena map root     =              262,144
    # bytes lost to arena map mid      =              262,144
    # bytes lost to arena map bot      =              131,072
    Total                              =              655,360
    loose_new_reference: DONE


.. index::
    pair: Debugging; sysconfig

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
          
