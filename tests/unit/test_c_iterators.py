import sys

import pytest

from cPyExtPatt.Iterators import cIterator


def test_c_iterator_dir():
    result = dir(cIterator)
    assert result == ['SequenceOfLong',
                      'SequenceOfLongIterator',
                      '__doc__',
                      '__file__',
                      '__loader__',
                      '__name__',
                      '__package__',
                      '__spec__',
                      'iterate_and_print']


@pytest.mark.skipif(not (sys.version_info.minor < 11), reason='Python < 3.11')
def test_c_iterator_sequence_of_long_dir_pre_311():
    result = dir(cIterator.SequenceOfLong)
    assert result == ['__class__',
                      '__delattr__',
                      '__dir__',
                      '__doc__',
                      '__eq__',
                      '__format__',
                      '__ge__',
                      '__getattribute__',
                      '__getitem__',
                      '__gt__',
                      '__hash__',
                      '__init__',
                      '__init_subclass__',
                      '__iter__',
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
                      'size']


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python >= 3.11')
def test_c_iterator_sequence_of_long_dir_311_plus():
    result = dir(cIterator.SequenceOfLong)
    assert result == ['__class__',
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
                      '__iter__',
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
                      'size']


@pytest.mark.skipif(not (sys.version_info.minor < 11), reason='Python < 3.11')
def test_c_iterator_sequence_of_long_iterator_dir_pre_311():
    result = dir(cIterator.SequenceOfLongIterator)
    assert result == ['__class__',
                      '__delattr__',
                      '__dir__',
                      '__doc__',
                      '__eq__',
                      '__format__',
                      '__ge__',
                      '__getattribute__',
                      '__gt__',
                      '__hash__',
                      '__init__',
                      '__init_subclass__',
                      '__iter__',
                      '__le__',
                      '__lt__',
                      '__ne__',
                      '__new__',
                      '__next__',
                      '__reduce__',
                      '__reduce_ex__',
                      '__repr__',
                      '__setattr__',
                      '__sizeof__',
                      '__str__',
                      '__subclasshook__']


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python >= 3.11')
def test_c_iterator_sequence_of_long_iterator_dir_311_plus():
    result = dir(cIterator.SequenceOfLongIterator)
    assert result == ['__class__',
                      '__delattr__',
                      '__dir__',
                      '__doc__',
                      '__eq__',
                      '__format__',
                      '__ge__',
                      '__getattribute__',
                      '__getstate__',
                      '__gt__',
                      '__hash__',
                      '__init__',
                      '__init_subclass__',
                      '__iter__',
                      '__le__',
                      '__lt__',
                      '__ne__',
                      '__new__',
                      '__next__',
                      '__reduce__',
                      '__reduce_ex__',
                      '__repr__',
                      '__setattr__',
                      '__sizeof__',
                      '__str__',
                      '__subclasshook__']


def test_c_iterator_ctor():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    assert sequence
    assert type(sequence) is cIterator.SequenceOfLong
    assert sequence.size() == 3


def test_c_iterator_ctor_iter():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = iter(sequence)
    assert iterator
    assert type(iterator) is cIterator.SequenceOfLongIterator


def test_c_iterator_ctor_iter_for():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    result = [v for v in sequence]
    assert result == [1, 7, 4]


def test_c_iterator_ctor_iter_del_original():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = iter(sequence)
    del sequence
    result = [v for v in iterator]
    assert result == [1, 7, 4]


def test_c_iterator_ctor_iter_next():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = iter(sequence)
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4


def test_c_iterator_ctor_iter_next_raises():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = iter(sequence)
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4
    with pytest.raises(StopIteration):
        next(iterator)


def yield_from_an_iterator_times_two(iterator):
    for value in iterator:
        yield 2 * value


def test_c_iterator_yield_forward():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = iter(sequence)
    result = []
    for v in yield_from_an_iterator_times_two(iterator):
        result.append(v)
    assert result == [2, 14, 8]


def test_c_iterator_sorted():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    result = sorted(sequence)
    print()
    print(result)
    assert result == [1, 4, 7, ]
    original = [v for v in sequence]
    assert original == [1, 7, 4, ]


def test_c_iterator_reversed():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    result = reversed(sequence)
    print()
    print(result)
    assert list(result) == [4, 7, 1,]


def test_c_iterator_generator_expression_sum():
    """https://docs.python.org/3/glossary.html#term-generator-expression"""
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    result = sum(v * 4 for v in sequence)
    assert result == 4 * (1 + 7 + 4)


def test_modify_list_during_iteration_a():
    lst = list(range(8))
    print()
    result = []
    for i, value in enumerate(lst):
        result.append(value)
        print(f'i={i} value={value}')
        del lst[i]
    assert result == [0, 2, 4, 6]


def test_modify_list_during_iteration_b():
    lst = list(range(8))
    print()
    result = []
    for i, value in enumerate(lst):
        result.append(value)
        print(f'i={i} value={value}')
        lst.pop()
    assert result == [0, 1, 2, 3]


def test_modify_list_during_iteration_c():
    lst = list(range(8))
    print()
    result = []
    for i, value in enumerate(lst):
        result.append(value)
        print(f'i={i} value={value}')
        if i and i % 2 == 0:
            lst.append(8 * i)
    assert result == [0, 1, 2, 3, 4, 5, 6, 7, 16, 32, 48, 64, 80, 96]


@pytest.mark.parametrize(
    'arg, expected',
    (
            (
                    'abc',
                    """iterate_and_print:
[0]: a
[1]: b
[2]: c
iterate_and_print: DONE
"""
            ),
    )
)
def test_iterate_and_print(arg, expected, capfd):
    cIterator.iterate_and_print(arg)
    captured = capfd.readouterr()
    assert captured.out == expected


@pytest.mark.parametrize(
    'arg, error',
    (
            (1, "'int' object is not iterable"),
    )
)
def test_iterate_and_print_raises(arg, error):
    with pytest.raises(TypeError) as err:
        cIterator.iterate_and_print(arg)
    assert err.value.args[0] == error
