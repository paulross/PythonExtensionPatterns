import pytest

from cPyExtPatt.SimpleExample import cFibA
from cPyExtPatt.SimpleExample import cFibB


@pytest.mark.parametrize(
    'index, expected',
    (
            (1, 1,),
            (2, 1,),
            (3, 2,),
            (8, 21,),
            (30, 832040,),
    )
)
def test_cFibA_fibonacci(index, expected):
    result = cFibA.fibonacci(index)
    assert result == expected


@pytest.mark.parametrize(
    'index, expected',
    (
            (1, 1,),
            (2, 1,),
            (3, 2,),
            (8, 21,),
            (30, 832040,),
    )
)
def test_cFibB_fibonacci(index, expected):
    result = cFibB.fibonacci(index)
    assert result == expected
