.. moduleauthor:: Paul Ross <apaulross@gmail.com>
.. sectionauthor:: Paul Ross <apaulross@gmail.com>

.. highlight:: python
    :linenothreshold: 30

.. toctree::
    :maxdepth: 3

..
    Links, mostly to the Python documentation.
    Specific container links are just before the appropriate section.

.. index::
    single: Installation

.. _chapter_installation:

======================================
Installation
======================================

This project is primarily
`a documentation project <http://pythonextensionpatterns.readthedocs.org/en/latest/index.html>`_
however it does contain a lot of code examples and tests.

Project Links
=============

- Source is `on GitHub <https://github.com/paulross/PythonExtensionPatterns>`_.
- Documentation `Read the Docs <http://pythonextensionpatterns.readthedocs.org/en/latest/index.html>`_.
- Project is `on PyPi <https://pypi.org/project/cPyExtPatt/>`_.

This code can be installed as follows.

Setup
=====
First make a virtual environment in your :file:`{<PYTHONVENVS>}`, say :file:`{~/pyvenvs}`:

.. code-block:: console

    $ python3 -m venv <PYTHONVENVS>/cPyExtPatt
    $ source <PYTHONVENVS>/cPyExtPatt/bin/activate
    (cPyExtPatt) $

Stable release
==============

To install ``cPyExtPatt``, run this command in your terminal:

.. code-block:: console

    $ pip install cPyExtPatt

This is the preferred method to install this project, as it will always install the most recent stable release.

If you don't have `pip`_ installed, this `Python installation guide`_ can guide
you through the process.

.. _pip: https://pip.pypa.io
.. _Python installation guide: http://docs.python-guide.org/en/latest/starting/installation/


From sources
============

The sources for cpip can be downloaded from the `Github repo`_.

You can clone the public repository:

.. code-block:: console

    (cPyExtPatt) $ git clone https://github.com/paulross/PythonExtensionPatterns.git

Install Requirements
====================

Then install the requirements in your virtual environment (these are pretty minimal):

.. code-block:: console

    (cPyExtPatt) $ pip install -r requirement.txt

Once you have a copy of the source, you can install it with:

.. code-block:: console

    (cPyExtPatt) $ python setup.py install

Or:

.. code-block:: console

    (cPyExtPatt) $ python setup.py develop

As you prefer.

What is in the Package
======================

Within the ``cPyExtPatt`` package are several modules.

Any module can be imported with:

.. code-block:: python

    from cPyExtPatt import <module>

The modules are:

=========================== =================================================================
Module                      Description
=========================== =================================================================
``cExceptions``             Examples of creating and using exceptions.
``cModuleGlobals``          Accessing module globals.
``cObject``                 Exploring ``PyObject``.
``cParseArgs``              Parsing Python function arguments in all their ways.
``cParseArgsHelper``        More parsing Python function arguments.
``cPyRefs``                 Exploring reference counts.
``cPickle``                 Pickling objects.
``cFile``                   Working with Python and C files.
``Capsules.spam``           A simple capsule.
``Capsules.spam_capsule``   A target capsule.
``Capsules.spam_client``    A client capsule.
``Capsules.datetimetz``       Using an existing capsule from Python's stdlib.
``cpp.placement_new``       Example of using C++ placement new.
``cpp.cUnicode``            Example of working with unicode to and from C++ and Python.
``SimpleExample.cFibA``     A simple example of a Python C extension.
``SimpleExample.cFibB``     A simple example of a Python C extension.
``Iterators.cIterator``     Iterators in C.
``SubClass.sublist``        Subclassing, in this case a list.
``Threads.csublist``        Illustrates thread contention in C.
``Threads.cppsublist``      Illustrates thread contention in C++.
``Logging.cLogging``        Examples of logging.
``cRefCount``               Reference count explorations.
``cCtxMgr``                 Example of a context manager.
``cStructSequence``         Example of a named tuple in C.
``cWatchers``               Example of a dictionary watcher in C.
=========================== =================================================================

In addition there are these modules availlable in Python 3.12+:

=========================== =================================================================
Module                      Description
=========================== =================================================================
``cWatchers``               Examples of watchers.
=========================== =================================================================

Running the Tests
====================

Then you should be able to run the tests:

.. code-block:: console

    (cPyExtPatt) $ pytest tests/
    ================================ test session starts ================================
    platform darwin -- Python 3.13.1, pytest-8.3.4, pluggy-1.5.0
    rootdir: /Users/engun/GitHub/paulross/PythonExtensionPatterns
    collected 275 items

    tests/unit/test_c_capsules.py ..s..................s.                         [  8%]
    tests/unit/test_c_cpp.py .............                                        [ 13%]
    tests/unit/test_c_ctxmgr.py .....                                             [ 14%]
    tests/unit/test_c_custom_pickle.py .....                                      [ 16%]
    tests/unit/test_c_exceptions.py .s...s......                                  [ 21%]
    tests/unit/test_c_file.py .....ss.......s                                     [ 26%]
    tests/unit/test_c_iterators.py .s.s...............                            [ 33%]
    tests/unit/test_c_logging.py ..                                               [ 34%]
    tests/unit/test_c_module_globals.py .......                                   [ 36%]
    tests/unit/test_c_object.py .s..s..sss.                                       [ 40%]
    tests/unit/test_c_parse_args.py ...s..s.......ss.....................sssssss. [ 57%]
    ..........ssss.....                                                           [ 64%]
    tests/unit/test_c_parse_args_helper.py .....                                  [ 65%]
    tests/unit/test_c_py_refs.py .....                                            [ 67%]
    tests/unit/test_c_ref_count.py s............................................. [ 84%]
    ..........                                                                    [ 88%]
    tests/unit/test_c_simple_example.py ..........                                [ 91%]
    tests/unit/test_c_struct_sequence.py .                                        [ 92%]
    tests/unit/test_c_subclass.py .s...                                           [ 93%]
    tests/unit/test_c_threads.py ..s.s............                                [100%]

    ========================= 242 passed, 33 skipped in 33.99s ==========================

The skipped tests are specific to a Python version that is not the current version in your virtual environment.

Building the Documentation
==========================

If you want to build the documentation you need to:

.. code-block:: console

    (cPyExtPatt) $ cd doc/sphinx
    (cPyExtPatt) $ make html latexpdf

This takes about 40 seconds from clean.

The landing page is *build/html/index.html* in *doc/sphinx*.
The PDF is *build/latex/PythonExtensionPatterns.pdf* in *doc/sphinx*.

Building Everything
==========================

At the project root there is a script ``build_all.sh`` which, for every supported version of Python:

- Builds and tests the C/C++ code.
- Creates a Python virtual environment (optionally deleting any existing one).
- Run ``pip install -r requirements.txt`` on the virtual environment.
- Run ``python setup.py develop`` in that virtual environment.
- Run ``pytest tests/``.
- Run ``python setup.py bdist_wheel``.
- Run ``python setup.py sdist``.
- Optionally, create the documentation.
- Report the results.

The script will halt on the first error returning the error code.

Takes about 70 seconds per Python version.

.. _Github repo: https://github.com/paulross/PythonExtensionPatterns
.. _zip: https://github.com/paulross/PythonExtensionPatterns/archive/refs/heads/master.zip
