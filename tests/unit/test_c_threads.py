import datetime
import sys
import threading
import time
import zoneinfo

import faulthandler

faulthandler.enable()

import pytest

from cPyExtPatt.Threads import cppsublist
from cPyExtPatt.Threads import csublist


def test_cppsublist_dir():
    result = dir(cppsublist)
    assert result == ['__doc__',
                      '__file__',
                      '__loader__',
                      '__name__',
                      '__package__',
                      '__spec__',
                      'cppSubList',
                      ]


def test_csublist_dir():
    result = dir(csublist)
    assert result == ['__doc__',
                      '__file__',
                      '__loader__',
                      '__name__',
                      '__package__',
                      '__spec__',
                      'cSubList',
                      ]


@pytest.mark.skipif(not (sys.version_info.minor <= 10), reason='Python 3.9, 3.10')
def test_cppsublist_cppsublist_dir_pre_311():
    sublist_object = cppsublist.cppSubList()
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
                      'clear',
                      'copy',
                      'count',
                      'extend',
                      'index',
                      'insert',
                      'max',
                      'pop',
                      'remove',
                      'reverse',
                      'sort']


@pytest.mark.skipif(not (sys.version_info.minor > 10), reason='Python 3.11+')
def test_cppsublist_cppsublist_dir_post_310():
    sublist_object = cppsublist.cppSubList()
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
                      'clear',
                      'copy',
                      'count',
                      'extend',
                      'index',
                      'insert',
                      'max',
                      'pop',
                      'remove',
                      'reverse',
                      'sort']


@pytest.mark.skipif(not (sys.version_info.minor <= 10), reason='Python 3.9, 3.10')
def test_cppsublist_csublist_dir_pre_311():
    sublist_object = csublist.cSubList()
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
                      'clear',
                      'copy',
                      'count',
                      'extend',
                      'index',
                      'insert',
                      'max',
                      'pop',
                      'remove',
                      'reverse',
                      'sort']


@pytest.mark.skipif(not (sys.version_info.minor > 10), reason='Python 3.11+')
def test_cppsublist_csublist_dir_post_310():
    sublist_object = csublist.cSubList()
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
                      'clear',
                      'copy',
                      'count',
                      'extend',
                      'index',
                      'insert',
                      'max',
                      'pop',
                      'remove',
                      'reverse',
                      'sort']


def test_cppsublist_cppsublist_append():
    obj = cppsublist.cppSubList()
    obj.append(42)
    assert obj == [42, ]


@pytest.mark.parametrize(
    'values, expected',
    (
            ((0,), 0),
            ((3, 2,), 3),
            ((3, 2, 1,), 3),
    )
)
def test_cppsublist_cppsublist_max(values, expected):
    obj = cppsublist.cppSubList()
    for value in values:
        obj.append(value)
    assert obj.max() == expected


def cppsublist_max(obj, count):
    print(f'sublist_max(): Thread name {threading.current_thread().name}', flush=True)
    for _i in range(count):
        print(f'sublist_max(): Thread name {threading.current_thread().name} Result: {obj.max()}', flush=True)
        time.sleep(0.25)
    print(f'sublist_max(): Thread name {threading.current_thread().name} DONE', flush=True)


def cppsublist_append(obj, count):
    print(f'sublist_append(): Thread name {threading.current_thread().name}', flush=True)
    for _i in range(count):
        print(f'sublist_append(): Thread name {threading.current_thread().name}', flush=True)
        obj.append(len(obj))
        time.sleep(0.25)
    print(f'sublist_append(): Thread name {threading.current_thread().name} DONE', flush=True)


def test_threaded_cpp():
    print()
    print('test_threaded_max() START', flush=True)
    obj = cppsublist.cppSubList(range(128))
    threads = []
    for i in range(4):
        threads.append(
            threading.Thread(name=f'sublist_max[{i:2d}]', target=cppsublist_max, args=(obj, 2))
        )
        threads.append(
            threading.Thread(name=f'sublist_append[{i:2d}]', target=cppsublist_append, args=(obj, 2))
        )
    for thread in threads:
        thread.start()
    print('Waiting for worker threads', flush=True)
    main_thread = threading.current_thread()
    for t in threading.enumerate():
        if t is not main_thread:
            t.join()
    print('Worker threads DONE', flush=True)


def test_csublist_csublist_ctor_range():
    obj = csublist.cSubList(range(128))
    assert obj == list(range(128))


def test_csublist_csublist_append():
    obj = csublist.cSubList()
    obj.append(42)
    assert obj == [42, ]


@pytest.mark.parametrize(
    'values, expected',
    (
            ((0,), 0),
            ((3, 2,), 3),
            ((3, 2, 1,), 3),
    )
)
def test_csublist_csublist_max(values, expected):
    obj = csublist.cSubList()
    for value in values:
        obj.append(value)
    assert obj.max() == expected


def csublist_max(obj, count):
    print(
        f'sublist_max(): Thread name {threading.current_thread().name}',
        flush=True
    )
    for _i in range(count):
        print(
            f'sublist_max(): Thread name {threading.current_thread().name}'
            f' Result: {obj.max()}',
            flush=True
        )
        time.sleep(0.25)
    print(
        f'sublist_max(): Thread name {threading.current_thread().name} DONE',
        flush=True
    )


def csublist_append(obj, count):
    print(
        f'sublist_append(): Thread name {threading.current_thread().name}',
        flush=True
    )
    for _i in range(count):
        print(
            f'sublist_append(): Thread name {threading.current_thread().name}',
            flush=True
        )
        obj.append(len(obj))
        time.sleep(0.25)
    print(
        f'sublist_append(): Thread name {threading.current_thread().name} DONE',
        flush=True
    )


def test_threaded_c():
    print()
    print('test_threaded_c() START', flush=True)
    obj = csublist.cSubList(range(128))
    threads = []
    for i in range(4):
        threads.append(
            threading.Thread(
                name=f'sublist_max[{i:2d}]',
                target=csublist_max,
                args=(obj, 2),
            )
        )
        threads.append(
            threading.Thread(
                name=f'sublist_append[{i:2d}]',
                target=csublist_append,
                args=(obj, 2),
            )
        )
    for thread in threads:
        thread.start()
    print('Waiting for worker threads', flush=True)
    main_thread = threading.current_thread()
    for t in threading.enumerate():
        if t is not main_thread:
            t.join()
    print('Worker threads DONE', flush=True)
