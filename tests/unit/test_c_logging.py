import logging

import pytest

from cPyExtPatt.Logging import cLogging


def test_c_logging_dir():
    result = dir(cLogging)
    assert result == [
        'CRITICAL',
        'DEBUG',
        'ERROR',
        'EXCEPTION',
        'INFO',
        'WARNING',
        '__doc__',
        '__file__',
        '__loader__',
        '__name__',
        '__package__',
        '__spec__',
        'c_file_line_function',
        'log',
        'py_file_line_function',
    ]


def test_c_logging_log():
    result = cLogging.log(cLogging.CRITICAL, "Test log message")
    assert result is not None


def test_c_file_line_function_file():
    file, line, function = cLogging.c_file_line_function()
    assert file == 'src/cpy/Logging/cLogging.c'
    assert line == 129
    assert function == 'c_file_line_function'


def test_py_file_line_function_file():
    file, _line, _function = cLogging.py_file_line_function()
    assert file == __file__


def test_py_file_line_function_line():
    _file, line, _function = cLogging.py_file_line_function()
    assert line == 47


def test_py_file_line_function_function():
    _file, _line, function = cLogging.py_file_line_function()
    assert function == 'test_py_file_line_function_function'
