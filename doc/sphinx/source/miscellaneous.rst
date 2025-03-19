.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

.. _miscellaneous:

.. index::
    single: Miscellaneous

====================================
Miscellaneous
====================================

This chapter covers various miscellaneous issues that the author has found with creating Python Extensions over the
years.

------------------------------------
No ``PyInit_...`` Function Found  
------------------------------------

This is probably Mac OS X and Clang specific but when you import your extension and you get an error like:

``ImportError: dynamic module does not define module export function (PyInit_Foo)``

Have a look at the binary.

.. code-block:: sh

    $ nm -m Foo.cpython-36m-darwin.so | grep Init
    00000000000010d0 (__TEXT,__text) non-external (was a private external) _PyInit_Foo

Sometimes (why?) clang does not make the symbol external. I have found that adding ``__attribute__((visibility("default")))`` to the module initialisation function can fix this:

.. code-block:: sh

    __attribute__((visibility("default")))
    PyMODINIT_FUNC
    PyInit_Foo(void) {
        /* ... */
    }

And the binary now looks like this:

.. code-block:: sh

    $ nm -m Foo.cpython-36m-darwin.so | grep Init
    00000000000010d0 (__TEXT,__text) external _PyInit_Foo

.. _miscellaneous_migration_python_c:

---------------------------------------
Migrating from Python to a C Extension
---------------------------------------

Suppose you have followed my advice in :ref:`introduction_summary_advice` in that you write you code in Python first
then, when profiling shows the slow spots, rewrite in C.
You might not want to do this all at once so here is a technique that allows you to migrate to C in a flexible way, say
over a number of releases, without you users having to change *their* code.

Suppose you have a bunch of functions and classes in a Python module ``spam.py``.
Then take all that Python code an put it in a file, say, ``py_spam.py``.
Now create an empty C Extension calling it, say, ``c_spam``.

Change ``spam.py`` to be merely:

.. code-block:: python

    from py_spam import *
    from c_spam import *

Your users, including your test code, just uses ``import spam`` and they get all of ``py_spam`` for now and nothing
from ``c_spam`` as it is empty.

You can now, judiciously, add functionality and classes to ``c_spam`` and your users will automatically get those as
``spam`` overwrites the appropriate imports from ``py_spam`` with the ones from ``c_spam``.




