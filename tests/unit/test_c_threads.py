import datetime
import sys
import threading
import time
import zoneinfo

import faulthandler

faulthandler.enable()

import pytest

from cPyExtPatt.Threads import sublist




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
                      'max',
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
                      'max',
                      'pop',
                      'remove',
                      'reverse',
                      'sort',
                      'state']


def test_sublist_sublist_ctor_range():
    obj = sublist.SubList(range(128))
    assert obj.appends == 0
    assert obj == list(range(128))


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


@pytest.mark.parametrize(
    'values, expected',
    (
            ((0,), 0),
            ((3, 2,), 3),
            ((3, 2, 1,), 3),
    )
)
def test_sublist_sublist_max(values, expected):
    obj = sublist.SubList()
    assert obj.appends == 0
    for value in values:
        obj.append(value)
    assert obj.appends == len(values)
    assert obj.max() == expected


def sublist_max(obj, count):
    print(f'sublist_max(): Thread name {threading.current_thread().name}', flush=True)
    for _i in range(count):
        print(f'sublist_max(): Thread name {threading.current_thread().name} Result: {obj.max()}', flush=True)
        time.sleep(0.25)
    print(f'sublist_max(): Thread name {threading.current_thread().name} DONE', flush=True)


def sublist_append(obj, count):
    print(f'sublist_append(): Thread name {threading.current_thread().name}', flush=True)
    for _i in range(count):
        print(f'sublist_append(): Thread name {threading.current_thread().name} appends was: {obj.appends}', flush=True)
        obj.append(len(obj))
        time.sleep(0.25)
    print(f'sublist_append(): Thread name {threading.current_thread().name} DONE', flush=True)


def test_threaded_max():
    print()
    print('test_threaded_max() START', flush=True)
    obj = sublist.SubList(range(128))
    threads = []
    for i in range(4):
        threads.append(
            threading.Thread(name=f'sublist_max[{i:2d}]', target=sublist_max, args=(obj, 2))
        )
        threads.append(
            threading.Thread(name=f'sublist_append[{i:2d}]', target=sublist_append, args=(obj, 2))
        )
    for thread in threads:
        thread.start()
    print('Waiting for worker threads', flush=True)
    main_thread = threading.current_thread()
    for t in threading.enumerate():
        if t is not main_thread:
            t.join()
    print('Worker threads DONE', flush=True)
