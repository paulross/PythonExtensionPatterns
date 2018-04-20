.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

====================================
Miscellaneous
====================================

------------------------------------
No ``PyInit`` Function Found  
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

    $ nm -m cXmlWrite.cpython-36m-darwin.so | grep Init
    00000000000010d0 (__TEXT,__text) external _PyInit_cXmlWrite
