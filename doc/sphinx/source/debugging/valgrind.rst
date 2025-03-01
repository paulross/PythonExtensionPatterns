.. highlight:: python
    :linenothreshold: 10

.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3


.. _valgrind-label:

.. index::
    single: Valgrind

===============================================
Valgrind
===============================================

This is about how to build Valgrind, a Valgrind friendly version of Python and finally how to use and interpret Valgrind.

.. note::

    These instructions have been tested on Mac OS X 10.9 (Mavericks).
    They may or may not work on other OS's

.. index::
    single: Valgrind; Building

---------------------------------
Building Valgrind
---------------------------------

This should be fairly straightforward:

.. code-block:: bash

    svn co svn://svn.valgrind.org/valgrind
    cd valgrind
    ./autogen.sh
    ./configure
    make
    make install

.. _building-python-for-valgrind-label:

.. index::
    single: Valgrind; Building Python For

---------------------------------
Building Python for Valgrind
---------------------------------


Prepare the source by uncommenting ``Py_USING_MEMORY_DEBUGGER`` in Objects/obmalloc.c around line 1082 or so.

^^^^^^^^^^^^^^
Configuring
^^^^^^^^^^^^^^

``configure`` takes the following aguments:

======================= ==================================================================
Argument
======================= ==================================================================
``--enable-framework``  Installs it in /Library/Frameworks/Python.framework/Versions/
``--with-pydebug``      Debug build of Python. See Misc/SpecialBuilds.txt
``--without-pymalloc``  With Valgrind support Misc/README.valgrind
======================= ==================================================================

To make a framework install:

.. code-block:: bash

    ./configure --enable-framework --with-pydebug --without-pymalloc --with-valgrind
    sudo make frameworkinstall


To make a local version cd to the source tree and we will build a Valgrind version of Python in the ``valgrind/`` directory:

.. code-block:: bash

    mkdir valgrind
    cd valgrind
    ../configure --with-pydebug --without-pymalloc --with-valgrind
    make

Check debug build
-----------------

.. code-block:: bash

    $ python3 -X showrefcount
    
.. code-block:: python

    Python 3.4.3 (default, May 26 2015, 19:54:01) 
    [GCC 4.2.1 Compatible Apple LLVM 6.0 (clang-600.0.51)] on darwin
    Type "help", "copyright", "credits" or "license" for more information.
    >>> 23
    23
    [54793 refs, 0 blocks]
    >>> import sys
    [54795 refs, 0 blocks]
    >>> sys.gettotalrefcount()
    54817
    [54795 refs, 0 blocks]
    >>> import sysconfig
    >>> sysconfig.get_config_var('Py_DEBUG')
    1

.. _using-valgrind-label:

.. index::
    single: Valgrind; Using

---------------------------------
Using Valgrind
---------------------------------

In the ``<Python source>/Misc`` directory there is a ``valgrind-python.supp`` file that supresses some Valgrind spurious warnings. I find that this needs editing so:

.. code-block:: bash

    cp <Python source>/Misc/valgrind-python.supp ~/valgrind-python.supp
    vi ~/valgrind-python.supp

Uncomment ``PyObject_Free`` and ``PyObject_Realloc`` in the valgrind suppression file.

Invoking the Python interpreter with Valgrind:

.. code-block:: bash

    valgrind --tool=memcheck --dsymutil=yes --track-origins=yes --show-leak-kinds=all --trace-children=yes --suppressions=$HOME/.valgrind-python.supp <Python source>/valgrind/python.exe -X showrefcount


An example of using Valgrind to detect leaks is here: :ref:`leaked-new-references-valgrind-label`.
