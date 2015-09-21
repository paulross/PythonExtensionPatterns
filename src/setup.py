""" Usage:
python3 setup.py build

Created on May 30, 2013

@author: paulross
"""
import os

DEBUG = True

extra_compile_args=["-std=c99", ]
if DEBUG:
    extra_compile_args += ["-g3", "-O0", "-DDEBUG=1",]
else:
    extra_compile_args += ["-DNDEBUG", "-Os"]


from distutils.core import setup, Extension
setup(
    name                = 'cPyExtPatt',
    version             = '0.1.0',
    author              = 'Paul Ross',
    author_email        = 'cpipdev@gmail.com',
    maintainer          = 'Paul Ross',
    maintainer_email    = 'cpipdev@gmail.com',
    description         = 'Python Extension Patterns.',
    long_description    = """Examples of good and bad practice with Python Extensions.""",
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
    license             = 'GNU General Public License v2 (GPLv2)',
    ext_modules=[
        Extension("cExcep", sources=['cExcep.c',],
            include_dirs = ['/usr/local/include',], # os.path.join(os.getcwd(), 'include'),],
            library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
            extra_compile_args=extra_compile_args,
        ),
        Extension("cModuleGlobals", sources=['cModuleGlobals.c',],
            include_dirs = ['/usr/local/include',], # os.path.join(os.getcwd(), 'include'),],
            library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
            extra_compile_args=extra_compile_args,
        ),
        Extension("cObj", sources=['cObjmodule.c',],
            include_dirs = ['/usr/local/include',], # os.path.join(os.getcwd(), 'include'),],
            library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
            extra_compile_args=extra_compile_args,
        ),
        Extension("cParseArgs", sources=['cParseArgs.c',],
            include_dirs = ['/usr/local/include',], # os.path.join(os.getcwd(), 'include'),],
            library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
            extra_compile_args=extra_compile_args,
        ),
        Extension("cPyRefs", sources=['cPyRefs.c',],
            include_dirs = ['/usr/local/include',], # os.path.join(os.getcwd(), 'include'),],
            library_dirs = [os.getcwd(),],  # path to .a or .so file(s)
            #libraries = ['jpeg',],
            extra_compile_args=extra_compile_args,
        ),
    ]
)
