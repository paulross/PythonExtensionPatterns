.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

============
Introduction
============

Writing Python C Extensions can be daunting; you have to cast aside the security and fluidity of Python and embrace C,
not just C but Pythons C API, which is huge [#]_.
Not only do you have to worry about just your standard ``malloc()`` and ``free()`` cases but now you have to contend
with how CPython's does its memory management which is by *reference counting*.

I describe some of the pitfalls you (I am thinking of you as a savvy C coder) can encounter and some of the coding
patterns that you can use to avoid them.


---------------------
Firstly Why?
---------------------

There are several reasons why you might want to write a C extension:

^^^^^^^^^^^^^^^^^^^
Performance
^^^^^^^^^^^^^^^^^^^

This is the most compelling reason

50x or 100x improvement over pure Python is not unusual

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Interface with C/C++ libraries
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

C, but C++ (any version) works well.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Less memory
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A float in Python is 24 bytes, a double in C is 8 bytes.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The GIL
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Flee from the Global Interpreter Lock (GIL)

Against all of these there is the additional skill, time, and resulting complexity involved in writing C extensions.

------------------------------------
Alternatives to C Extensions
------------------------------------

There are several alternatives to writing an extension directly in C:

^^^^^^^^^^^^^^^^^^^
``ctypes``
^^^^^^^^^^^^^^^^^^^

`ctypes <https://docs.python.org/3/library/ctypes.html#module-ctypes>`_ is a well documented foreign function library
for Python and is part of Python's standard library.
The module allows direct access to C/C++ libraries (such as ``libc``).
If you need this functionality, for example you need to access a binary library where you do not have the original
source code so you can not build the library into your own module.

^^^^^^^^^^^^^^^^^^^
Code Generators
^^^^^^^^^^^^^^^^^^^

There are a number of projects out there that take high level code and generate C/C++ code that can then be built into
a Python module.
Some examples are:

- `SWIG <https://swig.org>`_ is a well established project.
  An advantage that distinguishes it from other projects is its multi-language support.
- `Cython <https://cython.org>`_ is another well established project.
  You write in Python-like pseudo code that is translated into C which is then compiled into a Python module.
  A notable feature is its excellent support for interfacing efficiently with ``numpy``.
  If you are using Cython you might find another project of mine,
  `Notes on Cython <https://github.com/paulross/NotesOnCython>`_, useful.
- `PyBibd11 <https://github.com/pybind/pybind11>`_ is an excellent and ingenious project that uses C++ template to do
  the bulk of the work in generating code for a Python module.

There are common drawbacks of code generators:

- The testing and debug story is generally poor.
- There is occasional pathological behaviour where a small change or version upgrade can introduce a large performance
  degradation.
- If you are crossing the boundary between the Python interpreter and compiled C/C++ at a high frequency, perhaps with
  many small objects, code generators can create a performance overhead compared to C extensions.
  An example is shown here with my project on `XML creation <https://github.com/paulross/xmlwriter>`_.


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
