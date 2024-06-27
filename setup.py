""" Usage:
python3 setup.py build

Created on May 30, 2013

@author: paulross
"""
import os

from distutils.core import setup, Extension
import os
import sysconfig

DEBUG = False
# Generally I write code so that if DEBUG is defined as 0 then all optimisations
# are off and asserts are enabled. Typically run times of these builds are x2 to x10
# release builds.
# If DEBUG > 0 then extra code paths are introduced such as checking the integrity of
# internal data structures. In this case the performance is by no means comparable
# with release builds.
DEBUG_LEVEL = 0

# Python stlib requirement:
LANGUAGE_STANDARD = "c99"
# Our level of C++
#LANGUAGE_STANDARD = "c++11"

# Common flags for both release and debug builds.
extra_compile_args = sysconfig.get_config_var('CFLAGS').split()
extra_compile_args += ["-std=%s" % LANGUAGE_STANDARD, "-Wall", "-Wextra"]
if DEBUG:
    extra_compile_args += ["-g3", "-O0", "-DDEBUG=%s" % DEBUG_LEVEL, "-UNDEBUG"]
else:
    extra_compile_args += ["-DNDEBUG", "-O3"]

PACKAGE_NAME = 'cPyExtPatt'

from distutils.core import setup, Extension
setup(
    name                = PACKAGE_NAME,
    version             = '0.2.0',
    author              = 'Paul Ross',
    author_email        = 'apaulross@gmail.com',
    maintainer          = 'Paul Ross',
    maintainer_email    = 'apaulross@gmail.com',
    description         = 'Python C Extension Patterns.',
    long_description    = """Examples of good and bad practice with Python C Extensions.""",
    platforms           = ['Mac OSX', 'POSIX',],
    classifiers         = [
        'Development Status :: 3 - Alpha',
        'Environment :: Console',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU General Public License v2 (GPLv2)',
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: C',
        'Programming Language :: Python',
        'Topic :: Programming',
    ],
    licence = 'GNU General Public License v2 (GPLv2)',
    ext_modules=[
        Extension(f"{PACKAGE_NAME}.cExceptions", sources=['src/cpy/cExceptions.c',],
            include_dirs = ['/usr/local/include',], # os.path.join(os.getcwd(), 'include'),],
            library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
            extra_compile_args=extra_compile_args,
        ),
        Extension(f"{PACKAGE_NAME}.cModuleGlobals", sources=['src/cpy/cModuleGlobals.c',],
            include_dirs = ['/usr/local/include',], # os.path.join(os.getcwd(), 'include'),],
            library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
            extra_compile_args=extra_compile_args,
        ),
        Extension(f"{PACKAGE_NAME}.cObject", sources=['src/cpy/cObject.c',],
            include_dirs = ['/usr/local/include',], # os.path.join(os.getcwd(), 'include'),],
            library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
            extra_compile_args=extra_compile_args,
        ),
        # Extension(f"{PACKAGE_NAME}.cParseArgs", sources=['src/cpy/cParseArgs.c',],
        #     include_dirs = ['/usr/local/include',], # os.path.join(os.getcwd(), 'include'),],
        #     library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
        #     extra_compile_args=extra_compile_args,
        # ),
        # Extension(f"{PACKAGE_NAME}.cPyRefs", sources=['src/cpy/cPyRefs.c',],
        #     include_dirs = ['/usr/local/include',], # os.path.join(os.getcwd(), 'include'),],
        #     library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
        #     #libraries = ['jpeg',],
        #     extra_compile_args=extra_compile_args,
        # ),
    ]
)
