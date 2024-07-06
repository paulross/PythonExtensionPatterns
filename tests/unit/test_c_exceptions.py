import sys

import pytest

from cPyExtPatt import cExceptions


def test_raise_error():
    with pytest.raises(ValueError) as err:
        cExceptions.raise_error()
    assert err.value.args[0] == 'Ooops.'


@pytest.mark.skipif(sys.version_info.minor > 9, reason='Python <= 3.9')
def test_raise_error_bad_old():
    with pytest.raises(SystemError) as err:
        cExceptions.raise_error_bad()
    assert err.value.args[0] == '<built-in function raise_error_bad> returned NULL without setting an error'


@pytest.mark.skipif(sys.version_info.minor <= 9, reason='Python > 3.9')
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


@pytest.mark.skipif(sys.version_info.minor > 9, reason='Python <= 3.9')
def test_raise_error_silent_old():
    with pytest.raises(SystemError) as err:
        cExceptions.raise_error_silent()
    assert err.value.args[0] == '<built-in function raise_error_silent> returned a result with an error set'


@pytest.mark.skipif(sys.version_info.minor <= 9, reason='Python > 3.9')
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


def test_raise_exception_base():
    with pytest.raises(cExceptions.ExceptionBase) as err:
        cExceptions.raise_exception_base()
    assert err.value.args[0] == 'One 1 two 2 three 3.'


def test_raise_specialised_error():
    with pytest.raises(cExceptions.SpecialisedError) as err:
        cExceptions.raise_specialised_error()
    assert err.value.args[0] == 'One 1 two 2 three 3.'
