import pytest

from cPyExtPatt import cParseArgs


def test_module_dir():
    assert dir(cParseArgs) == ['__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__', 'parse_args',
                               'parse_args_kwargs', 'parse_args_with_function_conversion_to_c',
                               'parse_args_with_immutable_defaults', 'parse_args_with_mutable_defaults',
                               'parse_no_args', 'parse_one_arg']


def test_parse_no_args():
    assert cParseArgs.parse_no_args() is None


def test_parse_no_args_raises():
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_no_args(123)
    assert err.value.args[0] == 'cPyExtPatt.cParseArgs.parse_no_args() takes no arguments (1 given)'


def test_parse_one_arg():
    assert cParseArgs.parse_one_arg(123) is None


def test_parse_one_arg_raises():
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_one_arg(123, 456)
    assert err.value.args[0] == 'cPyExtPatt.cParseArgs.parse_one_arg() takes exactly one argument (2 given)'


@pytest.mark.parametrize(
    'args, expected',
    (
            ((b'bytes', 123), 2),
            ((b'bytes', 123, 'str'), 3),
    )
)
def test_parse_args(args, expected):
    assert cParseArgs.parse_args(*args) == expected


@pytest.mark.parametrize(
    'args, expected',
    (
            # Number of arguments.
            ((), 'function takes at least 2 arguments (0 given)'),
            ((b'bytes',), 'function takes at least 2 arguments (1 given)'),
            ((b'bytes', 123, 'str', 7), 'function takes at most 3 arguments (4 given)'),
            # Type of arguments.
            (('str', 456), 'argument 1 must be bytes, not str'),
            ((b'bytes', 456.0), "'float' object cannot be interpreted as an integer"),
            ((b'bytes', '456'), "'str' object cannot be interpreted as an integer"),
            ((b'bytes', 456, 456), 'argument 3 must be str, not int'),
    )
)
def test_parse_args_raises(args, expected):
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_args(*args)
    assert err.value.args[0] == expected


@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            ((b'b', 5), {}, b'bbbbb'),
            (('b', 5), {}, 'bbbbb'),
            ((b'b',), {'count': 5}, b'bbbbb'),
            ((), {'sequence': b'b', 'count': 5}, b'bbbbb'),
            (([1, 2, 3], 3), {}, [1, 2, 3, 1, 2, 3, 1, 2, 3]),
            # NOTE: If count is absent entirely then an empty sequence of given type is returned.
            ((b'bytes',), {}, b''),
            ((b'b',), {}, b''),
    )
)
def test_parse_args_kwargs(args, kwargs, expected):
    assert cParseArgs.parse_args_kwargs(*args, **kwargs) == expected


@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            ((), {}, "function missing required argument 'sequence' (pos 1)"),
            ((5,), {'sequence': b'bytes', }, "argument for function given by name ('sequence') and position (1)"),
            ((), {'count': 2}, "function missing required argument 'sequence' (pos 1)"),
            ((), {'sequence': b'b', 'count': 5, 'foo': 27.2}, 'function takes at most 2 keyword arguments (3 given)'),
            ((b'b',), {'count': 5, 'foo': 27.2}, 'function takes at most 2 arguments (3 given)'),
    )
)
def test_parse_args_kwargs_raises(args, kwargs, expected):
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_args_kwargs(*args, **kwargs)
    assert err.value.args[0] == expected


@pytest.mark.parametrize(
    'arg, expected',
    (
            ([], 0),
            ([3, 7], 10),
    )
)
def test_parse_args_with_function_conversion_to_c(arg, expected):
    assert cParseArgs.parse_args_with_function_conversion_to_c(arg) == expected


@pytest.mark.parametrize(
    'arg, expected',
    (
            # Number of arguments.
            ((), 'check_list_of_longs(): First argument is not a list'),
            ([1, 2.9], 'check_list_of_longs(): Item 1 is not a Python integer.'),
    )
)
def test_parse_args_with_function_conversion_to_c_raises(arg, expected):
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_args_with_function_conversion_to_c(arg)
    assert err.value.args[0] == expected


@pytest.mark.parametrize(
    'args, expected',
    (
            (
                    (),
                    ('Hello world', ('Answer', 42))
            ),
            (
                    ('Some string',),
                    ('Some string', ('Answer', 42))
            ),
            (
                    ('Other string', ('Question', 456)),
                    ('Other string', ('Question', 456)),
            ),
    ),
)
def test_parse_args_with_immutable_defaults(args, expected):
    assert cParseArgs.parse_args_with_immutable_defaults(*args) == expected


# @pytest.mark.parametrize(
#     'args, expected',
#     (
#             # Number of arguments.
#             ((), 'check_list_of_longs(): First argument is not a list'),
#             ([1, 2.9], 'check_list_of_longs(): Item 1 is not a Python integer.'),
#     )
# )
# def test_parse_args_with_immutable_defaults_raises(args, expected):
#     with pytest.raises(TypeError) as err:
#         cParseArgs.parse_args_with_immutable_defaults(args)
#     assert err.value.args[0] == expected


def py_parse_args_with_mutable_defaults(obj, obj_list=[]):
    obj_list.append(obj)
    return obj_list


def test_py_parse_args_with_mutable_defaults():
    """Tests the Python behaviour."""
    local_list = []
    # print()
    # print(py_parse_args_with_mutable_defaults(1))
    # print(py_parse_args_with_mutable_defaults(2))
    # print(py_parse_args_with_mutable_defaults(3))
    # print()
    # print(py_parse_args_with_mutable_defaults(-1, local_list))
    # print(py_parse_args_with_mutable_defaults(-2, local_list))
    # print()
    # print(py_parse_args_with_mutable_defaults(4))

    assert py_parse_args_with_mutable_defaults(1) == [1, ]
    assert py_parse_args_with_mutable_defaults(2) == [1, 2, ]
    assert py_parse_args_with_mutable_defaults(3) == [1, 2, 3, ]
    assert py_parse_args_with_mutable_defaults(-1, local_list) == [-1, ]
    assert py_parse_args_with_mutable_defaults(-2, local_list) == [-1, -2]
    
    assert py_parse_args_with_mutable_defaults(4) == [1, 2, 3, 4, ]
    assert py_parse_args_with_mutable_defaults(5) == [1, 2, 3, 4, 5, ]

    assert py_parse_args_with_mutable_defaults(-3, local_list) == [-1, -2, -3, ]


def test_parse_args_with_mutable_defaults():
    """Tests the C extension behaviour."""
    local_list = []
    # print()
    # print(cParseArgs.parse_args_with_mutable_defaults(1))
    # print(cParseArgs.parse_args_with_mutable_defaults(2))
    # print(cParseArgs.parse_args_with_mutable_defaults(3))
    # print()
    # print(cParseArgs.parse_args_with_mutable_defaults(-1, local_list))
    # print(cParseArgs.parse_args_with_mutable_defaults(-2, local_list))
    # print()
    # print(cParseArgs.parse_args_with_mutable_defaults(4))

    assert cParseArgs.parse_args_with_mutable_defaults(1) == [1, ]
    assert cParseArgs.parse_args_with_mutable_defaults(2) == [1, 2, ]
    assert cParseArgs.parse_args_with_mutable_defaults(3) == [1, 2, 3, ]
    assert cParseArgs.parse_args_with_mutable_defaults(-1, local_list) == [-1, ]
    assert cParseArgs.parse_args_with_mutable_defaults(-2, local_list) == [-1, -2]

    assert cParseArgs.parse_args_with_mutable_defaults(4) == [1, 2, 3, 4, ]
    assert cParseArgs.parse_args_with_mutable_defaults(5) == [1, 2, 3, 4, 5, ]

    assert cParseArgs.parse_args_with_mutable_defaults(-3, local_list) == [-1, -2, -3, ]
