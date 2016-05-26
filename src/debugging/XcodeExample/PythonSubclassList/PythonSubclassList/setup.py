""" Usage:
python3 setup.py build

Created on May 30, 2013

@author: paulross
"""
import os

DEBUG = True

extra_compile_args=["-std=c99", "-Wall", "-Wextra"]
if DEBUG:
    extra_compile_args += ["-g3", "-O0", "-DDEBUG=1",]

from distutils.core import setup, Extension
setup(
    name                = 'Extending classes',
    version             = '0.1.0',
    author              = 'Paul Ross',
    author_email        = 'cpipdev@gmail.com',
    maintainer          = 'Paul Ross',
    maintainer_email    = 'cpipdev@gmail.com',
    description         = 'Example of extendig a class',
    long_description    = """""",
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
        'Topic :: Scientific/Engineering :: Data Processing',
    ],
    license             = 'GNU General Public License v2 (GPLv2)',
    ext_modules=[
        Extension(
            "ScList",
            sources=[
                'SubclassList.c',
                'py_call_super.c',
            ],
            include_dirs = ['.', '/usr/local/include',],
            library_dirs = [os.getcwd(),],
            #libraries = ['jpeg',],
            extra_compile_args=extra_compile_args,
        ),
    ]
)
