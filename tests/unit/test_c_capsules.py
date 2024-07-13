import pytest

from cPyExtPatt.Capsules import spam
from cPyExtPatt.Capsules import spam_capsule
from cPyExtPatt.Capsules import spam_client


def test_spam():
    result = spam.system("ls -l")
    assert result == 0


def test_spam_capsule():
    result = spam_capsule.system("ls -l")
    assert result == 0


def test_spam_capsule__C_API():
    print()
    print(spam_capsule._C_API)
    print(dir(spam_capsule._C_API))
    print(spam_capsule._C_API)
    print(type(spam_capsule._C_API).__mro__)
    assert 0


def test_spam_client():
    result = spam_client.system("ls -l")
    assert result == 0
