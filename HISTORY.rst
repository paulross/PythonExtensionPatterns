=====================
History
=====================

0.3.1rc2 (TODO)
=====================

- Add Python 3.14 support.

  - This checks the build against Python 3.14 but does not include any Python 3.14 specific features.
  - Several tests that check reference counts are now conditional on the Python version >= 3.14.
  - There are reference count changes, as yet unexplained, but reflected in the tests.

- Add CMake support and example of debugging in CLion.
- Drop support for Python 3.8 and 3.9 (although they will likely work if you try).

0.3.1rc1 (2026-02-02)
=====================

- Comprehensive review and copy edit of all chapters.
- Add support for ``Py_AtExit()`` for allocation tracking.
- Add allocation tracking example code (``TracAllocs.h/.cpp``, ``cTrackAllocs.cpp``) and documentation.
- Split the container debugging code to individual files for each container.
- Add examples and tests for ``PyDict_Next()`` and ``PyDict_Merge()``.
- Minor fixes to super() code.
- Minor fixes to setup.py.
- Several fixes for gcc when building on Linux.

0.3.1rc0 (2025-04-01)
=====================

- Add a single index entry. This is documentation for 0.3.1rc0, 341 pages.
- Add link to compiler flags resources.
- Add explicit ``#include`` that was failing on some platforms.
- Include note on ml_flags failure on import.
- Suggest change for Py_SETREF().
- Add warning on Py_SETREF and Py_XSETREF. Fixes to Python 3 links.
- Fix issue #33: Sections in the "Parsing Python Arguments" chapter are of the wrong depth.

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
