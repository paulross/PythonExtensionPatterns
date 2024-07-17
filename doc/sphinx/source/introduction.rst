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

Next up: understanding reference counts and Python's terminology.

.. rubric:: Footnotes

.. [#] Huge, but pretty consistent once mastered.
