.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

=================================
Setting Compiler Flags
=================================

It is sometimes difficult to decide what flags to set for the compiler and the best advice is to use the same flags that the version of Python you are using was compiled with. Here are a couple of ways to do that.


---------------------------------
From the Command Line
---------------------------------

In the Python install directory there is a `pythonX.Y-config` executable that can be used to extract the compiler flags where X is the major version and Y the minor version. For example (output is wrapped here for clarity):

.. code-block:: sh

    $ which python
    /usr/bin/python
    $ python -V
    Python 2.7.5
    $ /usr/bin/python2.7-config --cflags
    -I/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
    -I/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
    -fno-strict-aliasing -fno-common -dynamic -arch x86_64 -arch i386 -g -Os -pipe
    -fno-common -fno-strict-aliasing -fwrapv -DENABLE_DTRACE -DMACOSX -DNDEBUG -Wall
    -Wstrict-prototypes -Wshorten-64-to-32 -DNDEBUG -g -fwrapv -Os -Wall
    -Wstrict-prototypes -DENABLE_DTRACE


--------------------------------------------------
Programatically from Within a Python Process
--------------------------------------------------

The ``sysconfig`` module contains information about the build environment for the particular version of Python:

.. code-block:: python

    >>> import sysconfig
    >>> sysconfig.get_config_var('CFLAGS')
    '-fno-strict-aliasing -fno-common -dynamic -arch x86_64 -arch i386 -g -Os -pipe -fno-common -fno-strict-aliasing -fwrapv -DENABLE_DTRACE -DMACOSX -DNDEBUG -Wall -Wstrict-prototypes -Wshorten-64-to-32 -DNDEBUG -g -fwrapv -Os -Wall -Wstrict-prototypes -DENABLE_DTRACE'
    >>> import pprint
    >>> pprint.pprint(sysconfig.get_paths())
    {'data': '/System/Library/Frameworks/Python.framework/Versions/2.7',
     'include': '/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7',
     'platinclude': '/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7',
     'platlib': '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages',
     'platstdlib': '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7',
     'purelib': '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages',
     'scripts': '/System/Library/Frameworks/Python.framework/Versions/2.7/bin',
     'stdlib': '/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7'}
    >>> sysconfig.get_paths()['include']
    '/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7'

--------------------------------------------------
From the Command Line using ``sysconfig``
--------------------------------------------------

This very verbose output will give you a complete picture of your environment:

.. code-block:: sh

    $ python3 -m sysconfig
    Platform: "macosx-10.6-intel"
    Python version: "3.4"
    Current installation scheme: "posix_prefix"

    Paths: 
        data = "/Library/Frameworks/Python.framework/Versions/3.4"
        include = "/Library/Frameworks/Python.framework/Versions/3.4/include/python3.4m"
        platinclude = "/Library/Frameworks/Python.framework/Versions/3.4/include/python3.4m"
        platlib = "/Library/Frameworks/Python.framework/Versions/3.4/lib/python3.4/site-packages"
        platstdlib = "/Library/Frameworks/Python.framework/Versions/3.4/lib/python3.4"
        purelib = "/Library/Frameworks/Python.framework/Versions/3.4/lib/python3.4/site-packages"
        scripts = "/Library/Frameworks/Python.framework/Versions/3.4/bin"
        stdlib = "/Library/Frameworks/Python.framework/Versions/3.4/lib/python3.4"

    Variables: 
        ABIFLAGS = "m"
        AC_APPLE_UNIVERSAL_BUILD = "1"
        AIX_GENUINE_CPLUSPLUS = "0"
        AR = "ar"
        ARFLAGS = "rc"
        ASDLGEN = "python /Users/sysadmin/build/v3.4.4/Parser/asdl_c.py"
        ASDLGEN_FILES = "/Users/sysadmin/build/v3.4.4/Parser/asdl.py /Users/sysadmin/build/v3.4.4/Parser/asdl_c.py"
        AST_ASDL = "/Users/sysadmin/build/v3.4.4/Parser/Python.asdl"
        AST_C = "Python/Python-ast.c"
        AST_C_DIR = "Python"
        AST_H = "Include/Python-ast.h"
        AST_H_DIR = "Include"
        BASECFLAGS = "-fno-strict-aliasing -fno-common -dynamic"
        BASECPPFLAGS = ""
        BASEMODLIBS = ""
        BINDIR = "/Library/Frameworks/Python.framework/Versions/3.4/bin"
        BINLIBDEST = "/Library/Frameworks/Python.framework/Versions/3.4/lib/python3.4"
    ...


--------------------------------------------------
Setting Flags Automatically in ``setup.py``
--------------------------------------------------

The sysconfig module allows you to create a generic ``setup.py`` script for Python C extensions (see highlighted line):

.. code-block:: python
    :emphasize-lines: 15

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
                ],
                library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
                extra_compile_args=extra_compile_args,
                language='c++11',
            ),
        ]
    )
