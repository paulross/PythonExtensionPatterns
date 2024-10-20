# Update Python Extension Patterns 2024-06

This is a plan for a refresh of this project to version 0.2.0+.

~~strikethrough~~ means done.

## Update Python Versions

See: https://devguide.python.org/versions/

~~Target 3.8 as near end of life?~~ No.

~~At least Python 3.9 to 3.13.~~ Yes.

~~Remove any reference to Python 2, for example "Porting to Python 3".~~

## General Improvements

- ~~Reorganise source code so that all C/C++ examples are included in source files.~~
- ~~Make that all buildable.~~
- ~~Different Python versions - use tox.ini?~~ No, build_all.sh
- ~~Add tests.~~
- ~~Standardise code such as filenames, function names etc.~~
- ~~Improve the introduction with a discussion of reasons for doing this.~~
- ~~Add a 'simple example' section after the introduction.~~
- Standardise headings/subheadings etc.
- ~~Add sub-classing examples.~~
- ~~Review thread safety, add example code and tests.~~

## Merge `PythonExtensionsBasic`

There is lots of good stuff here:

- ~~File handling such as `src/FileObjectToStdout/cFileObjectToStdout.c` and `src/FilePath/FilePath.cpp`~~
- ~~File wrapper between Python/C++ `src/PythonFile/PythonFileWrapper.h`~~
- ~~Capsules~~
- ~~C++ placement new.~~
- ~~Pickling~~
- ~~Iterators and Generators.~~
- C++ snippets as testable code: ~~Unicode~~, .
- ~~Mark Buffer protocol (from RaPiVot) as TODO~~.
- ~~Index, with entries such as :index:`Unicode`:~~ Abandoned because Sphinx does not support this in a useful way.

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
- ~~The Python Cookbook https://www.amazon.co.uk/Python-Cookbook-David-Beazley/dp/1449340377/~~.

## Project

- ~~Resolve all issues on master.~~
- ~~Resolve all pull requests on master.~~
- ~~Rebase off master.~~
- ~~Complete README.md~~
- ~~Use README.md as `long_description` in `setup.py`.~~
- ~~Fix `long_description_content_type` in `setup.py` to `text/markdown` REST is `text/x-rst`.~~
- ~~Use HISTORY.rst in `setup.py`, where? Maybe convert to markdown and add to README.~~

~~Contributors:~~

~~https://github.com/nnathan - Section on Logging.~~

## Other

Add section on using CMake.

## Announce

- On Python announce mailing list.
- Propose change to the Python dev page for Python versions: https://devguide.python.org/internals/exploring/#exploring
