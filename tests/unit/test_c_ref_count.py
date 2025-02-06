import sys

import pytest

from cPyExtPatt import cRefCount


@pytest.mark.skipif(not (sys.version_info.minor < 13), reason='Python pre 3.13')
def test_module_dir_pre_3_13():
    assert dir(cRefCount) == [
        '__doc__',
        '__file__',
        '__loader__',
        '__name__',
        '__package__',
        '__spec__',
        'dict_buildvalue_no_steals',
        'dict_no_steals',
        'dict_no_steals_decref_after_set',
        'list_buildvalue_steals',
        'list_steals',
        'set_no_steals',
        'set_no_steals_decref',
        'test_PyDict_GetItem',
        'test_PyDict_SetDefault_default_unused',
        'test_PyDict_SetDefault_default_used',
        'test_PyDict_SetItem_fails_not_a_dict',
        'test_PyDict_SetItem_fails_not_hashable',
        'test_PyDict_SetItem_increments',
        'test_PyList_Append',
        'test_PyList_Append_fails_NULL',
        'test_PyList_Append_fails_not_a_list',
        'test_PyList_Insert',
        'test_PyList_Insert_Is_Truncated',
        'test_PyList_Insert_Negative_Index',
        'test_PyList_Insert_fails_NULL',
        'test_PyList_Insert_fails_not_a_list',
        'test_PyList_Py_BuildValue',
        'test_PyList_SET_ITEM_NULL',
        'test_PyList_SET_ITEM_NULL_SET_ITEM',
        'test_PyList_SET_ITEM_replace_same',
        'test_PyList_SET_ITEM_steals',
        'test_PyList_SET_ITEM_steals_replace',
        'test_PyList_SetIem_NULL_SetItem',
        'test_PyList_SetItem_NULL',
        'test_PyList_SetItem_fails_not_a_list',
        'test_PyList_SetItem_fails_out_of_range',
        'test_PyList_SetItem_replace_same',
        'test_PyList_SetItem_steals',
        'test_PyList_SetItem_steals_replace',
        'test_PySet_Add',
        'test_PySet_Discard',
        'test_PySet_Pop',
        'test_PyTuple_Py_BuildValue',
        'test_PyTuple_Py_PyTuple_Pack',
        'test_PyTuple_SET_ITEM_NULL',
        'test_PyTuple_SET_ITEM_NULL_SET_ITEM',
        'test_PyTuple_SET_ITEM_replace_same',
        'test_PyTuple_SET_ITEM_steals',
        'test_PyTuple_SET_ITEM_steals_replace',
        'test_PyTuple_SetIem_NULL_SetItem',
        'test_PyTuple_SetItem_NULL',
        'test_PyTuple_SetItem_fails_not_a_tuple',
        'test_PyTuple_SetItem_fails_out_of_range',
        'test_PyTuple_SetItem_replace_same',
        'test_PyTuple_SetItem_steals',
        'test_PyTuple_SetItem_steals_replace',
        'tuple_buildvalue_steals',
        'tuple_steals'
    ]


@pytest.mark.skipif(not (sys.version_info.minor >= 13), reason='Python 3.13+')
def test_module_dir_3_13():
    assert dir(cRefCount) == [
        '__doc__',
        '__file__',
        '__loader__',
        '__name__',
        '__package__',
        '__spec__',
        'dict_buildvalue_no_steals',
        'dict_no_steals',
        'dict_no_steals_decref_after_set',
        'list_buildvalue_steals',
        'list_steals',
        'set_no_steals',
        'set_no_steals_decref',
        'test_PyDict_GetItem',
        'test_PyDict_Pop_key_absent',
        'test_PyDict_Pop_key_present',
        'test_PyDict_SetDefaultRef_default_unused',
        'test_PyDict_SetDefaultRef_default_used',
        'test_PyDict_SetDefault_default_unused',
        'test_PyDict_SetDefault_default_used',
        'test_PyDict_SetItem_fails_not_a_dict',
        'test_PyDict_SetItem_fails_not_hashable',
        'test_PyDict_SetItem_increments',
        'test_PyList_Append',
        'test_PyList_Append_fails_NULL',
        'test_PyList_Append_fails_not_a_list',
        'test_PyList_Insert',
        'test_PyList_Insert_Is_Truncated',
        'test_PyList_Insert_Negative_Index',
        'test_PyList_Insert_fails_NULL',
        'test_PyList_Insert_fails_not_a_list',
        'test_PyList_Py_BuildValue',
        'test_PyList_SET_ITEM_NULL',
        'test_PyList_SET_ITEM_NULL_SET_ITEM',
        'test_PyList_SET_ITEM_replace_same',
        'test_PyList_SET_ITEM_steals',
        'test_PyList_SET_ITEM_steals_replace',
        'test_PyList_SetIem_NULL_SetItem',
        'test_PyList_SetItem_NULL',
        'test_PyList_SetItem_fails_not_a_list',
        'test_PyList_SetItem_fails_out_of_range',
        'test_PyList_SetItem_replace_same',
        'test_PyList_SetItem_steals',
        'test_PyList_SetItem_steals_replace',
        'test_PySet_Add',
        'test_PySet_Discard',
        'test_PySet_Pop',
        'test_PyTuple_Py_BuildValue',
        'test_PyTuple_Py_PyTuple_Pack',
        'test_PyTuple_SET_ITEM_NULL',
        'test_PyTuple_SET_ITEM_NULL_SET_ITEM',
        'test_PyTuple_SET_ITEM_replace_same',
        'test_PyTuple_SET_ITEM_steals',
        'test_PyTuple_SET_ITEM_steals_replace',
        'test_PyTuple_SetIem_NULL_SetItem',
        'test_PyTuple_SetItem_NULL',
        'test_PyTuple_SetItem_fails_not_a_tuple',
        'test_PyTuple_SetItem_fails_out_of_range',
        'test_PyTuple_SetItem_replace_same',
        'test_PyTuple_SetItem_steals',
        'test_PyTuple_SetItem_steals_replace',
        'tuple_buildvalue_steals',
        'tuple_steals'
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


def test_c_ref_count_dict_no_steals_decref_after_set():
    assert cRefCount.dict_no_steals_decref_after_set() == 0


def test_c_ref_count_dict_buildvalue_no_steals():
    assert cRefCount.dict_buildvalue_no_steals() == 0


def test_PyTuple_SetItem_steals():
    assert cRefCount.test_PyTuple_SetItem_steals() == 0


def test_PyTuple_SET_ITEM_steals():
    assert cRefCount.test_PyTuple_SET_ITEM_steals() == 0


def test_PyTuple_SetItem_steals_replace():
    assert cRefCount.test_PyTuple_SetItem_steals_replace() == 0


def test_PyTuple_SET_ITEM_steals_replace():
    assert cRefCount.test_PyTuple_SET_ITEM_steals_replace() == 0


def test_PyTuple_SetItem_NULL():
    assert cRefCount.test_PyTuple_SetItem_NULL() == 0


def test_PyTuple_SET_ITEM_NULL():
    assert cRefCount.test_PyTuple_SET_ITEM_NULL() == 0


def test_PyTuple_SetIem_NULL_SetItem():
    assert cRefCount.test_PyTuple_SetIem_NULL_SetItem() == 0


def test_PyTuple_SET_ITEM_NULL_SET_ITEM():
    assert cRefCount.test_PyTuple_SET_ITEM_NULL_SET_ITEM() == 0


def test_PyTuple_SetItem_fails_not_a_tuple():
    with pytest.raises(SystemError) as err:
        cRefCount.test_PyTuple_SetItem_fails_not_a_tuple()
    assert err.value.args[0].endswith('bad argument to internal function')


def test_PyTuple_SetItem_fails_out_of_range():
    with pytest.raises(IndexError) as err:
        cRefCount.test_PyTuple_SetItem_fails_out_of_range()
    assert err.value.args[0] == 'tuple assignment index out of range'


def test_PyTuple_SetItem_replace_same():
    assert cRefCount.test_PyTuple_SetItem_replace_same() == 0


def test_PyTuple_SET_ITEM_replace_same():
    assert cRefCount.test_PyTuple_SET_ITEM_replace_same() == 0


def test_PyTuple_Py_PyTuple_Pack():
    assert cRefCount.test_PyTuple_Py_PyTuple_Pack() == 0


def test_PyTuple_Py_BuildValue():
    assert cRefCount.test_PyTuple_Py_BuildValue() == 0


def test_PyList_SetItem_steals():
    assert cRefCount.test_PyList_SetItem_steals() == 0


def test_PyList_SET_ITEM_steals():
    assert cRefCount.test_PyList_SET_ITEM_steals() == 0


def test_PyList_SetItem_steals_replace():
    assert cRefCount.test_PyList_SetItem_steals_replace() == 0


def test_PyList_SET_ITEM_steals_replace():
    assert cRefCount.test_PyList_SET_ITEM_steals_replace() == 0


def test_PyList_SetItem_NULL():
    assert cRefCount.test_PyList_SetItem_NULL() == 0


def test_PyList_SET_ITEM_NULL():
    assert cRefCount.test_PyList_SET_ITEM_NULL() == 0


def test_PyList_SetIem_NULL_SetItem():
    assert cRefCount.test_PyList_SetIem_NULL_SetItem() == 0


def test_PyList_SET_ITEM_NULL_SET_ITEM():
    assert cRefCount.test_PyList_SET_ITEM_NULL_SET_ITEM() == 0


def test_PyList_SetItem_fails_not_a_list():
    with pytest.raises(SystemError) as err:
        cRefCount.test_PyList_SetItem_fails_not_a_list()
    assert err.value.args[0].endswith('bad argument to internal function')


def test_PyList_SetItem_fails_out_of_range():
    with pytest.raises(IndexError) as err:
        cRefCount.test_PyList_SetItem_fails_out_of_range()
    assert err.value.args[0] == 'list assignment index out of range'


def test_PyList_Append():
    assert cRefCount.test_PyList_Append() == 0


def test_PyList_Append_fails_not_a_list():
    with pytest.raises(SystemError) as err:
        cRefCount.test_PyList_Append_fails_not_a_list()
    assert err.value.args[0].endswith(' bad argument to internal function')


def test_PyList_Append_fails_NULL():
    with pytest.raises(SystemError) as err:
        cRefCount.test_PyList_Append_fails_NULL()
    assert err.value.args[0].endswith(' bad argument to internal function')


def test_PyList_Insert():
    assert cRefCount.test_PyList_Insert() == 0


def test_PyList_Insert_Is_Truncated():
    assert cRefCount.test_PyList_Insert_Is_Truncated() == 0


def test_PyList_Insert_Negative_Index():
    assert cRefCount.test_PyList_Insert_Negative_Index() == 0


def test_PyList_Insert_fails_not_a_list():
    with pytest.raises(SystemError) as err:
        cRefCount.test_PyList_Insert_fails_not_a_list()
    assert err.value.args[0].endswith(' bad argument to internal function')


def test_PyList_Insert_fails_NULL():
    with pytest.raises(SystemError) as err:
        cRefCount.test_PyList_Insert_fails_NULL()
    assert err.value.args[0].endswith(' bad argument to internal function')


def test_PyList_SetItem_replace_same():
    assert cRefCount.test_PyList_SetItem_replace_same() == 0


def test_PyList_SET_ITEM_replace_same():
    assert cRefCount.test_PyList_SET_ITEM_replace_same() == 0


def test_PyList_Py_BuildValue():
    assert cRefCount.test_PyList_Py_BuildValue() == 0


def test_PyDict_SetItem_increments():
    assert cRefCount.test_PyDict_SetItem_increments() == 0


def test_PyDict_SetItem_fails_not_a_dict():
    with pytest.raises(SystemError) as err:
        cRefCount.test_PyDict_SetItem_fails_not_a_dict()
    assert err.value.args[0].endswith('bad argument to internal function')


def test_PyDict_SetItem_fails_not_hashable():
    with pytest.raises(TypeError) as err:
        cRefCount.test_PyDict_SetItem_fails_not_hashable()
    assert err.value.args[0] == "unhashable type: 'list'"


def test_PyDict_SetDefault_default_unused():
    assert cRefCount.test_PyDict_SetDefault_default_unused() == 0


def test_PyDict_SetDefault_default_used():
    assert cRefCount.test_PyDict_SetDefault_default_used() == 0


@pytest.mark.skipif(not (sys.version_info.minor >= 13), reason='Python 3.13+')
def test_PyDict_SetDefaultRef_default_unused():
    assert cRefCount.test_PyDict_SetDefaultRef_default_unused() == 0


@pytest.mark.skipif(not (sys.version_info.minor >= 13), reason='Python 3.13+')
def test_PyDict_SetDefaultRef_default_used():
    assert cRefCount.test_PyDict_SetDefaultRef_default_used() == 0


def test_test_PyDict_GetItem():
    assert cRefCount.test_PyDict_GetItem() == 0


@pytest.mark.skipif(not (sys.version_info.minor >= 13), reason='Python 3.13+')
def test_PyDict_Pop_key_present():
    assert cRefCount.test_PyDict_Pop_key_present() == 0


@pytest.mark.skipif(not (sys.version_info.minor >= 13), reason='Python 3.13+')
def test_PyDict_Pop_key_absent():
    assert cRefCount.test_PyDict_Pop_key_absent() == 0


def test_PySet_Add():
    assert cRefCount.test_PySet_Add() == 0


def test_PySet_Discard():
    assert cRefCount.test_PySet_Discard() == 0


def test_PySet_Pop():
    assert cRefCount.test_PySet_Pop() == 0
