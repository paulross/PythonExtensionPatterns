import logging

import pytest

from cPyExtPatt.Logging import cLogging


def test_c_logging_dir():
    result = dir(cLogging)
    assert result == ['CRITICAL',
                      'DEBUG',
                      'ERROR',
                      'EXCEPTION',
                      'FATAL',
                      'INFO',
                      'WARNING',
                      '__doc__',
                      '__file__',
                      '__loader__',
                      '__name__',
                      '__package__',
                      '__spec__',
                      'log']


def test_c_logging_log():
    result = cLogging.log(cLogging.CRITICAL, "Test log message")
    assert result is None
