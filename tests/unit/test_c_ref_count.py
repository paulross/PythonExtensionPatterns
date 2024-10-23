import pytest

from cPyExtPatt import cRefCount


def test_module_dir():
    assert dir(cRefCount) == [
        '__doc__',
        '__file__',
        '__loader__',
        '__name__',
        '__package__',
        '__spec__',
        'dict_buildvalue_no_steals',
        'dict_no_steals',
        'dict_no_steals_decref',
        'list_buildvalue_steals',
        'list_steals',
        'set_no_steals',
        'set_no_steals_decref',
        'tuple_buildvalue_steals',
        'tuple_steals',
    ]


def test_c_ref_count_tuple_steals():
    assert cRefCount.tuple_steals() == 0


def test_c_ref_count_tuple_buildvalue_steals():
    assert cRefCount.tuple_buildvalue_steals() == 0


def test_c_ref_count_list_steals():
    assert cRefCount.list_steals() == 0


def test_c_ref_count_list_buildvalue_steals():
    assert cRefCount.list_buildvalue_steals() == 0


def test_c_ref_count_set_no_steals():
    assert cRefCount.set_no_steals() == 0


def test_c_ref_count_set_no_steals_decref():
    assert cRefCount.set_no_steals_decref() == 0


def test_c_ref_count_dict_no_steals():
    assert cRefCount.dict_no_steals() == 0


def test_c_ref_count_dict_no_steals_decref():
    assert cRefCount.dict_no_steals_decref() == 0


def test_c_ref_count_dict_buildvalue_no_steals():
    assert cRefCount.dict_buildvalue_no_steals() == 0
