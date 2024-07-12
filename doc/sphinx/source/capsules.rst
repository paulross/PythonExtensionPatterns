.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

***************
Capsules
***************

Usually C extension code is used solely by the extension itself, for that reason all functions are declared ``static``
however there are cases where the C extension code is useful to another C extension.
When modules are used as shared libraries the symbols in one extension might not be visible to another extension.

`Capsules <https://docs.python.org/3/extending/extending.html#providing-a-c-api-for-an-extension-module>`_
are a means by which this can be achieved by passing C pointers from one extension module to another.



