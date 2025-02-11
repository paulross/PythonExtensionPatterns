Techniques
====================================

This describes some of the techniques I have found useful.
Bear in mind:

* Tracking down memory leaks can take a long, long time.
* Every memory leak is its own special little snowflake!
  So what works will be situation specific.

High Level
------------------

It is worth spending a fair bit of time at high level before diving into the code since:

* Working at high level is relatively cheap.
* It is usually non-invasive.
* It will quickly find out the *scale* of the problem.
* It will quickly find out the *repeatability* of the problem.
* You should be able to create the test that shows that the leak is **fixed**.

At the end of this you should be able to state:

* The *frequency* of the memory leak.
* The *severity* of the memory leak.

Relevant quote: **"Time spent on reconnaissance is seldom wasted."**


Using Platform Tools
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The high level investigation will usually concentrate on using platform tools such as builtin memory management tools or
Python tools such as  ``pymentrace``'s :ref:`examples-process` or ``psutil`` will prove useful.


Specific Tricks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TODO: Finish this.

Turn the GC Off
"""""""""""""""""""""

Turning the garbage collector off with ``gc.disable()`` is worth trying to see what effect, if any, it has.

Medium Level
------------------

TODO: Finish this.

Information From the ``sys`` Module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``pymentrace``'s :ref:`examples-debug_malloc_stats` is a very useful wrapper around
:py:func:`sys._debugmallocstats` which can report changes to Python's small object allocator.


``tracemalloc``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``pymentrace``'s :ref:`examples-trace_malloc` is a very useful wrapper around
:py:mod:`tracemalloc` which can report changes to Python's memory allocator.

``objgraph``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TODO: Finish this.


Specific Tricks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TODO: Finish this.

Finding Which Python Objects are Holding References to an Object
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

TODO: Finish this.

C/C++ Increasing Reference Count Excessively
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

TODO: Finish this.

Low Level
------------------

TODO: Finish this