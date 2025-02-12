.. moduleauthor:: Paul Ross <apaulross@gmail.com>
.. sectionauthor:: Paul Ross <apaulross@gmail.com>


.. index::
    single: pymemtrace
    single: Memory Leaks; pymemtrace

=============================
``pymemtrace``
=============================

My ``pymemtrace`` project contains a number of tools that help detect memory usage and leaks.
The documentation contains advice on handling memory leaks.

* On PyPi: `<https://pypi.org/project/pymemtrace/>`_
* Project: `<https://github.com/paulross/pymemtrace>`_
* Documentation: `<https://pymemtrace.readthedocs.io/en/latest/index.html>`_

Here is the introduction to that project:

``pymemtrace`` provides tools for tracking and understanding Python memory usage at different levels, at different
granularities and with different runtime costs.

Full documentation: https://pymemtrace.readthedocs.io

.. index::
    single: pymemtrace; Tools

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


.. index::
    single: pymemtrace; Tool Characteristics

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
   :widths: 30 30 30 20 20
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
-------

Python memory tracing.

* Free software: MIT license
* Documentation: https://pymemtrace.readthedocs.io.
* Project: https://github.com/paulross/pymemtrace.

Credits
-------

Phil Smith (AHL) with whom a casual lunch time chat lead to the creation of an earlier, but quite different
implementation, of ``cPyMemTrace`` in pure Python.

This package was created with Cookiecutter_ and the `audreyr/cookiecutter-pypackage`_ project template.

.. _Cookiecutter: https://github.com/audreyr/cookiecutter
.. _`audreyr/cookiecutter-pypackage`: https://github.com/audreyr/cookiecutter-pypackage


.. todo::

    Add pymemtrace with running code and examples.

.. index::
    single: pymemtrace; process

.. _chapter_memory_leaks.pymemtrace.proces:

``pymemtrace`` Process
======================

`pymemtrac's process <https://pymemtrace.readthedocs.io/en/latest/examples/process.html>`_ is an ultralight weight tool
for monitoring the memory usage of a process at regular intervals.

``process.log_process`` provides a context manager that launches a separate thread that logs the memory usage in JSON
of the current process at regular intervals (the CLI version can monitor any user specified process).
The log format is designed so that the data can be easily extracted using, say, regular expressions.

Here is an example that creates randomly sized large strings.
Of interest is the line:

.. code-block:: python

    process.add_message_to_queue(f'String of {size:,d} bytes')

Which injects a message into the log output.
Here is the example:

.. code-block:: python

    """
    Example of using process that logs process data to the current log.
    """
    import logging
    import random
    import sys
    import time

    from pymemtrace import process

    logger = logging.getLogger(__file__)

    def main() -> int:
        logging.basicConfig(
            level=logging.INFO,
            format= (
                '%(asctime)s - %(filename)s#%(lineno)d - %(process)5d'
                ' - (%(threadName)-10s) - %(levelname)-8s - %(message)s'
            ),
        )
        logger.info('Demonstration of logging a process')
        # Log process data to the log file every 0.5 seconds.
        with process.log_process(interval=0.5, log_level=logger.getEffectiveLevel()):
            for i in range(8):
                size = random.randint(128, 128 + 256) * 1024 ** 2
                # Add a message to report in the next process write.
                process.add_message_to_queue(f'String of {size:,d} bytes')
                s = ' ' * size
                time.sleep(0.75 + random.random())
                del s
                time.sleep(0.25 + random.random() / 2)
        return 0


    if __name__ == '__main__':
        sys.exit(main())

Might give:

.. code-block:: text

    $ python3.12 ex_process.py
    2025-02-12 14:16:58,675 - ex_process.py#19 - 10193 - (MainThread) - INFO     - Demonstration of logging a process
    2025-02-12 14:16:58,676 - process.py#289 - 10193 - (ProcMon   ) - INFO     - ProcessLoggingThread-JSON-START {"timestamp": "2025-02-12 14:16:58.676195", "memory_info": {"rss": 18067456, "vms": 34990526464, "pfaults": 6963, "pageins": 1369}, "cpu_times": {"user": 0.340946528, "system": 0.991057664, "children_user": 0.0, "children_system": 0.0}, "elapsed_time": 5.407800197601318, "pid": 10193}
    2025-02-12 14:16:59,180 - process.py#293 - 10193 - (ProcMon   ) - INFO     - ProcessLoggingThread-JSON {"timestamp": "2025-02-12 14:16:59.180476", "memory_info": {"rss": 199512064, "vms": 35171934208, "pfaults": 51261, "pageins": 1374}, "cpu_times": {"user": 0.379827552, "system": 1.031979648, "children_user": 0.0, "children_system": 0.0}, "elapsed_time": 5.912204027175903, "pid": 10193, "label": "String of 181,403,648 bytes"}
    2025-02-12 14:16:59,682 - process.py#289 - 10193 - (ProcMon   ) - INFO     - ProcessLoggingThread-JSON {"timestamp": "2025-02-12 14:16:59.681947", "memory_info": {"rss": 18104320, "vms": 34990526464, "pfaults": 51262, "pageins": 1374}, "cpu_times": {"user": 0.380316928, "system": 1.047401792, "children_user": 0.0, "children_system": 0.0}, "elapsed_time": 6.413706064224243, "pid": 10193}
    ...
    2025-02-12 14:17:12,312 - process.py#289 - 10193 - (ProcMon   ) - INFO     - ProcessLoggingThread-JSON {"timestamp": "2025-02-12 14:17:12.312343", "memory_info": {"rss": 247758848, "vms": 35220168704, "pfaults": 508755, "pageins": 1374}, "cpu_times": {"user": 0.820292992, "system": 1.639239552, "children_user": 0.0, "children_system": 0.0}, "elapsed_time": 19.044106006622314, "pid": 10193}
    2025-02-12 14:17:12,763 - process.py#289 - 10193 - (MainThread) - INFO     - ProcessLoggingThread-JSON-STOP {"timestamp": "2025-02-12 14:17:12.762896", "memory_info": {"rss": 18116608, "vms": 34990526464, "pfaults": 508756, "pageins": 1374}, "cpu_times": {"user": 0.820827264, "system": 1.663195264, "children_user": 0.0, "children_system": 0.0}, "elapsed_time": 19.49466300010681, "pid": 10193}

    Process finished with exit code 0

Here is the memory data from one line in more detail.

.. code-block:: json

    {
        "timestamp": "2025-02-12 14:17:05.719867",
        "memory_info": {
            "rss": 264527872,
            "vms": 35236945920,
            "pfaults": 342856,
            "pageins": 1374
        },
        "cpu_times": {
            "user": 0.658989888,
            "system": 1.418129152,
            "children_user": 0.0,
            "children_system": 0.0
        },
        "elapsed_time": 12.45150899887085,
        "pid": 10193,
        "label": "String of 246,415,360 bytes"
    }

``pymemtrace.process`` provides and number of ways of tabulating and plotting this data that gives a clearer picture,
at a high level, of what is happening to the process memory.
