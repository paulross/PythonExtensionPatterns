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

.. _DTrace examples: https://pymemtrace.readthedocs.io/en/latest/examples/dtrace.html
.. _technical note on DTrace: https://pymemtrace.readthedocs.io/en/latest/tech_notes/dtrace.html#tech-notes-dtrace

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
  See some `DTrace examples`_ and a `technical note on DTrace`_.
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
   :widths: 25 30 30 20 20
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

.. index::
    single: pymemtrace; cPyMemTrace
    single: cPyMemTrace

``pymemtrace`` cPyMemTrace
==========================

``cPyMemTrace`` is a Python profiler written in 'C' that records the
`Resident Set Size <https://en.wikipedia.org/wiki/Resident_set_size>`_
for every Python and C call and return.

``cPyMemTrace`` writes this data to a log file with a name of the form:

- ``YYYYMMDD`` The date.
- ``_HHMMSS`` The time.
- ``_PID``
- ``_P`` or ``_T`` depending on whether it is a profile function or a trace function.
- ``n`` where n is the stack depth of the current profile or trace function as multiple nested profile or trace
  functions are allowed.
- The Python version such as ``PY3.13.0b3``.
- ``.log``.

For example ``"20241107_195847_62264_P_2_PY3.13.0b3.log"``

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

This produces a log file in the current working directory.
For brevity the log file does not show every change in the RSS but only when the RSS changes by some threshold.
By default this threshold is the system page size (typically 4096 bytes) [#]_.

Here is an example output:

.. code-block:: text

          Event dEvent  Clock    What     File    #line Function      RSS           dRSS
    NEXT: 0     +0      0.066718 CALL   test.py #   9 create_string  9101312      9101312
    NEXT: 1     +1      0.067265 RETURN test.py #  10 create_string 10153984      1052672
    PREV: 4     +3      0.067285 CALL   test.py #   9 create_string 10153984            0
    NEXT: 5     +4      0.067777 RETURN test.py #  10 create_string 11206656      1052672
    PREV: 8     +3      0.067787 CALL   test.py #   9 create_string 11206656            0
    NEXT: 9     +4      0.068356 RETURN test.py #  10 create_string 12259328      1052672
    PREV: 12    +3      0.068367 CALL   test.py #   9 create_string 12259328            0
    NEXT: 13    +4      0.068944 RETURN test.py #  10 create_string 13312000      1052672
    PREV: 16    +3      0.068954 CALL   test.py #   9 create_string 13312000            0
    NEXT: 17    +4      0.069518 RETURN test.py #  10 create_string 14364672      1052672
    PREV: 20    +3      0.069534 CALL   test.py #   9 create_string 14364672            0
    NEXT: 21    +4      0.070101 RETURN test.py #  10 create_string 15417344      1052672
    PREV: 24    +3      0.070120 CALL   test.py #   9 create_string 15417344            0
    NEXT: 25    +4      0.070663 RETURN test.py #  10 create_string 16470016      1052672
    PREV: 28    +3      0.070677 CALL   test.py #   9 create_string 16470016            0
    NEXT: 29    +4      0.071211 RETURN test.py #  10 create_string 17522688      1052672

So in this example events 2 and 3 are omitted as there is no change in the RSS.
Events 4 and 5 are included as there is a change in the RSS between them.

Logging Every Event
--------------------------------

If all events are needed then change the constructor argument to 0:

.. code-block:: python

    with cPyMemTrace.Profile(0):
        # As before

And the log file looks like this:

.. code-block:: text

          Event dEvent  Clock    What     File    #line Function      RSS           dRSS
    NEXT: 0     +0      0.079408 CALL     test.py #   9 create_string  9105408      9105408
    NEXT: 1     +1      0.079987 RETURN   test.py #  10 create_string 10158080      1052672
    NEXT: 2     +1      0.079994 C_CALL   test.py #  64 append        10158080            0
    NEXT: 3     +1      0.079998 C_RETURN test.py #  64 append        10158080            0
    NEXT: 4     +1      0.080003 CALL     test.py #   9 create_string 10158080            0
    NEXT: 5     +1      0.080682 RETURN   test.py #  10 create_string 11210752      1052672
    NEXT: 6     +1      0.080693 C_CALL   test.py #  64 append        11210752            0
    NEXT: 7     +1      0.080698 C_RETURN test.py #  64 append        11210752            0
    NEXT: 8     +1      0.080704 CALL     test.py #   9 create_string 11210752            0
    NEXT: 9     +1      0.081414 RETURN   test.py #  10 create_string 12263424      1052672
    NEXT: 10    +1      0.081424 C_CALL   test.py #  64 append        12263424            0
    NEXT: 11    +1      0.081429 C_RETURN test.py #  64 append        12263424            0
    NEXT: 12    +1      0.081434 CALL     test.py #   9 create_string 12263424            0
    NEXT: 13    +1      0.081993 RETURN   test.py #  10 create_string 13316096      1052672
    NEXT: 14    +1      0.081998 C_CALL   test.py #  64 append        13316096            0
    ...
    NEXT: 59    +1      0.084531 C_RETURN test.py #  66 pop           17526784            0
    NEXT: 60    +1      0.084535 C_CALL   test.py #  65 len           17526784            0
    NEXT: 61    +1      0.084539 C_RETURN test.py #  65 len           17526784            0
    NEXT: 62    +1      0.084541 C_CALL   test.py #  66 pop           17526784            0
    NEXT: 63    +1      0.084561 C_RETURN test.py #  66 pop           17526784            0
    NEXT: 64    +1      0.084566 C_CALL   test.py #  65 len           17526784            0
    NEXT: 65    +1      0.084568 C_RETURN test.py #  65 len           17526784            0

There is some discussion about the performance of ``cPyMemTrace`` here in the
`technical note on cPyMemTrace <https://pymemtrace.readthedocs.io/en/latest/tech_notes/cPyMemTrace.html>`_.
and some more
`cPyMemTrace code examples here <https://github.com/paulross/pymemtrace/blob/master/pymemtrace/examples/ex_cPyMemTrace.py>`_

.. index::
    single: pymemtrace; DTrace
    single: DTrace

``pymemtrace`` and DTrace
=========================

With an OS that supports DTrace (for example Mac OS X) ``pymemtrace`` provides some D scripts that support memory
profiling.
DTrace is an extremely powerful tool that can produce and enormous amount of detailed information on memory
allocations and de-allocations.

This is beyond the scope of *this* document however some examples are shown in `DTrace examples`_.
There is some discussion about the performance of ``DTrace`` here in the `technical note on DTrace`_
and some more
`DTrace code examples here <https://github.com/paulross/pymemtrace/blob/master/pymemtrace/examples/ex_dtrace.py>`_.

.. index::
    single: pymemtrace; Debug Malloc Stats
    single: Debug Malloc Stats
    single: sys._debugmallocstats()

``pymemtrace`` Debug Malloc Stats
=================================

CPython has the function the :py:func:`sys._debugmallocstats()` that can dump the status of the Python small object
memory allocator.
For example:

.. code-block:: python

    >>> import sys
    >>> sys._debugmallocstats()
    Small block threshold = 512, in 32 size classes.

    class   size   num pools   blocks in use  avail blocks
    -----   ----   ---------   -------------  ------------
        0     16           1              90           931
        1     32           2             830           190
        2     48           9            2916           144
        3     64          48           11539           701
        4     80          30            6076            44
        5     96           6             999            21
        6    112           4             454           126
        7    128           8             943            73
        8    144           2             188            38
        9    160          19            1915            23
       10    176           2             126            58
       11    192           2             100            70
       12    208           5             342            48
       13    224           4             277            11
       14    240           5             271            69
       15    256           4             189            63
       16    272           3             144            36
       17    288           3             125            43
       18    304           2              88            18
       19    320           2              78            24
       20    336           2              54            42
       21    352           2              50            42
       22    368           2              54            34
       23    384           2              49            35
       24    400           5             171            29
       25    416           2              40            38
       26    432           1              26            11
       27    448           1              28             8
       28    464           1              34             1
       29    480           1              28             6
       30    496           1              26             6
       31    512           2              41            21

    # arenas allocated total           =                    3
    # arenas reclaimed                 =                    0
    # arenas highwater mark            =                    3
    # arenas allocated current         =                    3
    3 arenas * 1048576 bytes/arena     =            3,145,728

    # bytes in allocated blocks        =            2,654,688
    # bytes in available blocks        =              322,672
    6 unused pools * 16384 bytes       =               98,304
    # bytes lost to pool headers       =                8,784
    # bytes lost to quantization       =               12,128
    # bytes lost to arena alignment    =               49,152
    Total                              =            3,145,728

    arena map counts
    # arena map mid nodes              =                    1
    # arena map bot nodes              =                    1

    # bytes lost to arena map root     =              262,144
    # bytes lost to arena map mid      =              262,144
    # bytes lost to arena map bot      =              131,072
    Total                              =              655,360

               55 free PyDictObjects * 48 bytes each =                2,640
               6 free PyFloatObjects * 24 bytes each =                  144
               80 free PyListObjects * 40 bytes each =                3,200
       3 free 1-sized PyTupleObjects * 32 bytes each =                   96
     814 free 2-sized PyTupleObjects * 40 bytes each =               32,560
      58 free 3-sized PyTupleObjects * 48 bytes each =                2,784
      20 free 4-sized PyTupleObjects * 56 bytes each =                1,120
       6 free 5-sized PyTupleObjects * 64 bytes each =                  384
       3 free 6-sized PyTupleObjects * 72 bytes each =                  216
       1 free 7-sized PyTupleObjects * 80 bytes each =                   80
       3 free 8-sized PyTupleObjects * 88 bytes each =                  264
       2 free 9-sized PyTupleObjects * 96 bytes each =                  192
     1 free 10-sized PyTupleObjects * 104 bytes each =                  104
     1 free 11-sized PyTupleObjects * 112 bytes each =                  112
     0 free 12-sized PyTupleObjects * 120 bytes each =                    0
     3 free 13-sized PyTupleObjects * 128 bytes each =                  384
     0 free 14-sized PyTupleObjects * 136 bytes each =                    0
     4 free 15-sized PyTupleObjects * 144 bytes each =                  576
     1 free 16-sized PyTupleObjects * 152 bytes each =                  152
     1 free 17-sized PyTupleObjects * 160 bytes each =                  160
     1 free 18-sized PyTupleObjects * 168 bytes each =                  168
     0 free 19-sized PyTupleObjects * 176 bytes each =                    0
     2 free 20-sized PyTupleObjects * 184 bytes each =                  368
    >>>

The drawback of this is that you really want to see the before and after snapshot when a particular operation is
performed and that means comparing quite verbose output.

``pymemtrace`` has a module ``debug_malloc_stats`` that can provide is a wrapper around the
:py:func:`sys._debugmallocstats` function which take snapshots of
memory before and after code execution and report the significant differences of the Python small object allocator.
It uses a text parser to show only the differences between before and after.
For example:

.. code-block:: python

    from pymemtrace import debug_malloc_stats

    print(f'example_debug_malloc_stats_for_documentation()')
    list_of_strings = []
    with debug_malloc_stats.DiffSysDebugMallocStats() as malloc_diff:
        for i in range(1, 9):
            list_of_strings.append(' ' * (i * 8))
    print(f'DiffSysDebugMallocStats.diff():')
    print(f'{malloc_diff.diff()}')

The output is:

.. code-block:: text

    example_debug_malloc_stats_for_documentation()
    DiffSysDebugMallocStats.diff():
    class   size   num pools   blocks in use  avail blocks
    -----   ----   ---------   -------------  ------------
        1     32          +1             +52           +74
        2     48          +0             +17           -17
        3     64          +0             +33           -33
        4     80          +1             +51            -1
        5     96          +2             +34           +50
        6    112          +0              +2            -2
        7    128          +0              +1            -1
       10    176          +0              +1            -1
       12    208          +0              +1            -1
       17    288          +0              +1            -1
       18    304          +0              +2            -2
       25    416          +0              +3            -3
       26    432          +0              +3            -3
       27    448          +0              +3            -3
       29    480          +0              +3            -3
       30    496          +0              +1            -1
       31    512          +0              +1            -1

    # bytes in allocated blocks        =              +19,904
    # bytes in available blocks        =               -3,808
    -4 unused pools * 4096 bytes       =              -16,384
    # bytes lost to pool headers       =                 +192
    # bytes lost to quantization       =                  +96

      -1 free 1-sized PyTupleObjects * 32 bytes each =                  -32
      +1 free 5-sized PyTupleObjects * 64 bytes each =                  +64
               +2 free PyDictObjects * 48 bytes each =                  +96
               -2 free PyListObjects * 40 bytes each =                  -80
             +1 free PyMethodObjects * 48 bytes each =                  +48

There are more examples in https://pymemtrace.readthedocs.io/en/latest/examples/debug_malloc_stats.html
and some more
`debug_malloc_stats code examples here <https://github.com/paulross/pymemtrace/blob/master/pymemtrace/examples/ex_debug_malloc_stats.py>`_

.. index::
    single: pymemtrace; tracemalloc
    single: tracemalloc

``pymemtrace`` tracemalloc
=================================

Python has the :py:mod:`tracemalloc`
(`documentation <https://docs.python.org/3/library/tracemalloc.html#module-tracemalloc>`_)
that can provide the following information:

- Trace where an object was allocated.
- Statistics on allocated memory blocks per filename and per line number:
  total size, number and average size of allocated memory blocks.
- Compute the differences between two snapshots to detect memory leaks.

However the :py:mod:`tracemalloc` also consumes memory so this can conceal what is really going on.

``pymemtrace.trace_malloc`` contains some utility wrappers around the :py:mod:`tracemalloc` module and
the can compensate for the memory used by :py:mod:`tracemalloc` module.

Using ``trace_malloc`` Directly
----------------------------------------

Adding 1Mb Strings
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Here is an example of adding 1Mb strings to a list under the watchful eye of :py:class:`trace_malloc.TraceMalloc`:

.. code-block:: python

    from pymemtrace import trace_malloc

    list_of_strings = []
    print(f'example_trace_malloc_for_documentation()')
    with trace_malloc.TraceMalloc('filename') as tm:
        for i in range(8):
            list_of_strings.append(' ' * 1024**2)
    print(f' tm.memory_start={tm.memory_start}')
    print(f'tm.memory_finish={tm.memory_finish}')
    print(f'         tm.diff={tm.diff}')
    for stat in tm.statistics:
        print(stat)

Typical output is:

.. code-block:: text

    example_trace_malloc_for_documentation()
     tm.memory_start=13072
    tm.memory_finish=13800
             tm.diff=8388692
    pymemtrace/examples/ex_trace_malloc.py:0: size=8194 KiB (+8193 KiB), count=16 (+10), average=512 KiB
    /Library/Frameworks/Python.framework/Versions/3.8/lib/python3.8/tracemalloc.py:0: size=6464 B (+504 B), count=39 (+10), average=166 B
    Documents/workspace/pymemtrace/pymemtrace/trace_malloc.py:0: size=3076 B (-468 B), count=10 (-1), average=308 B
    /Library/Frameworks/Python.framework/Versions/3.8/lib/python3.8/logging/__init__.py:0: size=16.3 KiB (-128 B), count=49 (-2), average=340 B
    /Library/Frameworks/Python.framework/Versions/3.8/lib/python3.8/abc.py:0: size=3169 B (+0 B), count=30 (+0), average=106 B
    /Library/Frameworks/Python.framework/Versions/3.8/lib/python3.8/posixpath.py:0: size=480 B (+0 B), count=1 (+0), average=480 B
    /Library/Frameworks/Python.framework/Versions/3.8/lib/python3.8/threading.py:0: size=168 B (+0 B), count=2 (+0), average=84 B
    /Library/Frameworks/Python.framework/Versions/3.8/lib/python3.8/_weakrefset.py:0: size=72 B (+0 B), count=1 (+0), average=72 B


To eliminate the lines that is caused by ``tracemalloc`` itself change the last two lines to:

.. code-block:: python

    for stat in tm.net_statistics:
        print(stat)

Which removes the line:

.. code-block:: text

    /Library/Frameworks/Python.framework/Versions/3.8/lib/python3.8/tracemalloc.py:0: size=6464 B (+504 B), count=39 (+10), average=166 B

Using ``trace_malloc`` as a Decorator
----------------------------------------

``trace_malloc`` provides a function decorator that can log the tracemalloc memory usage caused by execution a function.
For example:

.. code-block:: python

    from pymemtrace import trace_malloc

    @trace_malloc.trace_malloc_log(logging.INFO)
    def example_decorator_for_documentation(list_of_strings):
        for i in range(8):
            list_of_strings.append(create_string(1024**2))

    list_of_strings = []
    example_decorator_for_documentation(list_of_strings)

Would log something like the following:

.. code-block:: text

    2025-02-13 11:37:39,194 -   trace_malloc.py#87   - 10121 - (MainThread) - INFO     - TraceMalloc memory delta: 8,389,548 for "example_decorator_for_documentation()"

Here are more `examples <https://pymemtrace.readthedocs.io/en/latest/examples/trace_malloc.html>`_
and some more
`trace_malloc code examples here <https://github.com/paulross/pymemtrace/blob/master/pymemtrace/examples/ex_trace_malloc.py>`_

.. Example footnote [#]_.

.. rubric:: Footnotes

.. [#] This is obtained from ``int getpagesize(void);`` in ``#include <unistd.h>``.
