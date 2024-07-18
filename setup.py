""" Usage:
python3 setup.py build

Created on May 30, 2013

@author: paulross
"""
import os

import os
import pathlib
from setuptools import setup, Extension
import sysconfig

DEBUG = True
# Generally I write code so that if DEBUG is defined as 0 then all optimisations
# are off and asserts are enabled. Typically run times of these builds are x2 to x10
# release builds.
# If DEBUG > 0 then extra code paths are introduced such as checking the integrity of
# internal data structures. In this case the performance is by no means comparable
# with release builds.
DEBUG_LEVEL = 0

# Python stlib requirement:
LANGUAGE_STANDARD_C = "c99"
# Our level of C++
LANGUAGE_STANDARD_CPP = "c++11"

# Common flags for both release and debug builds.
# C
extra_compile_args_c = sysconfig.get_config_var('CFLAGS').split()
extra_compile_args_c += ["-std=%s" % LANGUAGE_STANDARD_C, "-Wall", "-Wextra"]
if DEBUG:
    extra_compile_args_c += ["-g3", "-O0", "-DDEBUG=%s" % DEBUG_LEVEL, "-UNDEBUG"]
else:
    extra_compile_args_c += ["-DNDEBUG", "-O3"]
# C++
extra_compile_args_cpp = sysconfig.get_config_var('CFLAGS').split()
extra_compile_args_cpp += ["-std=%s" % LANGUAGE_STANDARD_CPP, "-Wall", "-Wextra"]
if DEBUG:
    extra_compile_args_cpp += ["-g3", "-O0", "-DDEBUG=%s" % DEBUG_LEVEL, "-UNDEBUG"]
else:
    extra_compile_args_cpp += ["-DNDEBUG", "-O3"]

PYTHON_INCLUDE_DIRECTORIES = [
    sysconfig.get_paths()['include'],
]

PACKAGE_NAME = 'cPyExtPatt'

# Make directory cPyExtPatt/ and sub-directories such as Capsules/
for dir_path in (os.path.join(os.path.dirname(__file__), 'cPyExtPatt'),
                 os.path.join(os.path.dirname(__file__), 'cPyExtPatt', 'Capsules'),
                 os.path.join(os.path.dirname(__file__), 'cPyExtPatt', 'cpp'),
                 os.path.join(os.path.dirname(__file__), 'cPyExtPatt', 'SimpleExample'),
                 ):
    if not os.path.exists(dir_path):
        print(f'Making directory {dir_path}')
        os.makedirs(dir_path)
        pathlib.Path(os.path.join(dir_path, '__init__.py')).touch()

# For keywords see: https://setuptools.pypa.io/en/latest/references/keywords.html
setup(
    name=PACKAGE_NAME,
    version='0.2.0',
    author='Paul Ross',
    author_email='apaulross@gmail.com',
    maintainer='Paul Ross',
    maintainer_email='apaulross@gmail.com',
    description='Python C Extension Patterns.',
    long_description="""Examples of good and bad practice with Python C Extensions.""",
    long_description_content_type='text/plain',
    platforms=['Mac OSX', 'POSIX', ],
    packages=[PACKAGE_NAME, ],
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Environment :: Console',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: GNU General Public License v2 (GPLv2)',
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: C',
        'Programming Language :: C++',
        'Programming Language :: Python',
        'Topic :: Programming',
    ],
    licence='GNU General Public License v2 (GPLv2)',
    # See: https://setuptools.pypa.io/en/latest/userguide/ext_modules.html
    # language='c' or language='c++',
    ext_modules=[
        Extension(f"{PACKAGE_NAME}.cExceptions", sources=['src/cpy/cExceptions.c', ],
                  include_dirs=['/usr/local/include', ],  # os.path.join(os.getcwd(), 'include'),],
                  library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),
        Extension(f"{PACKAGE_NAME}.cModuleGlobals", sources=['src/cpy/cModuleGlobals.c', ],
                  include_dirs=['/usr/local/include', ],  # os.path.join(os.getcwd(), 'include'),],
                  library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),
        Extension(f"{PACKAGE_NAME}.cObject", sources=['src/cpy/cObject.c', ],
                  include_dirs=['/usr/local/include', ],  # os.path.join(os.getcwd(), 'include'),],
                  library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),
        Extension(f"{PACKAGE_NAME}.cParseArgs", sources=['src/cpy/cParseArgs.c', ],
                  include_dirs=['/usr/local/include', ],  # os.path.join(os.getcwd(), 'include'),],
                  library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),
        # Legacy code, see src/cpy/cParseArgsHelper.cpp for comments.
        Extension(f"{PACKAGE_NAME}.cParseArgsHelper", sources=['src/cpy/cParseArgsHelper.cpp', ],
                  include_dirs=['/usr/local/include', ],  # os.path.join(os.getcwd(), 'include'),],
                  library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
                  extra_compile_args=extra_compile_args_cpp,
                  language='c++11',
                  ),
        Extension(f"{PACKAGE_NAME}.cPyRefs", sources=['src/cpy/cPyRefs.c', ],
                  include_dirs=['/usr/local/include', ],  # os.path.join(os.getcwd(), 'include'),],
                  library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
                  # libraries = ['jpeg',],
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),
        Extension(f"{PACKAGE_NAME}.cPickle", sources=['src/cpy/Pickle/cCustomPickle.c', ],
                  include_dirs=['/usr/local/include', ],  # os.path.join(os.getcwd(), 'include'),],
                  library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),
        Extension(f"{PACKAGE_NAME}.cFile", sources=[
            'src/cpy/File/cFile.cpp',
            'src/cpy/File/PythonFileWrapper.cpp',
        ],
                  include_dirs=['/usr/local/include', 'src/cpy/File', ],  # os.path.join(os.getcwd(), 'include'),],
                  library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
                  extra_compile_args=extra_compile_args_cpp,
                  language='c++11',
                  ),
        Extension(f"{PACKAGE_NAME}.Capsules.spam", sources=['src/cpy/Capsules/spam.c', ],
                  include_dirs=['/usr/local/include', ],  # os.path.join(os.getcwd(), 'include'),],
                  library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),
        Extension(f"{PACKAGE_NAME}.Capsules.spam_capsule", sources=['src/cpy/Capsules/spam_capsule.c', ],
                  include_dirs=['/usr/local/include', 'src/cpy/Capsules', ],  # os.path.join(os.getcwd(), 'include'),],
                  library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),
        Extension(f"{PACKAGE_NAME}.Capsules.spam_client", sources=['src/cpy/Capsules/spam_client.c', ],
                  include_dirs=['/usr/local/include', 'src/cpy/Capsules', ],  # os.path.join(os.getcwd(), 'include'),],
                  library_dirs=[os.getcwd(), ],  # path to .a or .so file(s)
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),
        Extension(f"{PACKAGE_NAME}.Capsules.datetimetz",
                  sources=[
                      'src/cpy/Capsules/datetimetz.c',
                      'src/cpy/Util/py_call_super.c',
                  ],
                  include_dirs=[
                      '/usr/local/include',
                      'src/cpy/Capsules',
                      'src/cpy/Util',
                  ],
                  library_dirs=[os.getcwd(), ],
                  extra_compile_args=extra_compile_args_c,
                  language='c',
                  ),
        Extension(f"{PACKAGE_NAME}.cpp.placement_new",
                  sources=['src/cpy/cpp/placement_new.cpp', ],
                  include_dirs=['/usr/local/include', 'src/cpy/cpp', ],
                  library_dirs=[os.getcwd(), ],
                  extra_compile_args=extra_compile_args_cpp,
                  language='c++11',
                  ),
        Extension(f"{PACKAGE_NAME}.cpp.cUnicode",
                  sources=['src/cpy/cpp/cUnicode.cpp', ],
                  include_dirs=['/usr/local/include', ],
                  library_dirs=[os.getcwd(), ],
                  extra_compile_args=extra_compile_args_cpp,
                  language='c++11',
                  # undef_macros=undef_macros,
                  ),
        Extension(f"{PACKAGE_NAME}.SimpleExample.cFibA",
                  sources=['src/cpy/SimpleExample/cFibA.c', ],
                  include_dirs=[],
                  library_dirs=[],
                  libraries=[],
                  extra_compile_args=[
                      '-Wall', '-Wextra', '-Werror', '-Wfatal-errors', '-Wpedantic',
                      '-Wno-unused-function', '-Wno-unused-parameter',
                      '-Qunused-arguments', '-std=c99',
                      '-UDEBUG', '-DNDEBUG', '-Ofast', '-g',
                  ],
                  language='c',
                  ),
        Extension(f"{PACKAGE_NAME}.SimpleExample.cFibB",
                  sources=['src/cpy/SimpleExample/cFibB.c', ],
                  include_dirs=[],
                  library_dirs=[],
                  libraries=[],
                  # For best performance.
                  extra_compile_args=[
                      '-Wall', '-Wextra', '-Werror', '-Wfatal-errors', '-Wpedantic',
                      '-Wno-unused-function', '-Wno-unused-parameter',
                      '-Qunused-arguments', '-std=c99',
                      '-UDEBUG', '-DNDEBUG', '-Ofast', '-g',
                  ],
                  language='c',
                  ),
        # Extension(name=f"{PACKAGE_NAME}.Generators.gen_cpp",
        #           include_dirs=[],
        #           sources=["src/cpy/Generators/cGenerator.cpp", ],
        #           extra_compile_args=extra_compile_args_cpp,
        #           language='c++11',
        #           ),
    ]
)
