import sys

import pytest

from cPyExtPatt import cSeqObject


def test_module_dir():
    assert dir(cSeqObject) == ['SequenceLongObject', '__doc__', '__file__', '__loader__', '__name__',
                               '__package__', '__spec__', ]


@pytest.mark.skipif(not (sys.version_info.minor < 11), reason='Python < 3.11')
def test_SequenceLongObject_dir_pre_311():
    result = dir(cSeqObject.SequenceLongObject)
    assert result == [
        '__add__',
        '__class__',
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
        '__init__',
        '__init_subclass__',
        '__le__',
        '__len__',
        '__lt__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__setitem__',
        '__sizeof__',
        '__str__',
        '__subclasshook__',
    ]


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python >= 3.11')
def test_SequenceLongObject_dir_311_plus():
    result = dir(cSeqObject.SequenceLongObject)
    assert result == [
        '__add__',
        '__class__',
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
        '__getstate__',  # New
        '__gt__',
        '__hash__',
        '__init__',
        '__init_subclass__',
        '__le__',
        '__len__',
        '__lt__',
        '__mul__',
        '__ne__',
        '__new__',
        '__reduce__',
        '__reduce_ex__',
        '__repr__',
        '__rmul__',
        '__setattr__',
        '__setitem__',
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


@pytest.mark.parametrize(
    'initial_sequence, count, expected',
    (
            (
                    [], 1, [],
            ),
            (
                    [7, 4, 1, ], 0, [],
            ),
            (
                    [7, 4, 1, ], -1, [],
            ),
            (
                    [7, 4, 1, ], 1, [7, 4, 1, ],
            ),
            (
                    [7, 4, 1, ], 2, [7, 4, 1, 7, 4, 1, ],
            ),
            (
                    [7, 4, 1, ], 3, [7, 4, 1, 7, 4, 1, 7, 4, 1, ],
            ),
    )
)
def test_SequenceLongObject_repeat(initial_sequence, count, expected):
    obj_a = cSeqObject.SequenceLongObject(initial_sequence)
    obj = obj_a * count
    print()
    assert id(obj_a) != id(obj)
    assert list(obj) == expected
    assert list(obj) == (list(obj_a) * count)


@pytest.mark.parametrize(
    'initial_sequence, index, expected',
    (
            (
                    [7, 4, 1, ], 0, 7,
            ),
            (
                    [7, 4, 1, ], 1, 4,
            ),
            (
                    [7, 4, 1, ], 2, 1,
            ),
            (
                    [7, 4, 1, ], -1, 1,
            ),
            (
                    [7, 4, 1, ], -2, 4,
            ),
            (
                    [7, 4, 1, ], -3, 7,
            ),
    )
)
def test_SequenceLongObject_item(initial_sequence, index, expected):
    obj = cSeqObject.SequenceLongObject(initial_sequence)
    assert obj[index] == expected


@pytest.mark.parametrize(
    'initial_sequence, index, expected',
    (
            (
                    [], 0, 'Index 0 is out of range for length 0',
            ),
            (
                    [], -1, 'Index -1 is out of range for length 0',
            ),
            (
                    [1, ], 2, 'Index 2 is out of range for length 1',
            ),
    )
)
def test_SequenceLongObject_item_raises(initial_sequence, index, expected):
    obj = cSeqObject.SequenceLongObject(initial_sequence)
    with pytest.raises(IndexError) as err:
        obj[index]
    assert err.value.args[0] == expected


@pytest.mark.parametrize(
    'initial_sequence, index, value, expected',
    (
            (
                    [7, 4, 1, ], 0, 14, [14, 4, 1, ],
            ),
            (
                    [7, 4, 1, ], -1, 14, [7, 4, 14, ],
            ),
            (
                    [7, 4, 1, ], -2, 14, [7, 14, 1, ],
            ),
            (
                    [7, 4, 1, ], -3, 14, [14, 4, 1, ],
            ),
    )
)
def test_SequenceLongObject_setitem(initial_sequence, index, value, expected):
    obj = cSeqObject.SequenceLongObject(initial_sequence)
    obj[index] = value
    assert list(obj) == expected


@pytest.mark.parametrize(
    'initial_sequence, index, expected',
    (
            (
                    [7, 4, 1, ], 3, 'Index 3 is out of range for length 3',
            ),
            (
                    [7, 4, 1, ], -4, 'Index -4 is out of range for length 3',
            ),
    )
)
def test_SequenceLongObject_setitem_raises(initial_sequence, index, expected):
    print()
    print(initial_sequence, index, expected)
    obj = cSeqObject.SequenceLongObject(initial_sequence)
    with pytest.raises(IndexError) as err:
        obj[index] = 100
        print(list(obj))
    assert err.value.args[0] == expected


@pytest.mark.parametrize(
    'initial_sequence, index, expected',
    (
            (
                    [7, ], 0, [],
            ),
            (
                    [7, ], -1, [],
            ),
            (
                    [7, 4, 1, ], 1, [7, 1, ],
            ),
            (
                    [7, 4, ], 0, [4, ],
            ),
            (
                    [7, 4, 1, ], -1, [7, 4, ],
            ),
            (
                    [7, 4, 1, ], -2, [7, 1, ],
            ),
            (
                    [7, 4, 1, ], -3, [4, 1, ],
            ),
    )
)
def test_SequenceLongObject_delitem(initial_sequence, index, expected):
    obj = cSeqObject.SequenceLongObject(initial_sequence)
    del obj[index]
    assert list(obj) == expected


@pytest.mark.parametrize(
    'initial_sequence, index, expected',
    (
            (
                    [], 0, 'Index 0 is out of range for length 0',
            ),
            (
                    [], -1, 'Index -1 is out of range for length 0',
            ),
            (
                    [7, ], 1, 'Index 1 is out of range for length 1',
            ),
            (
                    [7, ], -3, 'Index -3 is out of range for length 1',
            ),
    )
)
def test_SequenceLongObject_delitem_raises(initial_sequence, index, expected):
    print()
    print(initial_sequence, index, expected)
    obj = cSeqObject.SequenceLongObject(initial_sequence)
    print(list(obj))
    with pytest.raises(IndexError) as err:
        del obj[index]
    assert err.value.args[0] == expected


@pytest.mark.parametrize(
    'initial_sequence, value, expected',
    (
            (
                    [7, ], 0, False,
            ),
            (
                    [7, ], 7, True,
            ),
            (
                    [1, 4, 7, ], 7, True,
            ),
    )
)
def test_SequenceLongObject_contains(initial_sequence, value, expected):
    obj = cSeqObject.SequenceLongObject(initial_sequence)
    result = value in obj
    assert result == expected


def test_SequenceLongObject_concat_inplace():
    obj_a = cSeqObject.SequenceLongObject([7, 4, 1, ])
    obj_b = cSeqObject.SequenceLongObject([70, 40, 100, ])
    assert id(obj_a) != id(obj_b)
    obj_a += obj_b
    assert len(obj_a) == 6
    assert list(obj_a) == [7, 4, 1, ] + [70, 40, 100, ]


@pytest.mark.parametrize(
    'initial_sequence, count, expected',
    (
            (
                    [], 1, [],
            ),
            (
                    [7, 4, 1, ], 0, [],
            ),
            (
                    [7, 4, 1, ], -1, [],
            ),
            (
                    [7, 4, 1, ], 1, [7, 4, 1, ],
            ),
            (
                    [7, 4, 1, ], 2, [7, 4, 1, 7, 4, 1, ],
            ),
            (
                    [7, 4, 1, ], 3, [7, 4, 1, 7, 4, 1, 7, 4, 1, ],
            ),
    )
)
def test_SequenceLongObject_repeat_inplace(initial_sequence, count, expected):
    obj = cSeqObject.SequenceLongObject(initial_sequence)
    obj *= count
    assert list(obj) == expected
    assert list(obj) == (initial_sequence * count)
