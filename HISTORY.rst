=====================
History
=====================

0.3.1rc0 (TODO)
=====================

- Fix issue #33: Sections in the "Parsing Python Arguments" chapter are of the wrong deepth.

0.3.0 (2025-03-20)
=====================

Added Chapters
--------------

- "Containers and Reference Counts" which corrects the Python documentation where that is wrong, misleading or missing.
- "Struct Sequences (namedtuple in C)" which corrects the Python documentation where that is wrong, misleading or missing.
- "Context Managers" with practical C code examples.
- "Watchers" with practical examples for dictionary watchers (Python 3.12+).
- "Installation" for the project.
- "Source Code Layout" for the project.

Changed Chapters
----------------

- Update the "Homogeneous Python Containers and C++" chapter.
- Expand the "Memory Leaks" chapter.
- Extended the "Logging" chapter to show how to access the CPython Frame from C.
- Add "Emulating Sequence Types" to the "Creating New Types" chapter.
- Expand the Index.

Other
------

- Python versions supported: 3.9, 3.10, 3.11, 3.12, 3.13.
- Development Status :: 5 - Production/Stable
- The documentation content, example and test code has roughly doubled since version 0.2.2.
- PDF Documentation is 339 pages.

TODO
----

- Add "Debugging Python with CLion".

..
    .. todo::

        Update this history file.

0.2.2 (2024-10-21)
=====================

- Expand note on PyDict_SetItem(), PySet_Add() with code in src/cpy/RefCount/cRefCount.c and tests.

0.2.1 (2024-07-29)
=====================

- Python versions supported: 3.9, 3.10, 3.11, 3.12, 3.13 (possibly backwards compatible with Python 3.6, 3.7, 3.8)
- Almost all example code is built and tested against these Python versions.
- Added a chapter on managing file paths and files between Python and C.
- Added a chapter on subclassing from your classes or builtin classes.
- Added a chapter on pickling from C.
- Added a chapter on Capsules.
- Added a chapter on Iterators and Generators.
- Added a chapter on memory leaks and how to detect them.
- Added a chapter on thread safety.
- Update "Homogeneous Python Containers and C++" to refer to https://github.com/paulross/PyCppContainers
- All the documentation has been extensively reviewed and corrected where necessary.
- Development Status :: 5 - Production/Stable

Contributors
-------------------------

Many thanks!

Pull Requests
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- https://github.com/marioemmanuel
- https://github.com/miurahr
- https://github.com/gdevanla
- https://github.com/joelwhitehouse
- https://github.com/dhermes
- https://github.com/gst
- https://github.com/adamchainz
- https://github.com/nnathan


Issues
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- https://github.com/ngoldbaum
- https://github.com/niki-sp
- https://github.com/ldo
- https://github.com/1a1a11a
- https://github.com/congma

0.1.0 (2014-09-09)
=====================

- First release.
- Originally "Examples of reliable coding of Python 'C' extensions by Paul Ross.".
- Development Status :: 3 - Alpha
