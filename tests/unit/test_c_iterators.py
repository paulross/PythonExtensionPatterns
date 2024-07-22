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
                      '__spec__']


def test_c_iterator_sequence_of_long_dir():
    result = dir(cIterator.SequenceOfLong)
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
                      '__le__',
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
                      'iter_forward',
                      'iter_reverse',
                      'size']


def test_c_iterator_sequence_of_long_iterator_dir():
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


def test_c_iterator_ctor_iter_forward_type():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = sequence.iter_forward()
    assert iterator
    assert type(iterator) is cIterator.SequenceOfLongIterator


def test_c_iterator_ctor_iter_reverse_type():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = sequence.iter_reverse()
    assert iterator
    assert type(iterator) is cIterator.SequenceOfLongIterator


def test_c_iterator_ctor_iter_forward():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    # iterator = sequence.iter_forward()
    # result = [v for v in iterator]
    result = [v for v in sequence]
    assert result == [1, 7, 4]


def test_c_iterator_ctor_iter_reverse():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = sequence.iter_reverse()
    result = [v for v in iterator]
    assert result == [4, 7, 1]


def test_c_iterator_ctor_iter_forward_del_original():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = sequence.iter_forward()
    del sequence
    result = [v for v in iterator]
    assert result == [1, 7, 4]


def test_c_iterator_ctor_iter_forward_next():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = sequence.iter_forward()
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4


def test_c_iterator_ctor_iter_forward_next_raises():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = sequence.iter_forward()
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4
    with pytest.raises(StopIteration):
        next(iterator)


def yield_from_a_iterator(iter):
    for value in iter:
        yield value


def test_c_iterator_yield_forward():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = sequence.iter_forward()
    result = []
    for v in yield_from_a_iterator(iterator):
        result.append(v)
    assert result == [1, 7, 4]


def test_c_iterator_yield_reverse():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = sequence.iter_reverse()
    result = []
    for v in yield_from_a_iterator(iterator):
        result.append(v)
    assert result == [4, 7, 1]


def test_c_iterator_iter_forward_next():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    iterator = iter(sequence.iter_forward())
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4


def test_c_iterator_sorted():
    sequence = cIterator.SequenceOfLong([1, 7, 4])
    result = sorted(sequence.iter_forward())
    print()
    print(result)
    assert result == [1, 4, 7, ]


def test_modify_list_during_iteration_a():
    lst = list(range(8))
    print()
    for i, value in enumerate(lst):
        print(f'i={i} value={value}')
        del lst[i]


def test_modify_list_during_iteration_b():
    lst = list(range(8))
    print()
    for i, value in enumerate(lst):
        print(f'i={i} value={value}')
        lst.pop()
