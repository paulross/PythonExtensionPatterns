import datetime
import sys
import zoneinfo

import pytest

from cPyExtPatt.cpp import CppCtorDtorInPyObject


def test_placement_new():
    obj = CppCtorDtorInPyObject()
    del(obj)


