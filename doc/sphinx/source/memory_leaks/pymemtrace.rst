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


``pymemtrace`` cPyMemTrace
==========================

``cPyMemTrace`` is a Python profiler written in 'C' that records the
`Resident Set Size <https://en.wikipedia.org/wiki/Resident_set_size>`_
for every Python and C call and return.
It writes this data to a log file with a name of the form ``YYMMDD_HHMMSS_PID.log``.

Logging Changes in RSS
--------------------------------

Here is a simple example:

.. code-block:: python

    from pymemtrace import cPyMemTrace

    def create_string(l: int) -> str:
        return ' ' * l

    with cPyMemTrace.Profile():
        l = []
        for i in range(8):
            l.append(create_string(1024**2))
        while len(l):
            l.pop()

This produces a log file in the current working directory:

.. code-block:: text

          Event        dEvent  Clock        What     File    #line Function      RSS           dRSS
    NEXT: 0            +0      0.066718     CALL     test.py #   9 create_string  9101312      9101312
    NEXT: 1            +1      0.067265     RETURN   test.py #  10 create_string 10153984      1052672
    PREV: 4            +3      0.067285     CALL     test.py #   9 create_string 10153984            0
    NEXT: 5            +4      0.067777     RETURN   test.py #  10 create_string 11206656      1052672
    PREV: 8            +3      0.067787     CALL     test.py #   9 create_string 11206656            0
    NEXT: 9            +4      0.068356     RETURN   test.py #  10 create_string 12259328      1052672
    PREV: 12           +3      0.068367     CALL     test.py #   9 create_string 12259328            0
    NEXT: 13           +4      0.068944     RETURN   test.py #  10 create_string 13312000      1052672
    PREV: 16           +3      0.068954     CALL     test.py #   9 create_string 13312000            0
    NEXT: 17           +4      0.069518     RETURN   test.py #  10 create_string 14364672      1052672
    PREV: 20           +3      0.069534     CALL     test.py #   9 create_string 14364672            0
    NEXT: 21           +4      0.070101     RETURN   test.py #  10 create_string 15417344      1052672
    PREV: 24           +3      0.070120     CALL     test.py #   9 create_string 15417344            0
    NEXT: 25           +4      0.070663     RETURN   test.py #  10 create_string 16470016      1052672
    PREV: 28           +3      0.070677     CALL     test.py #   9 create_string 16470016            0
    NEXT: 29           +4      0.071211     RETURN   test.py #  10 create_string 17522688      1052672

By default not all events are recorded just any that increase the RSS by one page along with the immediately preceding event.

Logging Every Event
--------------------------------

If all events are needed then change the constructor argument to 0:

.. code-block:: python

    with cPyMemTrace.Profile(0):
        # As before

And the log file looks like this:

.. code-block:: text

          Event        dEvent  Clock        What     File    #line Function      RSS           dRSS
    NEXT: 0            +0      0.079408     CALL     test.py #   9 create_string  9105408      9105408
    NEXT: 1            +1      0.079987     RETURN   test.py #  10 create_string 10158080      1052672
    NEXT: 2            +1      0.079994     C_CALL   test.py #  64 append        10158080            0
    NEXT: 3            +1      0.079998     C_RETURN test.py #  64 append        10158080            0
    NEXT: 4            +1      0.080003     CALL     test.py #   9 create_string 10158080            0
    NEXT: 5            +1      0.080682     RETURN   test.py #  10 create_string 11210752      1052672
    NEXT: 6            +1      0.080693     C_CALL   test.py #  64 append        11210752            0
    NEXT: 7            +1      0.080698     C_RETURN test.py #  64 append        11210752            0
    NEXT: 8            +1      0.080704     CALL     test.py #   9 create_string 11210752            0
    NEXT: 9            +1      0.081414     RETURN   test.py #  10 create_string 12263424      1052672
    NEXT: 10           +1      0.081424     C_CALL   test.py #  64 append        12263424            0
    NEXT: 11           +1      0.081429     C_RETURN test.py #  64 append        12263424            0
    NEXT: 12           +1      0.081434     CALL     test.py #   9 create_string 12263424            0
    NEXT: 13           +1      0.081993     RETURN   test.py #  10 create_string 13316096      1052672
    NEXT: 14           +1      0.081998     C_CALL   test.py #  64 append        13316096            0
    ...
    NEXT: 59           +1      0.084531     C_RETURN test.py #  66 pop           17526784            0
    NEXT: 60           +1      0.084535     C_CALL   test.py #  65 len           17526784            0
    NEXT: 61           +1      0.084539     C_RETURN test.py #  65 len           17526784            0
    NEXT: 62           +1      0.084541     C_CALL   test.py #  66 pop           17526784            0
    NEXT: 63           +1      0.084561     C_RETURN test.py #  66 pop           17526784            0
    NEXT: 64           +1      0.084566     C_CALL   test.py #  65 len           17526784            0
    NEXT: 65           +1      0.084568     C_RETURN test.py #  65 len           17526784            0

There is some discussion about the performance of ``cPyMemTrace`` here :ref:`tech_notes-cpymemtrace`
