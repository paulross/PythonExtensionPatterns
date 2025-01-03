.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

============
Introduction
============

This projects explores reliable patterns of coding Python Extensions in C.
It covers the essentials of reference counts, exceptions and creating functions that are safe and efficient.

Writing Python C Extensions can be daunting; you have to cast aside the security and fluidity of Python and embrace C,
not just C but Pythons C API, which is huge [#]_ and changes between versions [#]_.
Not only do you have to worry about just your standard ``malloc()`` and ``free()`` cases but now you have to contend
with how CPython's does its memory management which is by *reference counting*.

I describe some of the pitfalls you (I am thinking of you as a savvy C coder) can encounter and some of the coding
patterns that you can use to avoid them.

This also might help if you are considering code to submit to the Python standard library which depends extensively on
C extensions.

.. index::
    single: Personal Note

---------------------
A Personal Note
---------------------

This project has its roots when, long ago, I joined a tech company that had created a large number of production
critical Python C Extensions.
These were all written by a single engineer who left shortly after I joined.
The CTO appointed me as their replacement on the dubious basis that I knew Python and C.

I really struggled to bring my knowledge of both languages to their very complicated, and crucial, codebase.
To be honest I don't think I did a great job, but as I was the 'owner' I somehow got away with it.

After about six months of anxiety it occurred to me that, rather learning from their scrappy CPython C code,
I asked myself "how would you write a Python C Extension from scratch?".
So on the commute and at weekends I did just that and slowly things became clearer.
This eventually lead me to being invited to PyConUS to give a talk about the subject.
This document is a synthesis of the latter journey which ended up giving me far more confidence about the subject than
during my earlier difficulties.

My fond hope is that you will find this document makes it much easier for you to work in this field.
Another way of saying that is that I dedicate this document to you, and your work.

So why write Python C Extensions?

---------------------
Firstly Why?
---------------------

There are several reasons why you might want to write a C extension:

^^^^^^^^^^^^^^^^^^^
Performance
^^^^^^^^^^^^^^^^^^^

This is the most compelling reason, a 50x or 100x improvement over pure Python is not unusual.

.. index::
    single: C/C++; Libraries

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Interface with C/C++ libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you have a library in C or C++ you will have to write a C extension to give a Python interface to that library.

.. index::
    single: Memory Usage

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Less memory
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Python is pretty memory hungry, for example a float in Python is 24 bytes, the corresponding double in C is 8 bytes.
C and C++ have more specific deallocation policies than with a garbage collected language.

.. index::
    single: GIL

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The GIL
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

C Extensions do not need the Global Interpreter Lock (GIL) when working with C/C++ code.

---------------------
Why Not?
---------------------

Like everything, there are disadvantages, here are some:

- C Extensions are something of a niche skill.
  Achieving and maintaining that skill has costs.
  Hopefully this project reduces those.
- Writing C Extensions is more time consuming than pure Python.
  There is also the intellectual problem that you are dealing with Pure C/CPython/Python code which expects
  a lot of context switching.
  This project proposes patterns of code that should reduce the cognitive overhead of all of that.
- Testing C Extensions, whilst excellent at a high level, can be really tricky at a line-of-code level.

------------------------------------
Alternatives to C Extensions
------------------------------------

There are several alternatives to writing an extension directly in C, here are some:

.. index::
    single: ctypes

^^^^^^^^^^^^^^^^^^^
``ctypes``
^^^^^^^^^^^^^^^^^^^

`ctypes <https://docs.python.org/3/library/ctypes.html#module-ctypes>`_ is a well documented foreign function library
for Python and is part of Python's standard library.
The module allows direct access to C/C++ libraries (such as ``libc``).
If you need this functionality, for example you need to access a binary library where you do not have the original
source code so you can not build the library into your own code.

.. index::
    single: Code Generators

^^^^^^^^^^^^^^^^^^^
Code Generators
^^^^^^^^^^^^^^^^^^^

There are a number of projects out there that take high level code and generate C/C++ code that can then be built into
a Python module.
Some examples are:

- `SWIG <https://swig.org>`_ is a very well established project.
  An advantage that distinguishes it from other projects is its multi-language support.
- `Cython <https://cython.org>`_ is another well established project.
  You write in Python-like pseudo code that is translated into C which is then compiled into a Python module.
  A notable feature is its excellent support for working with ``numpy``.
  If you are using Cython you might find another project of mine,
  `Notes on Cython <https://github.com/paulross/NotesOnCython>`_, useful.
- `PyBind11 <https://github.com/pybind/pybind11>`_ is an excellent and ingenious project that uses C++ template to do
  the bulk of the work in generating code for a Python module.

There are common drawbacks of code generators:

- The testing and debug story is generally poor.
- There is occasional pathological behaviour where a small change or version upgrade can introduce a large performance
  degradation.
- If you are crossing the boundary between the Python interpreter and compiled C/C++ at a high frequency, perhaps with
  many small objects, code generators can create a performance overhead compared to C extensions.
  An example is shown here with my project on `XML creation <https://github.com/paulross/xmlwriter>`_.


There are many other alternatives such as ``pypy``, ``numba`` that are worth knowing about.

.. _introduction_summary_advice:

------------------------------------
Summary Advice
------------------------------------

My advice if you are thinking about extensions:

- They can be really powerful, 100x powerful
- They can be expensive to write and maintain
- It helps to follow established patterns
- Write everything in Python, benchmark/profile before deciding what to put into CPython
- Use them for low level, stable, library code
- Keep the CPython layer as thin as possible
- Testing, testing testing!

Next up: a simple example showing the effect on code performance.

.. rubric:: Footnotes

.. [#] Huge, but pretty consistent once mastered.
.. [#] Version 0.2 of this project supports Python versions: 3.9, 3.10, 3.11, 3.12, 3.13.
