import pytest

from cPyExtPatt import cObject


def test_module_dir():
    assert dir(cObject) == ['Null', 'ObjectWithAttributes', 'Str', '__doc__', '__file__', '__loader__', '__name__',
                            '__package__', '__spec__', 'bug', 'error', 'foo', 'new', 'roj']


def test_null_dir():
    null = cObject.Null()
    print()
    print(dir(null))
    assert dir(null) == ['__class__', '__delattr__', '__dir__', '__doc__', '__eq__', '__format__', '__ge__',
                         '__getattribute__', '__getstate__', '__gt__', '__hash__', '__init__', '__init_subclass__',
                         '__le__', '__lt__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__',
                         '__setattr__', '__sizeof__', '__str__', '__subclasshook__']
