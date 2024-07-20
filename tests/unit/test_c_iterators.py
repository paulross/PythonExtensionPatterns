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


def test_c_iterator_ctor():
    result = cIterator.SequenceOfLong([1, 7, 4])
    assert result
    assert type(result) is cIterator.SequenceOfLong
    assert result.size() == 3


def test_c_iterator_ctor_iter_forward_type():
    result = cIterator.SequenceOfLong([1, 7, 4])
    iterator = result.iter_forward()
    assert iterator
    assert type(iterator) is cIterator.SequenceOfLongIterator


def test_c_iterator_ctor_iter_reverse_type():
    result = cIterator.SequenceOfLong([1, 7, 4])
    iterator = result.iter_reverse()
    assert iterator
    assert type(iterator) is cIterator.SequenceOfLongIterator


def test_c_iterator_ctor_iter_forward():
    generator = cIterator.SequenceOfLong([1, 7, 4])
    iterator = generator.iter_forward()
    result = [v for v in iterator]
    assert result == [1, 7, 4]


def test_c_iterator_ctor_iter_reverse():
    generator = cIterator.SequenceOfLong([1, 7, 4])
    iterator = generator.iter_reverse()
    result = [v for v in iterator]
    assert result == [4, 7, 1]


def test_c_iterator_ctor_iter_forward_del_original():
    generator = cIterator.SequenceOfLong([1, 7, 4])
    iterator = generator.iter_forward()
    del generator
    result = [v for v in iterator]
    assert result == [1, 7, 4]


def test_c_iterator_ctor_iter_forward_next():
    generator = cIterator.SequenceOfLong([1, 7, 4])
    iterator = generator.iter_forward()
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4


def test_c_iterator_ctor_iter_forward_next_raises():
    generator = cIterator.SequenceOfLong([1, 7, 4])
    iterator = generator.iter_forward()
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4
    with pytest.raises(StopIteration):
        next(iterator)


def yield_from_a_generator(gen):
    for value in gen:
        yield value


def test_c_iterator_yield_forward():
    generator_object = cIterator.SequenceOfLong([1, 7, 4])
    iterator = generator_object.iter_forward()
    result = []
    for v in yield_from_a_generator(iterator):
        result.append(v)
    assert result == [1, 7, 4]


def test_c_iterator_yield_reverse():
    generator_object = cIterator.SequenceOfLong([1, 7, 4])
    iterator = generator_object.iter_reverse()
    result = []
    for v in yield_from_a_generator(iterator):
        result.append(v)
    assert result == [4, 7, 1]


def test_gen_iter_forward_next():
    generator = cIterator.SequenceOfLong([1, 7, 4])
    iterator = iter(generator)
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4

# @pytest.mark.parametrize(
#     'index, expected',
#     (
#             (1, 1,),
#             (2, 1,),
#             (3, 2,),
#             (8, 21,),
#             (30, 832040,),
#     )
# )
# def test_cFibB_fibonacci(index, expected):
#     result = cFibB.fibonacci(index)
#     assert result == expected
