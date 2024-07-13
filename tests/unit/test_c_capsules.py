import pytest

from cPyExtPatt.Capsules import spam
from cPyExtPatt.Capsules import spam_capsule


def test_spam():
    result = spam.system("ls -l")
    assert result == 0


def test_spam_capsule():
    result = spam_capsule.system("ls -l")
    assert result == 0
