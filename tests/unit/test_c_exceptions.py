# >>> cExceptions.raise_error_silent()
# ValueError: ERROR: _raise_error_mixup()
#
# The above exception was the direct cause of the following exception:
#
# Traceback (most recent call last):
#   File "<stdin>", line 1, in <module>
# SystemError: <built-in function raise_error_silent> returned a result with an exception set
# >>> cExceptions.raise_error_silent_test()
# >>> ^D

import pytest

from cPyExtPatt import cExceptions


def test_raise_error():
    with pytest.raises(ValueError) as err:
        cExceptions.raise_error()
    assert err.value.args[0] == 'Ooops.'


def test_raise_error_bad():
    with pytest.raises(SystemError) as err:
        cExceptions.raise_error_bad()
    assert err.value.args[0] == '<built-in function raise_error_bad> returned NULL without setting an exception'


def test_raise_error_fmt():
    with pytest.raises(ValueError) as err:
        cExceptions.raise_error_fmt()
    assert err.value.args[0] == 'Can not read 12 bytes when offset 25 in byte length 32.'


def test_raise_error_overwrite():
    with pytest.raises(ValueError) as err:
        cExceptions.raise_error_overwrite()
    assert err.value.args[0] == 'ERROR: raise_error_overwrite()'


def test_raise_error_silent():
    with pytest.raises(SystemError) as err:
        cExceptions.raise_error_silent()
    assert err.value.args[0] == '<built-in function raise_error_silent> returned a result with an exception set'


def test_raise_error_silent_test():
    cExceptions.raise_error_silent_test()


def test_ExceptionBase_exists():
    exception = cExceptions.ExceptionBase('FOO')
    assert exception.args[0] == 'FOO'
    assert str(exception.__class__.__mro__) == (
        "("
        "<class 'cExceptions.ExceptionBase'>,"
        " <class 'Exception'>,"
        " <class 'BaseException'>,"
        " <class 'object'>"
        ")"
    )


def test_SpecialsiedError_exists():
    exception = cExceptions.SpecialisedError('FOO')
    assert exception.args[0] == 'FOO'
    assert str(exception.__class__.__mro__) == (
        "("
        "<class 'cExceptions.SpecialsiedError'>,"
        " <class 'cExceptions.ExceptionBase'>,"
        " <class 'Exception'>,"
        " <class 'BaseException'>,"
        " <class 'object'>"
        ")"
    )
