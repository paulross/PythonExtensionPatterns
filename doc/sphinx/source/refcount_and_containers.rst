.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _chapter_refcount_and_containers:

=====================================
Reference Counts An Python Containers
=====================================

Given the descriptions of *New*, *Stolen* and *Borrowed*
references described in the preceeding chapter, this chapter looks
in more detail of how the Python C API works with different containers.

Of particular interest is *Setters*, *Getters* and the behaviour of
``Py_BuildValue``.

Buckle up.

-----------------------
Tuple
-----------------------

The Python documentation for the `Tuple API <https://docs.python.org/3/c-api/tuple.html>`_.

.. list-table:: Tuple API
   :widths: 50 20 40
   :header-rows: 1

   * - Python C API
     - Behaviour
     - Notes
   * - ``PyTuple_SetItem()`` `Doc <https://docs.python.org/3/c-api/tuple.html#c.PyTuple_SetItem>`_
     - Steals, leaks original.
     - More stuff.
   * - ``PyTuple_SET_ITEM()`` `Doc <https://docs.python.org/3/c-api/tuple.html#c.PyTuple_SET_ITEM>`_
     - Steals, leaks original.
     - **Contrary** to the documentation this leaks.
   * - ``Py_BuildValue("(s)", val)``
     - Steals, leaks original.
     - More stuff.

-----------------------
List
-----------------------

-----------------------
Dictionary
-----------------------

-----------------------
Set
-----------------------

Example footnote [#]_.

.. rubric:: Footnotes

.. [#] A footnote.
