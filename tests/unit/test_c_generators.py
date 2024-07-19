import pytest

from cPyExtPatt.Generators import gen_c


def test_gen_c_ctor():
    result = gen_c.Generator([1, 7, 4])
    assert result
    assert type(result) is gen_c.Generator
    assert result.size() == 3


def test_gen_c_ctor_iter_forward_type():
    result = gen_c.Generator([1, 7, 4])
    iterator = result.iter_forward()
    assert iterator
    assert type(iterator) is gen_c.GeneratorIterator


def test_gen_c_ctor_iter_reverse_type():
    result = gen_c.Generator([1, 7, 4])
    iterator = result.iter_reverse()
    assert iterator
    assert type(iterator) is gen_c.GeneratorIterator


def test_gen_c_ctor_iter_forward():
    generator = gen_c.Generator([1, 7, 4])
    iterator = generator.iter_forward()
    result = [v for v in iterator]
    assert result == [1, 7, 4]


def test_gen_c_ctor_iter_reverse():
    generator = gen_c.Generator([1, 7, 4])
    iterator = generator.iter_reverse()
    result = [v for v in iterator]
    assert result == [4, 7, 1]


def test_gen_c_ctor_iter_forward_del_original():
    generator = gen_c.Generator([1, 7, 4])
    iterator = generator.iter_forward()
    del generator
    result = [v for v in iterator]
    assert result == [1, 7, 4]


def test_gen_c_ctor_iter_forward_next():
    generator = gen_c.Generator([1, 7, 4])
    iterator = generator.iter_forward()
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4


def test_gen_c_ctor_iter_forward_next_raises():
    generator = gen_c.Generator([1, 7, 4])
    iterator = generator.iter_forward()
    assert next(iterator) == 1
    assert next(iterator) == 7
    assert next(iterator) == 4
    with pytest.raises(StopIteration):
        next(iterator)


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
