.. highlight:: python
    :linenothreshold: 10

.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3


.. _memory_leaks-label:

*******************
Memory Leaks
*******************

My ``pymemtrace`` project contains a number of tools that help detect memory usage and leaks.
The documentation contains advice on handling memory leaks.

* On PyPi: `<https://pypi.org/project/pymemtrace/>`_
* Project: `<https://github.com/paulross/pymemtrace>`_
* Documentation: `<https://pymemtrace.readthedocs.io/en/latest/index.html>`_

Here is the introduction to that project:

=============================
``pymemtrace`` Introduction
=============================


``pymemtrace`` provides tools for tracking and understanding Python memory usage at different levels, at different
granularities and with different runtime costs.

Full documentation: https://pymemtrace.readthedocs.io

pymemtrace Tools
======================

The tools provided by ``pymemtrace``:

* ``process`` is a very lightweight way of logging the total memory usage at regular time intervals.
  It can plot memory over time with plotting programs such as ``gnuplot``.
  See `some process examples <https://pymemtrace.readthedocs.io/en/latest/examples/process.html>`_
* ``cPyMemTrace`` is a memory tracer written in C that can report total memory usage for every function call/return for
  both C and Python sections.
  See some `cPyMemTrace examples <https://pymemtrace.readthedocs.io/en/latest/examples/c_py_mem_trace.html>`_
  and a `technical note on cPyMemTrace <https://pymemtrace.readthedocs.io/en/latest/tech_notes/cPyMemTrace.html>`_.
* DTrace: Here are a number of D scripts that can trace the low level ``malloc()`` and ``free()`` system calls and
  report how much memory was allocated and by whom.
  See some `DTrace examples <https://pymemtrace.readthedocs.io/en/latest/examples/dtrace.html>`_
  and a `technical note on DTrace <https://pymemtrace.readthedocs.io/en/latest/tech_notes/dtrace.html>`_.
* ``trace_malloc`` is a convenience wrapper around the Python standard library `tracemalloc` module.
  This can report Python memory usage by module and line compensating for the cost of ``tracemalloc``.
  This can take memory snapshots before and after code blocks and show the change on memory caused by that code.
  See some `trace_malloc examples <https://pymemtrace.readthedocs.io/en/latest/examples/trace_malloc.html>`_
* ``debug_malloc_stats`` is a wrapper around the ``sys._debugmallocstats`` function that can take snapshots of
  memory before and after code execution and report the significant differences of the Python small object allocator.
  See some `debug_malloc_stats examples <https://pymemtrace.readthedocs.io/en/latest/examples/debug_malloc_stats.html>`_


Tool Characteristics
======================

Each tool can be characterised by:

- *Memory Granularity*: In how much detail is a memory change is observed.
  An example of *coarse* memory granularity is measuring the
  `Resident Set Size <https://en.wikipedia.org/wiki/Resident_set_size>`_ which is normally in chunks of 4096 bytes.
  An example of *fine* memory granularity is recording every ``malloc()`` and ``free()``.
- *Execution Granularity*: In how much code detail is the memory change observed.
  An example of *coarse* execution granularity is measuring the memory usage every second.
  An example of *fine* execution granularity is recording the memory usage every Python line.
- *Memory Cost*: How much extra memory the tool needs.
- *Execution Cost*: How much the execution time is increased.

Clearly there are trade-offs between these depending on the problem you are trying to solve.

.. list-table:: **Tool Characteristics**
   :widths: 15 30 30 30 30
   :header-rows: 1

   * - Tool
     - Memory Granularity
     - Execution Granularity
     - Memory Cost
     - Execution Cost
   * - ``process``
     - RSS (total Python and C memory).
     - Regular time intervals.
     - Near zero.
     - Near zero.
   * - ``cPyMemTrace``
     - RSS (total Python and C memory).
     - Per Python line, Python function and C function call.
     - Near zero.
     - x10 to x20.
   * - DTrace
     - Every ``malloc()`` and ``free()``.
     - Per function call and return.
     - Minimal.
     - x90 to x100.
   * - ``trace_malloc``
     - Every Python object.
     - Per Python line, per function call.
     - Significant but compensated.
     - x900 for small objects, x6 for large objects.
   * - ``debug_malloc_stats``
     - Python memory pool.
     - Snapshots the CPython memory pool either side of a block of code.
     - Minimal.
     - x2000+ for small objects, x12 for large objects.

Licence
======================

Python memory tracing.

* Free software: MIT license
* Documentation: https://pymemtrace.readthedocs.io.
* Project: https://github.com/paulross/pymemtrace.

Credits
======================

Phil Smith (AHL) with whom a casual lunch time chat lead to the creation of an earlier, but quite different
implementation, of ``cPyMemTrace`` in pure Python.

This package was created with Cookiecutter_ and the `audreyr/cookiecutter-pypackage`_ project template.

.. _Cookiecutter: https://github.com/audreyr/cookiecutter
.. _`audreyr/cookiecutter-pypackage`: https://github.com/audreyr/cookiecutter-pypackage


