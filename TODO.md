# Update Python Extension Patterns 2024-06

This is a plan for a refresh of this project to version 0.2.0.

## Update Python Versions

See: https://devguide.python.org/versions/

Target 3.8 as near end of life?

At least Python 3.9 to 3.13

~~Remove any reference to Python 2, for example "Porting to Python 3".~~

## General Improvements

- Reorganise source code so that all C/C++ examples are included in source files.
- Make that all buildable.
- Different Python versions - use tox.ini?
- Add tests.
- Standardise code such as filenames, function names etc.

## Merge `PythonExtensionsBasic`

There is lots of good stuff here:

- ~~File handling such as `src/FileObjectToStdout/cFileObjectToStdout.c` and `src/FilePath/FilePath.cpp`~~
- ~~File wrapper between Python/C++ `src/PythonFile/PythonFileWrapper.h`~~
- ~~Capsules~~
- ~~C++ placement new.~~
- ~~Pickling~~
- Generators
- C++ snippets as testable code: ~~Unicode~~, .
- Buffer protocol (from RaPiVot).
- Index.

## Other Projects to Merge Here

- Buffer protocol support, XCode: `dev/Xcode/Python/PythonC++/PythonC++/python_buffprot.hpp`
- Examples with numpy/pillow.

## Rust

Interfacing with Rust.

Maybe rewrite the example Custom class from the Python documentation in Rust?

Link: https://github.com/PyO3/pyo3
Maturin: https://github.com/PyO3/maturin


## Reference Other Projects

- ~~The CPython Internals book (RealPython)~~
- ~~Python memory tracing: https://github.com/paulross/pymemtrace~~
- ~~Python/C++ homogeneous containers: https://github.com/paulross/PyCppContainers~~


## Other

Add section on using CMake.

## Announce

- On Python announce mailing list.
- Propose change to the Python dev page for Python versions: https://devguide.python.org/internals/exploring/#exploring


