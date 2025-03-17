import sys

import pytest

from cPyExtPatt import cSeqObject


def test_module_dir():
    assert dir(cSeqObject) == ['SequenceLongObject', '__doc__', '__file__', '__loader__', '__name__',
                               '__package__', '__spec__', ]


def test_SequenceLongObject_dir():
    obj = cSeqObject.SequenceLongObject([7, 4, 1, ])
    assert dir(obj) == [
        '__add__',
        '__class__',
        '__delattr__',
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
        '__init__',
        '__init_subclass__',
        '__le__',
        '__len__',
        '__lt__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        '__repr__',
        '__setattr__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
    ]


def test_SequenceLongObject_len():
    obj = cSeqObject.SequenceLongObject([7, 4, 1, ])
    assert len(obj) == 3


def test_SequenceLongObject_concat():
    obj_a = cSeqObject.SequenceLongObject([7, 4, 1, ])
    # print(obj_a[4])
    # assert list(obj_a) == [7, 4, 1, ]
    # assert [v for v in obj_a] == [7, 4, 1, ]
    obj_b = cSeqObject.SequenceLongObject([70, 40, 100, ])
    assert id(obj_a) != id(obj_b)
    obj = obj_a + obj_b
    assert id(obj) != id(obj_a)
    assert id(obj) != id(obj_b)
    assert len(obj) == 6
    assert list(obj) == [7, 4, 1, ] + [70, 40, 100, ]

# @pytest.mark.skipif(not (sys.version_info.minor < 7), reason='Python < 3.7')
# def test_str_dir_pre_37():
#     s = cObject.Str()
#     assert dir(s) == ['__add__', '__class__', '__contains__', '__delattr__', '__dir__', '__doc__', '__eq__',
#                       '__format__', '__ge__', '__getattribute__', '__getitem__', '__getnewargs__',
#                       '__gt__', '__hash__', '__init__', '__init_subclass__', '__iter__', '__le__', '__len__', '__lt__',
#                       '__mod__', '__mul__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__rmod__',
#                       '__rmul__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__', 'capitalize', 'casefold',
#                       'center', 'count', 'encode', 'endswith', 'expandtabs', 'find', 'format', 'format_map', 'index',
#                       'isalnum', 'isalpha', 'isdecimal', 'isdigit', 'isidentifier', 'islower', 'isnumeric',
#                       'isprintable', 'isspace', 'istitle', 'isupper', 'join', 'ljust', 'lower', 'lstrip', 'maketrans',
#                       'partition', 'replace', 'rfind', 'rindex', 'rjust', 'rpartition',
#                       'rsplit', 'rstrip', 'split', 'splitlines', 'startswith', 'strip', 'swapcase', 'title',
#                       'translate', 'upper', 'zfill']
