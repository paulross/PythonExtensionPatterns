.. highlight:: python
    :linenothreshold: 10

.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. index::
    single: Debugging; Tools

=================================
Debugging Tools
=================================

First create your toolbox, in this one we have:

* Debug version of Python - great for finding out more detail of your Python code as it executes.
* Valgrind - the goto tool for memory leaks. It is a little tricky to get working but should be in every developers toolbox.
* OS memory monitioring - this is a quick and simple way of identifying whether memory leaks are happening or not.
  An example is given below: :ref:`simple-memory-monitor-label`

.. _debug-tools-debug-python-label:

.. index::
    single: Debugging; A Debug Python Version

------------------------------------------------
Build a Debug Version of Python
------------------------------------------------

There are a large combination of debug builds of Python that you can create and each one will give you extra information when you either:

* Invoke Python with a command line option.

  * Example: a ``Py_DEBUG`` build invoking Python with ``python -X showrefcount``

* Set an environment variable.

  * Example: a ``Py_DEBUG`` build invoking Python with ``PYTHONMALLOCSTATS=1 python``

* Additional functions that are added to the ``sys`` module that can give useful information.

  * Example: a ``Py_DEBUG`` build an calling ``sys.getobjects(...)``.


See here :ref:`debug-version-of-python-label` for instructions on how to do this.

.. _debug-tools-valgrind-label:

.. index::
    single: Debugging; Valgrind

------------------------------------------------
Valgrind
------------------------------------------------

See here :ref:`building-python-for-valgrind-label` for instructions on how to build Valgrind.

See here :ref:`using-valgrind-label` for instructions on how to use Valgrind.

Here :ref:`leaked-new-references-valgrind-label` is an example of finding a leak with Valgrind.


.. _simple-memory-monitor-label:

.. index::
    single: Debugging; Memory Monitor
    single: Memory Monitor
    see: Memory Monitor; pymemtrace
    see: pymemtrace; Memory Monitor

------------------------------------------------
A Simple Memory Monitor
------------------------------------------------

A useful technique is to monitor the memory usage of a Python program.
Here is a simple process memory monitor using the ``psutil`` library.
See the :ref:`memory_leaks-label` chapter for a more comprehensive approach, in particular
:ref:`memory-leaks.pymemtrace`.


.. code-block:: python

    import sys
    import time

    import psutil

    def memMon(pid, freq=1.0):
        proc = psutil.Process(pid)
        print(proc.memory_info_ex())
        prev_mem = None
        while True:
            try:
                mem = proc.memory_info().rss / 1e6
                if prev_mem is None:
                    print('{:10.3f} [Mb]'.format(mem))
                else:
                    print('{:10.3f} [Mb] {:+10.3f} [Mb]'.format(mem, mem - prev_mem))
                prev_mem = mem
                time.sleep(freq)
            except KeyboardInterrupt:
                try:
                    input(' Pausing memMon, <cr> to continue, ^C to end...')
                except KeyboardInterrupt:
                    print('\n')
                    return

    if __name__ == '__main__':
        if len(sys.argv) < 2:
            print('Usage: python pidmon.py <PID>')
            sys.exit(1)
        pid = int(sys.argv[1])
        memMon(pid)
        sys.exit(0)

Lets test it. In one shell fire up Python and find its PID::

    >>> import os
    >>> os.getpid()
    13360

In a second shell fire up pidmon.py with this PID:

.. code-block:: bash

    $ python3 pidmon.py 13360
    pextmem(rss=7364608, vms=2526482432, pfaults=9793536, pageins=24576)
         7.365 [Mb]
         7.365 [Mb]     +0.000 [Mb]
         7.365 [Mb]     +0.000 [Mb]
    ...

Pause pidmon.py with Ctrl-C:

.. code-block:: bash

    ^C Pausing memMon, <cr> to continue, ^C to end...

Go back to the first shell and create a large string (1Gb)::

    >>> s = ' ' * 1024**3

In the second shell continue pidmon.py with <cr> and we see the memory usage:

.. code-block:: bash

      1077.932 [Mb]  +1070.567 [Mb]
      1077.932 [Mb]     +0.000 [Mb]
      1077.932 [Mb]     +0.000 [Mb]
      ...

Go back to the first shell and delete the string::

    >>> del s

In the second shell we see the memory usage drop:

.. code-block:: bash

      1077.953 [Mb]     +0.020 [Mb]
      1077.953 [Mb]     +0.000 [Mb]
       272.679 [Mb]   -805.274 [Mb]
         4.243 [Mb]   -268.435 [Mb]
         4.243 [Mb]     +0.000 [Mb]
      ...

In the second shell halt pidmon with two Ctrl-C commands:

.. code-block:: bash

    ^C Pausing memMon, <cr> to continue, ^C to end...^C

So we can observe the total memory usage of another process simply and cheaply. This is often the first test to do when examining processes for memory leaks.
