import pytest

from cPyExtPatt import cModuleGlobals


def test_int():
    assert cModuleGlobals.INT == 42


def test_str():
    assert cModuleGlobals.STR == 'String value'


def test_list():
    assert cModuleGlobals.LST == [66, 68, 73,]


def test_list_alter():
    assert cModuleGlobals.LST == [66, 68, 73,]
    cModuleGlobals.LST.append(100)
    assert cModuleGlobals.LST == [66, 68, 73, 100,]
    assert cModuleGlobals.LST.pop(-1) == 100
    assert cModuleGlobals.LST == [66, 68, 73,]


def test_tuple():
    assert cModuleGlobals.TUP == (66, 68, 73,)


def test_map():
    assert cModuleGlobals.MAP == {b'123': 123, b'66': 66}


def test_print():
    cModuleGlobals.print()
    # assert 0
