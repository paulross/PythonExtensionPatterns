.. moduleauthor:: Paul Ross <apaulross@gmail.com>
.. sectionauthor:: Paul Ross <apaulross@gmail.com>

.. index:: single: Memory Leaks; Tools

Tools for Detecting Memory Leaks
====================================

Tools for analysing memory can be characterised by:

.. list-table:: Tool Characteristics
   :widths: 25 75
   :header-rows: 1

   * - Characteristic
     - Description
   * - **Availability**
     - Does it come with the platform?
       Is it within Python or the standard library?
       Does it need third party library installation or requires a special Python build of some sort?
   * - **Memory Granularity**
     - How detailed is the memory measurement?
       Somewhere between every ``malloc`` or the overall memory usage as seen by the OS.
   * - **Execution Granularity**
     - How detailed is the execution measurement? Per line, per function, for Python or C code?
   * - **Memory Cost**
     - What is the extra memory consumption is introduced by using this tool?
   * - **Execution Cost**
     - What is the extra runtime introduced by using this tool?
   * - **Developer Cost**
     - How hard it is to use the tool?

Each tool makes trade offs between each of these characteristics.

.. index:: single: Memory Leaks; Platform Tools

Platform Tools
------------------

These tools come ready with the platform. They give a good overall picture of the memory usage.

=========================== ====================================================================================================
Characteristic              Description
=========================== ====================================================================================================
**Availability**            Always.
**Memory Granularity**      Usually the total memory usage by the process.
**Execution Granularity**   Generally periodic at low frequency, typically of the order of seconds.
**Memory Cost**             Usually none.
**Execution Cost**          Usually none.
**Developer Cost**          Easy.
=========================== ====================================================================================================

Here are some tools available on differenct platforms:

Windows
^^^^^^^^^^^^^^^^^^^

A weakness of Windows, especially in corporate environments, is that the OS is usually severely locked down.
At best this usually vastly extends the time it takes to find a leak, at worst this hopelessly limits the tools that can
be installed or run on the platform.
In this case some leaks can never be found and fixed.

Windows Task Manager
""""""""""""""""""""""""""""

This is the basic tool for reviewing process memory.
The columns to monitor are "Working Set (Memory)" which broadly corresponds to the Unix
`Resident Set Size (RSS) <https://en.wikipedia.org/wiki/Resident_set_size>`_ .
The Sysinternals ``procexp`` (see below) is a more sophisticated version.

``perfmon.exe``
""""""""""""""""""""""""""""

This is a Microsoft tool for logging performance and plotting it in real time.
It is quite capable but a little fiddly to set up.
The third party Python library ``psutil`` is a useful alternative and the ``pymentrace.procces`` also provides memory
plotting of arbitrary processes: :ref:`chapter_memory_leaks.pymemtrace.proces` using ``gnuplot``.

Sysinternals Suite
""""""""""""""""""""""""""""

The outstanding `Windows Sysinternals tools <https://docs.microsoft.com/en-gb/sysinternals/>`_ are a wonderful
collection of tools and are essential for debugging any Windows application.

Linux
^^^^^^^^^^^^^^^^^^^

TODO: Finish this.

``/proc/<PID>``
"""""""""""""""""""""

The ``/proc/<PID>`` filesystem is full of good stuff.

Valgrind
"""""""""""""""""

`Valgrind <https://www.valgrind.org>`_ is essential on any Linux development platform.
There is a tutorial `here <https://pythonextensionpatterns.readthedocs.io/en/latest/debugging/valgrind.html>`_ for
building and using Python with Valgrind.

eBPF
"""""""""""""""""

The next big thing after DTrace is `eBPF <http://www.brendangregg.com/blog/2019-01-01/learn-ebpf-tracing.html>`_.
Truly awesome.


Mac OS X
^^^^^^^^^^^^^^^^^^^

Tools such as ``vmmap``, ``heap``, ``leaks``, ``malloc_history``, ``vm_stat`` can all help.
See the man pages for further information.

Some useful information for memory tools from
`Apple <https://developer.apple.com/library/archive/documentation/Performance/Conceptual/ManagingMemory/Articles/VMPages.html>`_


DTrace
"""""""""""""""""""

Mac OS X is DTrace aware, this needs a special build of Python, here is an
`introduction <https://github.com/paulross/dtrace-py>`_ that takes you through building and using a DTrace aware version
of Python on Mac OS X.

Some examples of using DTrace with
`pymemtrace <https://pymemtrace.readthedocs.io/en/latest/examples/dtrace.html>`_.

.. index:: single: Memory Leaks; Python Tools

Python Tools
------------------

Modules from the Standard Library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

=========================== ====================================================================================================
Characteristic              Description
=========================== ====================================================================================================
**Availability**            Always.
**Memory Granularity**      Various.
**Execution Granularity**   Various.
**Memory Cost**             Usually none.
**Execution Cost**          Usually none.
**Developer Cost**          Straightforward.
=========================== ====================================================================================================

``sys``
"""""""""""""""""""""

The :py:mod:`sys` has a number of useful functions, mostly CPython specific.

.. Sigh. Links do not work in list tables such as `<Documentation https://docs.python.org/dev/library/sys.html#sys.getallocatedblocks>`_


.. list-table:: ``sys`` Tools
   :widths: 40 40 40
   :header-rows: 1

   * - Tool
     - Description
     - Notes
   * - ``getallocatedblocks()``
     - Returns the number of
       `allocated blocks <https://docs.python.org/dev/library/sys.html#sys.getallocatedblocks>`_, regardless of size.
     - This has no information about the size of any block.
       CPython only.
       Implemented in ``Objects/obmalloc.c`` as ``_Py_GetAllocatedBlocks``.
       As implemented in Python 3.9 this returns the total reference count of every *pool* in every *arena*.
   * - ``getrefcount(object)``
     - Returns the `reference count <https://docs.python.org/dev/library/sys.html#sys.getrefcount>`_ of an object.
     - This is increased by one for the duration of the call.
   * - ``getsizeof(object)``
     - Returns the `size of an object <https://docs.python.org/dev/library/sys.html#sys.getsizeof>`_ in bytes.
     - Builtin objects will return correct results.
       Others are implementation specific.
       User defined objects can implement ``__sizeof__`` which will be called if available.
   * - ``_debugmallocstats(object)``
     - Prints the state of the
       `Python Memory Allocator <https://docs.python.org/dev/library/sys.html#sys._debugmallocstats>`_
       ``pymalloc`` to stderr.
       ``pymentrace``'s :ref:`memory_leaks.pymemtrace.debug_malloc_stats` is a very useful wrapper around
       :py:func:`sys._debugmallocstats` which can report changes to Python's small object allocator.
     - See :ref:`memory_leaks.pymemtrace.debug_malloc_stats` for a ``pymemtrace`` wrapper that makes this much
       more useful.


``gc``
"""""""""""""""""""""

The :py:mod:`gc` controls the Python garbage collector.
Turning the garbage collector off with ``gc.disable()`` is worth trying to see what effect, if any, it has.

``tracemalloc``
"""""""""""""""""""""

:py:mod:`tracemalloc` is a useful module that can trace memory blocks allocate by Python.
It is invasive and using it consumes a significant amount of memory itself.
See :ref:`memory_leaks.pymemtrace.trace_malloc` for a ``pymemtrace`` wrapper that makes this much more useful by
report changes to Python's memory allocator..

.. index:: single: Memory Leaks; Third Party Python Tools

Third Party Modules
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``psutil``
"""""""""""""""""""""

``psutil`` is an excellent, third party, package that can report high level information on a process.
`psutil on PyPi <https://pypi.org/project/psutil/>`_

Here are some notes on the
`cost of computing <https://pymemtrace.readthedocs.io/en/latest/tech_notes/rss_cost.html>`_
the Resident Set Size (RSS).


.. _objgraph: https://pypi.org/project/objgraph/

``objgraph``
"""""""""""""""""""""

`objgraph`_ is a wrapper around the Python garbage collector that can take a snapshot of the Python objects in scope.
This is quite invasive and expensive but can be very useful in specific cases.
It does some excellent visualisations using graphviz, xdot etc.


.. _guppy_3: https://pypi.org/project/guppy3/
.. _guppy_3_source: https://github.com/zhuyifei1999/guppy3/

``Guppy 3``
"""""""""""""""""""""

`guppy_3`_ (source: `guppy_3_source`_) is a project that gives a highly detailed picture of the current state of the
Python heap.


.. _memory_profiler: http://pypi.python.org/pypi/memory_profiler
.. _memory_profiler_src: https://github.com/pythonprofilers/memory_profiler

``memory_profiler``
"""""""""""""""""""""

Sadly `memory_profiler`_ (source: `memory_profiler_src`_) this excellent project is no longer actively maintained
(volunteers welcome, contact f@bianp.net).
It gives an annotated version of your source code line by line with the memory usage.
It is pure Python and relies just on ``psutil``.


.. index:: single: Memory Leaks; Python Debugging Tools

Debugging Tools
------------------

Debugging Python (and C/C++ extensions) with GDB:

* GDB support for Python: `<https://devguide.python.org/gdb/>`_
* Python debugging with GDB: `<https://wiki.python.org/moin/DebuggingWithGdb>`_
  and `<https://pythondev.readthedocs.io/gdb.html>`_
* Python debugging tools: `<https://pythondev.readthedocs.io/debug_tools.html>`_


Building a Debug Version of Python
---------------------------------------

This is an essential technique however it is limited, due to speed, to a development environment rather than in
production.

Building a debug version of Python in a variety of forms:
`<https://pythonextensionpatterns.readthedocs.io/en/latest/debugging/debug_python.html#debug-version-of-python-label>`_

Building a DTrace aware version of Python: `<https://github.com/paulross/dtrace-py>`_
Here are some examples of using
`DTrace with pymemtrace <https://pymemtrace.readthedocs.io/en/latest/examples/dtrace.html>`_ with some
`technical notes <https://pymemtrace.readthedocs.io/en/latest/tech_notes/dtrace.html>`_ on using DTrace.
