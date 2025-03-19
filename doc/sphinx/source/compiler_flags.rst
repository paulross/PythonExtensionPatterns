.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. index::
    single: Compiler Flags

=================================
Setting Compiler Flags
=================================

It is sometimes difficult to decide what flags to set for the compiler and the best advice is to use the same flags that
the version of Python you are using was compiled with. Here are a couple of ways to do that.

.. index::
    single: Compiler Flags; CLI

---------------------------------
From the Command Line
---------------------------------

In the Python install directory there is a `pythonX.Y-config` executable that can be used to extract the compiler flags
where X is the major version and Y the minor version. For example (output is wrapped here for clarity):

.. code-block:: sh

    $ which python3
    /Library/Frameworks/Python.framework/Versions/3.13/bin/python3
    $ python3 -VV
    Python 3.13.0b3 (v3.13.0b3:7b413952e8, Jun 27 2024, 09:57:31) [Clang 15.0.0 (clang-1500.3.9.4)]
    $ /Library/Frameworks/Python.framework/Versions/3.13/bin/python3-config --cflags
    -I/Library/Frameworks/Python.framework/Versions/3.13/include/python3.13
    -I/Library/Frameworks/Python.framework/Versions/3.13/include/python3.13
    -fno-strict-overflow -Wsign-compare -Wunreachable-code -fno-common -dynamic -DNDEBUG
    -g -O3 -Wall -arch arm64 -arch x86_64 -g


.. index::
    single: Compiler Flags; Programmatically
    single: sysconfig; importing

--------------------------------------------------
Programmatically from Within a Python Process
--------------------------------------------------

The ``sysconfig`` module contains information about the build environment for the particular version of Python:

.. code-block:: python

    >>> import sysconfig
    >>> sysconfig.get_config_var('CFLAGS')
    '-fno-strict-overflow -Wsign-compare -Wunreachable-code -fno-common -dynamic -DNDEBUG -g -O3 -Wall -arch arm64 -arch x86_64 -g'
    >>> import pprint
    >>> pprint.pprint(sysconfig.get_paths())
    {'data': '/Library/Frameworks/Python.framework/Versions/3.13',
     'include': '/Library/Frameworks/Python.framework/Versions/3.13/include/python3.13',
     'platinclude': '/Library/Frameworks/Python.framework/Versions/3.13/include/python3.13',
     'platlib': '/Library/Frameworks/Python.framework/Versions/3.13/lib/python3.13/site-packages',
     'platstdlib': '/Library/Frameworks/Python.framework/Versions/3.13/lib/python3.13',
     'purelib': '/Library/Frameworks/Python.framework/Versions/3.13/lib/python3.13/site-packages',
     'scripts': '/Library/Frameworks/Python.framework/Versions/3.13/bin',
     'stdlib': '/Library/Frameworks/Python.framework/Versions/3.13/lib/python3.13'}
    >>> sysconfig.get_paths()['include']
    '/Library/Frameworks/Python.framework/Versions/3.13/include/python3.13'

.. index::
    single: Compiler Flags; sysconfig
    single: sysconfig; CLI

--------------------------------------------------
From the Command Line using ``sysconfig``
--------------------------------------------------

This very verbose output will give you a complete picture of your environment:

.. code-block:: sh

    $ python3 -m sysconfig
    Platform: "macosx-10.13-universal2"
    Python version: "3.13"
    Current installation scheme: "posix_prefix"

    Paths:
        data = "/Library/Frameworks/Python.framework/Versions/3.13"
        include = "/Library/Frameworks/Python.framework/Versions/3.13/include/python3.13"
        platinclude = "/Library/Frameworks/Python.framework/Versions/3.13/include/python3.13"
        platlib = "/Library/Frameworks/Python.framework/Versions/3.13/lib/python3.13/site-packages"
        platstdlib = "/Library/Frameworks/Python.framework/Versions/3.13/lib/python3.13"
        purelib = "/Library/Frameworks/Python.framework/Versions/3.13/lib/python3.13/site-packages"
        scripts = "/Library/Frameworks/Python.framework/Versions/3.13/bin"
        stdlib = "/Library/Frameworks/Python.framework/Versions/3.13/lib/python3.13"

    Variables:
        ABIFLAGS = ""
        AC_APPLE_UNIVERSAL_BUILD = "0"
        AIX_BUILDDATE = "0"
        AIX_GENUINE_CPLUSPLUS = "0"
        ALIGNOF_LONG = "8"
        ALIGNOF_MAX_ALIGN_T = "8"
        ALIGNOF_SIZE_T = "8"
        ALT_SOABI = "0"
        ANDROID_API_LEVEL = "0"
        AR = "/usr/bin/xcrun ar"
        ...

.. index::
    single: Compiler Flags; setup.py

--------------------------------------------------
Setting Flags Automatically in ``setup.py``
--------------------------------------------------

The sysconfig module allows you to create a generic ``setup.py`` script for Python C extensions, something along these lines:

.. code-block:: python

    from distutils.core import setup, Extension
    import os
    import sysconfig

    _DEBUG = False
    # Generally I write code so that if DEBUG is defined as 0 then all optimisations
    # are off and asserts are enabled. Typically run times of these builds are x2 to x10
    # release builds.
    # If DEBUG > 0 then extra code paths are introduced such as checking the integrity of
    # internal data structures. In this case the performance is by no means comparable
    # with release builds.
    _DEBUG_LEVEL = 0

    # Common flags for both release and debug builds.
    extra_compile_args = sysconfig.get_config_var('CFLAGS').split()
    extra_compile_args += ["-std=c++11", "-Wall", "-Wextra"]
    if _DEBUG:
        extra_compile_args += ["-g3", "-O0", "-DDEBUG=%s" % _DEBUG_LEVEL, "-UNDEBUG"]
    else:
        extra_compile_args += ["-DNDEBUG", "-O3"]

    setup(
        name                = '...',
        version             = '...',
        author              = '...',
        author_email        = '...',
        maintainer          = '...',
        maintainer_email    = '...',
        description         = '...',
        long_description    = """...
    """,
        platforms           = ['Mac OSX', 'POSIX',],
        classifiers         = [
            '...',
        ],
        license             = 'GNU Lesser General Public License v2 or later (LGPLv2+)',
        ext_modules=[
            Extension("MyExtension",
                sources=[
                         '...',
                ],
                include_dirs=[
                    '.',
                    '...',
                    os.path.join(os.getcwd(), 'include'),
                    sysconfig.get_paths()['include'],
                ],
                library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
                extra_compile_args=extra_compile_args,
                language='c++11',
            ),
        ]
    )

.. index::
    pair: Compiler Flags; CMake

----------------------------------------
Getting C/C++ Flags from a CMake Build
----------------------------------------

If your project can be built under CMake (such as this one can be) then the compiler flags can be obtained by looking
at the generated file ``cmake-build-debug/CMakeFiles/<PROJECT>.dir/flags.make`` (debug) or
``cmake-build-release/CMakeFiles/<PROJECT>.dir/flags.make`` (release).

For example, for this project the file is ``cmake-build-debug/CMakeFiles/PythonExtensionPatterns.dir/flags.make``.

This looks something like this (wrapped for clarity and replaced user with <USER>) :

.. code-block:: text

    # CMAKE generated file: DO NOT EDIT!
    # Generated by "Unix Makefiles" Generator, CMake Version 3.28
    
    # compile C with \
    # /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc
    # compile CXX with \
    # /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++
    C_DEFINES = 
    
    C_INCLUDES = -I/Library/Frameworks/Python.framework/Versions/3.13/include/python3.13 \
        -I/Users/<USER>/CLionProjects/PythonExtensionPatterns/src/debugging/XcodeExample/PythonSubclassList/PythonSubclassList \
        -I/Users/<USER>/CLionProjects/PythonExtensionPatterns/src/cpy \
        -I/Users/<USER>/CLionProjects/PythonExtensionPatterns/src/cpy/Containers \
        -I/Users/<USER>/CLionProjects/PythonExtensionPatterns/src/cpy/Watchers
    
    C_FLAGSarm64 = -g -std=gnu11 -arch arm64 -isysroot \
        /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX13.3.sdk \
        -fcolor-diagnostics -Wall -Wextra -Wpedantic -Werror -Wfatal-errors \
        -Wno-unused-variable -Wno-unused-parameter -fexceptions \
        -Wno-c99-extensions -Wno-c++11-extensions -O0 -g3 -ggdb \
        -Wno-unused-function
    
    C_FLAGS = -g -std=gnu11 -arch arm64 -isysroot \
        /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX13.3.sdk \
        -fcolor-diagnostics -Wall -Wextra -Wpedantic -Werror -Wfatal-errors \
        -Wno-unused-variable -Wno-unused-parameter -fexceptions \
        -Wno-c99-extensions -Wno-c++11-extensions -O0 -g3 -ggdb \
        -Wno-unused-function
    
    CXX_DEFINES = 
    
    CXX_INCLUDES = -I/Library/Frameworks/Python.framework/Versions/3.13/include/python3.13 \
        -I/Users/<USER>/CLionProjects/PythonExtensionPatterns/src/debugging/XcodeExample/PythonSubclassList/PythonSubclassList \
        -I/Users/<USER>/CLionProjects/PythonExtensionPatterns/src/cpy \
        -I/Users/<USER>/CLionProjects/PythonExtensionPatterns/src/cpy/Containers \
        -I/Users/<USER>/CLionProjects/PythonExtensionPatterns/src/cpy/Watchers
    
    CXX_FLAGSarm64 = -g -std=gnu++17 -arch arm64 -isysroot \
        /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX13.3.sdk \
        -fcolor-diagnostics -Wall -Wextra -Wpedantic -Werror -Wfatal-errors \
        -Wno-unused-variable -Wno-unused-parameter -fexceptions \
        -Wno-c99-extensions -Wno-c++11-extensions -O0 -g3 -ggdb \
        -Wno-unused-function

    CXX_FLAGS = -g -std=gnu++17 -arch arm64 -isysroot \
        /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX13.3.sdk \
        -fcolor-diagnostics -Wall -Wextra -Wpedantic -Werror -Wfatal-errors \
        -Wno-unused-variable -Wno-unused-parameter -fexceptions \
        -Wno-c99-extensions -Wno-c++11-extensions -O0 -g3 -ggdb \
        -Wno-unused-function

This would be fairly easy to parse, perhaps by ``setup.py``.
