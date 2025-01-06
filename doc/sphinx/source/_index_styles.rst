
..
    Explore different index styles.

======================================
Index Styles
======================================

See: https://www.sphinx-doc.org/en/master/usage/restructuredtext/directives.html#directive-index

..
    .. index::
        single: execution; context
        pair: module; __main__
        pair: module; sys
        triple: module; search; path
        seealso: execution

----------------------
Inline Index Entries
----------------------

This is a normal reStructuredText :index:`paragraph` that contains several :index:`index entries <pair: index; entry>`.

.. raw:: latex

    [Continued on the next page]

    \pagebreak

.. index::
    single: execution

-----------------
Single execution
-----------------

``single: execution``.

.. raw:: latex

    [Continued on the next page]

    \pagebreak

.. index::
    single: execution; context

-------------------------
Single execution; context
-------------------------

``single: execution; context``.

.. raw:: latex

    [Continued on the next page]

    \pagebreak

.. index::
    pair: module; __main__
    pair: module; sys

-----------------
Pairs
-----------------

``pair: module; __main__``.

``pair: module; sys``.

.. raw:: latex

    [Continued on the next page]

    \pagebreak

.. index::
    triple: module; search; path

-----------------
Triple
-----------------

``triple: module; search; path``

.. raw:: latex

    [Continued on the next page]

    \pagebreak

.. index::
    see: execution; context

-------------------------
See execution; context
-------------------------

``see: execution; context``

.. raw:: latex

    [Continued on the next page]

    \pagebreak

.. index::
    seealso: execution; context

------------------------------
See Also execution; context
------------------------------

``seealso: execution; context``
