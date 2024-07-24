import datetime
import sys
import zoneinfo

import pytest

from cPyExtPatt.SubClass import sublist




def test_sublist_dir():
    result = dir(sublist)
    assert result == ['SubList',
                      '__doc__',
                      '__file__',
                      '__loader__',
                      '__name__',
                      '__package__',
                      '__spec__']



@pytest.mark.skipif(not (sys.version_info.minor <= 10), reason='Python 3.9, 3.10')
def test_sublist_sublist_dir_pre_311():
    sublist_object = sublist.SubList()
    result = dir(sublist_object)
    assert result == ['__add__',
                      '__class__',
                      '__class_getitem__',
                      '__contains__',
                      '__delattr__',
                      '__delitem__',
                      '__dir__',
                      '__doc__',
                      '__eq__',
                      '__format__',
                      '__ge__',
                      '__getattribute__',
                      '__getitem__',
                      '__gt__',
                      '__hash__',
                      '__iadd__',
                      '__imul__',
                      '__init__',
                      '__init_subclass__',
                      '__iter__',
                      '__le__',
                      '__len__',
                      '__lt__',
                      '__mul__',
                      '__ne__',
                      '__new__',
                      '__reduce__',
                      '__reduce_ex__',
                      '__repr__',
                      '__reversed__',
                      '__rmul__',
                      '__setattr__',
                      '__setitem__',
                      '__sizeof__',
                      '__str__',
                      '__subclasshook__',
                      'append',
                      'appends',
                      'clear',
                      'copy',
                      'count',
                      'extend',
                      'increment',
                      'index',
                      'insert',
                      'pop',
                      'remove',
                      'reverse',
                      'sort',
                      'state']


@pytest.mark.skipif(not (sys.version_info.minor > 10), reason='Python 3.11+')
def test_sublist_sublist_dir_post_310():
    sublist_object = sublist.SubList()
    result = dir(sublist_object)
    assert result == ['__add__',
                      '__class__',
                      '__class_getitem__',
                      '__contains__',
                      '__delattr__',
                      '__delitem__',
                      '__dir__',
                      '__doc__',
                      '__eq__',
                      '__format__',
                      '__ge__',
                      '__getattribute__',
                      '__getitem__',
                      '__getstate__',
                      '__gt__',
                      '__hash__',
                      '__iadd__',
                      '__imul__',
                      '__init__',
                      '__init_subclass__',
                      '__iter__',
                      '__le__',
                      '__len__',
                      '__lt__',
                      '__mul__',
                      '__ne__',
                      '__new__',
                      '__reduce__',
                      '__reduce_ex__',
                      '__repr__',
                      '__reversed__',
                      '__rmul__',
                      '__setattr__',
                      '__setitem__',
                      '__sizeof__',
                      '__str__',
                      '__subclasshook__',
                      'append',
                      'appends',
                      'clear',
                      'copy',
                      'count',
                      'extend',
                      'increment',
                      'index',
                      'insert',
                      'pop',
                      'remove',
                      'reverse',
                      'sort',
                      'state']


def test_sublist_sublist_append():
    obj = sublist.SubList()
    assert obj.appends == 0
    obj.append(42)
    assert obj.appends == 1
    assert obj == [42, ]


def test_sublist_sublist_state():
    obj = sublist.SubList()
    assert obj.state == 0
    obj.increment()
    assert obj.state == 1
