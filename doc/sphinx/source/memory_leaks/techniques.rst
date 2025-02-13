.. moduleauthor:: Paul Ross <apaulross@gmail.com>
.. sectionauthor:: Paul Ross <apaulross@gmail.com>

.. index:: single: Memory Leaks; Techniques

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
* You should be able to create the test that shows that the leak is firstly not fixed, then fixed.

At the end of this you should be able to state:

* The *frequency* of the memory leak.
* The *severity* of the memory leak.

Relevant quote: **"Time spent on reconnaissance is seldom wasted."**


Using Platform Tools
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The high level investigation will usually concentrate on using platform tools such as builtin memory management tools or
Python tools such as  ``pymentrace``'s :ref:`chapter_memory_leaks.pymemtrace.proces` or ``psutil`` will prove useful.

Specific Tricks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TODO: Finish this.
