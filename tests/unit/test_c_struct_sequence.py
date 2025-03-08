import pprint
import sys

import pytest

from cPyExtPatt import cStructSequence


@pytest.mark.skipif(not (sys.version_info.minor < 11), reason='Python < 3.11')
def test_c_struct_sequence_dir_pre_3_11():
    result = dir(cStructSequence)
    print()
    print(result)
    assert result == [
        'BasicNT_create',
        'ExcessNT_create',
        'NTRegisteredType',
        'NTUnRegistered_create',
        '__doc__',
        '__file__',
        '__loader__',
        '__name__',
        '__package__',
        '__spec__',
        'cTransaction_get',
    ]


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python 3.11+')
def test_c_struct_sequence_dir_3_11_onwards():
    result = dir(cStructSequence)
    print()
    print(result)
    assert result == [
        'BasicNT_create',
        'ExcessNT_create',
        'NTRegisteredType',
        'NTUnRegistered_create',
        'NTWithUnnamedField_create',
        '__doc__',
        '__file__',
        '__loader__',
        '__name__',
        '__package__',
        '__spec__',
        'cTransaction_get',
    ]


def test_basic_nt_create():
    basic_nt = cStructSequence.BasicNT_create('foo', 'bar')
    assert str(type(basic_nt)) == "<class 'cStructSequence.BasicNT'>"


def test_basic_nt_create_attributes():
    basic_nt = cStructSequence.BasicNT_create('foo', 'bar')
    assert basic_nt.field_one == "foo"
    assert basic_nt.field_two == "bar"
    assert basic_nt.index("foo") == 0
    assert basic_nt.index("bar") == 1
    assert basic_nt.n_fields == 2
    assert basic_nt.n_sequence_fields == 2
    assert basic_nt.n_unnamed_fields == 0


def test_nt_registered_type():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    assert str(type(nt)) == "<class 'cStructSequence.NTRegistered'>"


def test_nt_registered_str():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    assert str(nt) == "cStructSequence.NTRegistered(field_one='foo', field_two='bar')"


def test_nt_registered_mro():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    assert str(type(nt).__mro__) == "(<class 'cStructSequence.NTRegistered'>, <class 'tuple'>, <class 'object'>)"


@pytest.mark.skipif(not (sys.version_info.minor < 10), reason='Python < 3.10')
def test_nt_registered_dir_pre_3_10():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    assert dir(nt) == [
        '__add__',
        '__class__',
        '__class_getitem__',
        '__contains__',
        '__delattr__',
        '__dir__',
        '__doc__',
        '__eq__',
        '__format__',
        '__ge__',
        '__getattribute__',
        '__getitem__',
        '__getnewargs__',
        # '__getstate__',
        '__gt__',
        '__hash__',
        '__init__',
        '__init_subclass__',
        '__iter__',
        '__le__',
        '__len__',
        '__lt__',
        # '__match_args__',
        '__module__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        # '__replace__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
        'count',
        'field_one',
        'field_two',
        'index',
        'n_fields',
        'n_sequence_fields',
        'n_unnamed_fields',
    ]


@pytest.mark.skipif(not (sys.version_info.minor == 10), reason='Python 3.10')
def test_nt_registered_dir_3_10():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    assert dir(nt) == [
        '__add__',
        '__class__',
        '__class_getitem__',
        '__contains__',
        '__delattr__',
        '__dir__',
        '__doc__',
        '__eq__',
        '__format__',
        '__ge__',
        '__getattribute__',
        '__getitem__',
        '__getnewargs__',
        '__gt__',
        '__hash__',
        '__init__',
        '__init_subclass__',
        '__iter__',
        '__le__',
        '__len__',
        '__lt__',
        '__match_args__',
        '__module__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
        'count',
        'field_one',
        'field_two',
        'index',
        'n_fields',
        'n_sequence_fields',
        'n_unnamed_fields',
    ]


@pytest.mark.skipif(not (sys.version_info.minor in (11, 12)), reason='Python 3.11, 3.12')
def test_nt_registered_dir_3_11_3_12():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    assert dir(nt) == [
        '__add__',
        '__class__',
        '__class_getitem__',
        '__contains__',
        '__delattr__',
        '__dir__',
        '__doc__',
        '__eq__',
        '__format__',
        '__ge__',
        '__getattribute__',
        '__getitem__',
        '__getnewargs__',
        '__getstate__',
        '__gt__',
        '__hash__',
        '__init__',
        '__init_subclass__',
        '__iter__',
        '__le__',
        '__len__',
        '__lt__',
        '__match_args__',
        '__module__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
        'count',
        'field_one',
        'field_two',
        'index',
        'n_fields',
        'n_sequence_fields',
        'n_unnamed_fields',
    ]


@pytest.mark.skipif(not (sys.version_info.minor >= 13), reason='Python >= 3.13')
def test_nt_registered_dir_3_13_onwards():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    assert dir(nt) == [
        '__add__',
        '__class__',
        '__class_getitem__',
        '__contains__',
        '__delattr__',
        '__dir__',
        '__doc__',
        '__eq__',
        '__format__',
        '__ge__',
        '__getattribute__',
        '__getitem__',
        '__getnewargs__',
        '__getstate__',
        '__gt__',
        '__hash__',
        '__init__',
        '__init_subclass__',
        '__iter__',
        '__le__',
        '__len__',
        '__lt__',
        '__match_args__',
        '__module__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        '__replace__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
        'count',
        'field_one',
        'field_two',
        'index',
        'n_fields',
        'n_sequence_fields',
        'n_unnamed_fields',
    ]


def test_nt_registered_field_access():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    assert nt.field_one == 'foo'
    assert nt.field_two == 'bar'


def test_nt_registered_index():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    assert nt[0] == 'foo'
    assert nt[1] == 'bar'
    assert nt[-1] == 'bar'
    assert nt[-2] == 'foo'


def test_nt_registered_index_out_of_range():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    with pytest.raises(IndexError) as err:
        value = nt[2]
    assert err.value.args[0] == 'tuple index out of range'
    with pytest.raises(IndexError) as err:
        value = nt[-3]
    assert err.value.args[0] == 'tuple index out of range'


def test_nt_no__make():
    with pytest.raises(AttributeError) as err:
        nt = cStructSequence.NTRegisteredType._make(('foo', 'bar'))
    assert err.value.args[0] == "type object 'cStructSequence.NTRegistered' has no attribute '_make'"


def test_nt_no__asdict():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    with pytest.raises(AttributeError) as err:
        nt._asdict()
    assert err.value.args[0] == "'cStructSequence.NTRegistered' object has no attribute '_asdict'"


def test_nt_no__replace():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    with pytest.raises(AttributeError) as err:
        nt._replace(field_one='baz')
    assert err.value.args[0] == "'cStructSequence.NTRegistered' object has no attribute '_replace'"


def test_nt_no__fields():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    with pytest.raises(AttributeError) as err:
        nt._fields
    assert err.value.args[0] == "'cStructSequence.NTRegistered' object has no attribute '_fields'"


def test_nt_no__field_defaults():
    nt = cStructSequence.NTRegisteredType(('foo', 'bar'))
    with pytest.raises(AttributeError) as err:
        nt._fields_defaults
    assert err.value.args[0] == "'cStructSequence.NTRegistered' object has no attribute '_fields_defaults'"


def test_nt_unregistered_type_not_available():
    with pytest.raises(AttributeError) as err:
        cStructSequence.NTUnRegistered('bar', 'foo')
    assert err.value.args[0] == "module 'cPyExtPatt.cStructSequence' has no attribute 'NTUnRegistered'"


def test_nt_unregistered_type():
    ntu = cStructSequence.NTUnRegistered_create('bar', 'foo')
    assert str(type(ntu)) == "<class 'cStructSequence.NTUnRegistered'>"


def test_nt_unregistered_mro():
    ntu = cStructSequence.NTUnRegistered_create('bar', 'foo')
    assert str(type(ntu).__mro__) == "(<class 'cStructSequence.NTUnRegistered'>, <class 'tuple'>, <class 'object'>)"


@pytest.mark.skipif(not (sys.version_info.minor < 10), reason='Python < 3.10')
def test_nt_unregistered_dir_pre_3_10():
    ntu = cStructSequence.NTUnRegistered_create('bar', 'foo')
    assert dir(ntu) == [
        '__add__',
        '__class__',
        '__class_getitem__',
        '__contains__',
        '__delattr__',
        '__dir__',
        '__doc__',
        '__eq__',
        '__format__',
        '__ge__',
        '__getattribute__',
        '__getitem__',
        '__getnewargs__',
        # '__getstate__',
        '__gt__',
        '__hash__',
        '__init__',
        '__init_subclass__',
        '__iter__',
        '__le__',
        '__len__',
        '__lt__',
        # '__match_args__',
        '__module__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        # '__replace__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
        'count',
        'field_one',
        'field_two',
        'index',
        'n_fields',
        'n_sequence_fields',
        'n_unnamed_fields',
    ]


@pytest.mark.skipif(not (sys.version_info.minor == 10), reason='Python 3.10')
def test_nt_unregistered_dir_3_10():
    ntu = cStructSequence.NTUnRegistered_create('bar', 'foo')
    assert dir(ntu) == [
        '__add__',
        '__class__',
        '__class_getitem__',
        '__contains__',
        '__delattr__',
        '__dir__',
        '__doc__',
        '__eq__',
        '__format__',
        '__ge__',
        '__getattribute__',
        '__getitem__',
        '__getnewargs__',
        '__gt__',
        '__hash__',
        '__init__',
        '__init_subclass__',
        '__iter__',
        '__le__',
        '__len__',
        '__lt__',
        '__match_args__',
        '__module__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
        'count',
        'field_one',
        'field_two',
        'index',
        'n_fields',
        'n_sequence_fields',
        'n_unnamed_fields',
    ]


@pytest.mark.skipif(not (sys.version_info.minor in (11, 12)), reason='Python 3.11, 3.12')
def test_nt_unregistered_dir_3_11_3_12():
    ntu = cStructSequence.NTUnRegistered_create('bar', 'foo')
    assert dir(ntu) == [
        '__add__',
        '__class__',
        '__class_getitem__',
        '__contains__',
        '__delattr__',
        '__dir__',
        '__doc__',
        '__eq__',
        '__format__',
        '__ge__',
        '__getattribute__',
        '__getitem__',
        '__getnewargs__',
        '__getstate__',
        '__gt__',
        '__hash__',
        '__init__',
        '__init_subclass__',
        '__iter__',
        '__le__',
        '__len__',
        '__lt__',
        '__match_args__',
        '__module__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
        'count',
        'field_one',
        'field_two',
        'index',
        'n_fields',
        'n_sequence_fields',
        'n_unnamed_fields',
    ]


@pytest.mark.skipif(not (sys.version_info.minor >= 13), reason='Python >= 3.13')
def test_nt_unregistered_dir_3_13_onwards():
    ntu = cStructSequence.NTUnRegistered_create('bar', 'foo')
    assert dir(ntu) == [
        '__add__',
        '__class__',
        '__class_getitem__',
        '__contains__',
        '__delattr__',
        '__dir__',
        '__doc__',
        '__eq__',
        '__format__',
        '__ge__',
        '__getattribute__',
        '__getitem__',
        '__getnewargs__',
        '__getstate__',
        '__gt__',
        '__hash__',
        '__init__',
        '__init_subclass__',
        '__iter__',
        '__le__',
        '__len__',
        '__lt__',
        '__match_args__',
        '__module__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        '__replace__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
        'count',
        'field_one',
        'field_two',
        'index',
        'n_fields',
        'n_sequence_fields',
        'n_unnamed_fields',
    ]


def test_nt_unregistered_field_access():
    ntu = cStructSequence.NTUnRegistered_create('foo', 'bar')
    assert ntu.field_one == 'foo'
    assert ntu.field_two == 'bar'


def test_nt_unregistered_index():
    ntu = cStructSequence.NTUnRegistered_create('foo', 'bar')
    assert ntu[0] == 'foo'
    assert ntu[1] == 'bar'
    assert ntu[-1] == 'bar'
    assert ntu[-2] == 'foo'


def test_nt_unregistered_index_out_of_range():
    ntu = cStructSequence.NTUnRegistered_create('bar', 'foo')
    with pytest.raises(IndexError) as err:
        value = ntu[2]
    assert err.value.args[0] == 'tuple index out of range'
    with pytest.raises(IndexError) as err:
        value = ntu[-3]
    assert err.value.args[0] == 'tuple index out of range'


def test_nt_cTransaction_get_type():
    nt = cStructSequence.cTransaction_get(17145)
    assert str(type(nt)) == "<class 'cStructSequence.cTransaction'>"


def test_nt_cTransaction_get_fields():
    nt = cStructSequence.cTransaction_get(17145)
    assert nt.id == 17145
    assert nt.reference == "Some reference."
    assert nt.amount == 42.76


def test_excess_nt_create():
    nt = cStructSequence.ExcessNT_create('bar', 'foo', 'baz')
    assert str(type(nt)) == "<class 'cStructSequence.ExcessNT'>"


@pytest.mark.parametrize(
    'attr, value',
    (
            ('n_fields', 3,),
            ('n_sequence_fields', 2,),
            ('n_unnamed_fields', 0,),
    )
)
def test_excess_nt_getattr(attr, value):
    nt = cStructSequence.ExcessNT_create('bar', 'foo', 'baz')
    result = getattr(nt, attr)
    assert result == value


def test_excess_nt_field_three_avalible():
    nt = cStructSequence.ExcessNT_create('bar', 'foo', 'baz')
    assert nt.field_three == 'baz'


def test_excess_nt_field_three_index_missing():
    nt = cStructSequence.ExcessNT_create('bar', 'foo', 'baz')
    with pytest.raises(IndexError) as err:
        nt[2]
    assert err.value.args[0] == 'tuple index out of range'


@pytest.mark.skipif(not (11 <= sys.version_info.minor <= 12), reason='Python 3.11, 3.12')
def test_nt_with_unnamed_field_create_dir_3_11_and_before():
    ntuf = cStructSequence.NTWithUnnamedField_create('foo', 'bar', 'baz')
    print()
    pprint.pprint(dir(ntuf))
    assert dir(ntuf) == [
        '__add__',
        '__class__',
        '__class_getitem__',
        '__contains__',
        '__delattr__',
        '__dir__',
        '__doc__',
        '__eq__',
        '__format__',
        '__ge__',
        '__getattribute__',
        '__getitem__',
        '__getnewargs__',
        '__getstate__',
        '__gt__',
        '__hash__',
        '__init__',
        '__init_subclass__',
        '__iter__',
        '__le__',
        '__len__',
        '__lt__',
        '__match_args__',
        '__module__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
        'count',
        'field_one',
        'index',
        'n_fields',
        'n_sequence_fields',
        'n_unnamed_fields',
    ]


@pytest.mark.skipif(not (sys.version_info.minor >= 13), reason='Python 3.13+')
def test_nt_with_unnamed_field_create_dir_3_12_onwards():
    ntuf = cStructSequence.NTWithUnnamedField_create('foo', 'bar', 'baz')
    print()
    pprint.pprint(dir(ntuf))
    assert dir(ntuf) == [
        '__add__',
        '__class__',
        '__class_getitem__',
        '__contains__',
        '__delattr__',
        '__dir__',
        '__doc__',
        '__eq__',
        '__format__',
        '__ge__',
        '__getattribute__',
        '__getitem__',
        '__getnewargs__',
        '__getstate__',
        '__gt__',
        '__hash__',
        '__init__',
        '__init_subclass__',
        '__iter__',
        '__le__',
        '__len__',
        '__lt__',
        '__match_args__',
        '__module__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        '__replace__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
        'count',
        'field_one',
        'index',
        'n_fields',
        'n_sequence_fields',
        'n_unnamed_fields'
    ]


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python 3.11+')
def test_nt_with_unnamed_field_create_len():
    ntuf = cStructSequence.NTWithUnnamedField_create('foo', 'bar', 'baz')
    assert len(ntuf) == 1


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python 3.11+')
def test_nt_with_unnamed_field_create_n_fields():
    ntuf = cStructSequence.NTWithUnnamedField_create('foo', 'bar', 'baz')
    assert ntuf.n_fields == 2


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python 3.11+')
def test_nt_with_unnamed_field_create_n_sequence_fields():
    ntuf = cStructSequence.NTWithUnnamedField_create('foo', 'bar', 'baz')
    assert ntuf.n_sequence_fields == 1


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python 3.11+')
def test_nt_with_unnamed_field_create_n_unnamed_fields():
    ntuf = cStructSequence.NTWithUnnamedField_create('foo', 'bar', 'baz')
    assert ntuf.n_unnamed_fields == 1


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python 3.11+')
def test_nt_with_unnamed_field_create_index_tuple():
    ntuf = cStructSequence.NTWithUnnamedField_create('foo', 'bar', 'baz')
    assert tuple(ntuf) == ('foo',)


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python 3.11+')
def test_nt_with_unnamed_field_create_index_fields():
    ntuf = cStructSequence.NTWithUnnamedField_create('foo', 'bar', 'baz')
    assert ntuf[0] == 'foo'


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python 3.11+')
def test_nt_with_unnamed_field_create_index_fields_raises():
    ntuf = cStructSequence.NTWithUnnamedField_create('foo', 'bar', 'baz')
    with pytest.raises(IndexError) as err:
        assert ntuf[1] == 'bar'
    assert err.value.args[0] == 'tuple index out of range'


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python 3.11+')
def test_nt_with_unnamed_field_create_repr():
    ntuf = cStructSequence.NTWithUnnamedField_create('foo', 'bar', 'baz')
    assert repr(ntuf) == "cStructSequence.NTWithUnnamedField(field_one='foo')"


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python 3.11+')
def test_nt_with_unnamed_field_create_str():
    ntuf = cStructSequence.NTWithUnnamedField_create('foo', 'bar', 'baz')
    assert str(ntuf) == "cStructSequence.NTWithUnnamedField(field_one='foo')"
