import pytest

from cPyExtPatt import cObject


def test_module_dir():
    assert dir(cObject) == ['Null', 'ObjectWithAttributes', 'Str', '__doc__', '__file__', '__loader__', '__name__',
                            '__package__', '__spec__', 'error',]


def test_null_dir():
    null = cObject.Null()
    assert dir(null) == ['__class__', '__delattr__', '__dir__', '__doc__', '__eq__', '__format__', '__ge__',
                         '__getattribute__', '__getstate__', '__gt__', '__hash__', '__init__', '__init_subclass__',
                         '__le__', '__lt__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__',
                         '__setattr__', '__sizeof__', '__str__', '__subclasshook__']


def test_ObjectWithAttributes_dir():
    obj = cObject.ObjectWithAttributes()
    # print()
    # print(dir(obj))
    assert dir(obj) == ['__class__', '__delattr__', '__dir__', '__doc__', '__eq__', '__format__', '__ge__',
                        '__getattribute__', '__getstate__', '__gt__', '__hash__', '__init__', '__init_subclass__',
                        '__le__', '__lt__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__',
                        '__setattr__', '__sizeof__', '__str__', '__subclasshook__', 'demo',]


def test_ObjectWithAttributes_set_and_get():
    obj = cObject.ObjectWithAttributes()
    obj.some_attr = 'Some attribute'
    # print()
    # print(obj)
    # print(obj.some_attr)
    assert hasattr(obj, 'some_attr')
    assert obj.some_attr == 'Some attribute'


def test_ObjectWithAttributes_set_and_del():
    obj = cObject.ObjectWithAttributes()
    obj.some_attr = 'Some attribute'
    # print()
    # print(obj)
    # print(obj.some_attr)
    assert hasattr(obj, 'some_attr')
    delattr(obj, 'some_attr')
    assert not hasattr(obj, 'some_attr')
    with pytest.raises(AttributeError) as err:
        obj.some_attr
    assert err.value.args[0] == "'cObject.ObjectWithAttributes' object has no attribute 'some_attr'"


def test_str_dir():
    s = cObject.Str()
    assert dir(s) == ['__add__', '__class__', '__contains__', '__delattr__', '__dir__', '__doc__', '__eq__',
                      '__format__', '__ge__', '__getattribute__', '__getitem__', '__getnewargs__', '__getstate__',
                      '__gt__', '__hash__', '__init__', '__init_subclass__', '__iter__', '__le__', '__len__', '__lt__',
                      '__mod__', '__mul__', '__ne__', '__new__', '__reduce__', '__reduce_ex__', '__repr__', '__rmod__',
                      '__rmul__', '__setattr__', '__sizeof__', '__str__', '__subclasshook__', 'capitalize', 'casefold',
                      'center', 'count', 'encode', 'endswith', 'expandtabs', 'find', 'format', 'format_map', 'index',
                      'isalnum', 'isalpha', 'isascii', 'isdecimal', 'isdigit', 'isidentifier', 'islower', 'isnumeric',
                      'isprintable', 'isspace', 'istitle', 'isupper', 'join', 'ljust', 'lower', 'lstrip', 'maketrans',
                      'partition', 'removeprefix', 'removesuffix', 'replace', 'rfind', 'rindex', 'rjust', 'rpartition',
                      'rsplit', 'rstrip', 'split', 'splitlines', 'startswith', 'strip', 'swapcase', 'title',
                      'translate', 'upper', 'zfill']