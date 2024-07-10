import sys

import pytest

from cPyExtPatt import cParseArgs


def test_module_dir():
    assert dir(cParseArgs) == ['__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__', 'parse_args',
                               'parse_args_kwargs', 'parse_args_with_function_conversion_to_c',
                               'parse_args_with_immutable_defaults', 'parse_args_with_mutable_defaults',
                               'parse_default_bytes_object',
                               'parse_no_args', 'parse_one_arg', 'parse_pos_only_kwd_only', ]


def test_parse_no_args():
    assert cParseArgs.parse_no_args() is None


@pytest.mark.skipif(not (sys.version_info.minor > 8), reason='Python > 3.8')
def test_parse_no_args_raises():
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_no_args(123)
    assert err.value.args[0] == 'cPyExtPatt.cParseArgs.parse_no_args() takes no arguments (1 given)'


@pytest.mark.skipif(not (sys.version_info.minor <= 8), reason='Python <= 3.8')
def test_parse_no_args_raises_pre_39():
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_no_args(123)
    assert err.value.args[0] == 'parse_no_args() takes no arguments (1 given)'


def test_parse_one_arg():
    assert cParseArgs.parse_one_arg(123) is None


@pytest.mark.skipif(not (sys.version_info.minor > 8), reason='Python > 3.8')
def test_parse_one_arg_raises():
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_one_arg(123, 456)
    assert err.value.args[0] == 'cPyExtPatt.cParseArgs.parse_one_arg() takes exactly one argument (2 given)'


@pytest.mark.skipif(not (sys.version_info.minor <= 8), reason='Python <= 3.8')
def test_parse_one_arg_raises_pre_39():
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_one_arg(123, 456)
    assert err.value.args[0] == 'parse_one_arg() takes exactly one argument (2 given)'


@pytest.mark.parametrize(
    'args, expected',
    (
            ((b'bytes', 123), (b'bytes', 123, 'default_string')),
            ((b'bytes', 123, 'local_string'), (b'bytes', 123, 'local_string')),
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
            ((b'bytes', 456, 456), 'argument 3 must be str, not int'),
    )
)
def test_parse_args_raises(args, expected):
    """Signature is::

        def parse_args(a: bytes, b: int, c: str = '') -> int:
    """
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_args(*args)
    assert err.value.args[0] == expected


@pytest.mark.skipif(sys.version_info.minor > 9, reason='Python <= 3.9')
@pytest.mark.parametrize(
    'args, expected',
    (
            ((b'bytes', 456.0), "integer argument expected, got float"),
            ((b'bytes', '456'), "an integer is required (got type str)"),
    )
)
def test_parse_args_raises_conversion_old(args, expected):
    """Signature is::

        def parse_args(a: bytes, b: int, c: str = '') -> int:
    """
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_args(*args)
    assert err.value.args[0] == expected


@pytest.mark.skipif(sys.version_info.minor <= 9, reason='Python > 3.9')
@pytest.mark.parametrize(
    'args, expected',
    (
            ((b'bytes', 456.0), "'float' object cannot be interpreted as an integer"),
            ((b'bytes', '456'), "'str' object cannot be interpreted as an integer"),
    )
)
def test_parse_args_raises_conversion(args, expected):
    """Signature is::

        def parse_args(a: bytes, b: int, c: str = '') -> int:
    """
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
            # NOTE: If count is zero then an empty sequence of given type is returned.
            ((b'bytes', 0,), {}, b''),
            ((b'b', 0,), {}, b''),
            # NOTE: If count is absent then it defaults to 1.
            ((b'bytes',), {}, b'bytes'),
            ((b'b',), {}, b'b'),
            # args/kwargs are None
            (None, {'sequence': b'b', 'count': 5}, b'bbbbb'),
            (('b', 5), None, 'bbbbb'),
    )
)
def test_parse_args_kwargs(args, kwargs, expected):
    if args is None:
        if kwargs is None:
            assert cParseArgs.parse_args_kwargs() == expected
        else:
            assert cParseArgs.parse_args_kwargs(**kwargs) == expected
    elif kwargs is None:
        assert cParseArgs.parse_args_kwargs(*args) == expected
    else:
        assert cParseArgs.parse_args_kwargs(*args, **kwargs) == expected
    # assert cParseArgs.parse_args_kwargs(*args, **kwargs) == expected


def test_parse_args_kwargs_examples():
    """Variations on the signature::

        def parse_args_kwargs(sequence=typing.Sequence[typing.Any], count: int = 1) -> typing.Sequence[typing.Any]:
    """
    assert cParseArgs.parse_args_kwargs([1, 2, 3], 2) == [1, 2, 3, 1, 2, 3]
    assert cParseArgs.parse_args_kwargs([1, 2, 3], count=2) == [1, 2, 3, 1, 2, 3]
    assert cParseArgs.parse_args_kwargs(sequence=[1, 2, 3], count=2) == [1, 2, 3, 1, 2, 3]


@pytest.mark.skipif(not (7 <= sys.version_info.minor), reason='Python 3.7+')
@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            ((), {}, "function missing required argument 'sequence' (pos 1)"),
            ((5,), {'sequence': b'bytes', }, "argument for function given by name ('sequence') and position (1)"),
            ((), {'count': 2}, "function missing required argument 'sequence' (pos 1)"),
            ((), {'sequence': b'b', 'count': 5, 'foo': 27.2}, 'function takes at most 2 keyword arguments (3 given)'),
            ((b'b',), {'count': 5, 'foo': 27.2}, 'function takes at most 2 arguments (3 given)'),
            # args/kwargs are None
            (None, {'count': 5, }, "function missing required argument 'sequence' (pos 1)"),
            (None, None, "function missing required argument 'sequence' (pos 1)"),
    )
)
def test_parse_args_kwargs_raises(args, kwargs, expected):
    with pytest.raises(TypeError) as err:
        if args is None:
            if kwargs is None:
                cParseArgs.parse_args_kwargs()
            else:
                cParseArgs.parse_args_kwargs(**kwargs)
        elif kwargs is None:
            cParseArgs.parse_args_kwargs(*args)
        else:
            cParseArgs.parse_args_kwargs(*args, **kwargs)
    assert err.value.args[0] == expected


@pytest.mark.skipif(not (sys.version_info.minor < 7), reason='Python < 3.7')
@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            ((), {}, "Required argument 'sequence' (pos 1) not found"),
            ((5,), {'sequence': b'bytes', }, "Argument given by name ('sequence') and position (1)"),
            ((), {'count': 2}, "Required argument 'sequence' (pos 1) not found"),
            ((), {'sequence': b'b', 'count': 5, 'foo': 27.2}, 'function takes at most 2 arguments (3 given)'),
            ((b'b',), {'count': 5, 'foo': 27.2}, 'function takes at most 2 arguments (3 given)'),
            # args/kwargs are None
            (None, {'count': 5, }, "Required argument 'sequence' (pos 1) not found"),
            (None, None, "Required argument 'sequence' (pos 1) not found"),
    )
)
def test_parse_args_kwargs_raises_pre_37(args, kwargs, expected):
    with pytest.raises(TypeError) as err:
        if args is None:
            if kwargs is None:
                cParseArgs.parse_args_kwargs()
            else:
                cParseArgs.parse_args_kwargs(**kwargs)
        elif kwargs is None:
            cParseArgs.parse_args_kwargs(*args)
        else:
            cParseArgs.parse_args_kwargs(*args, **kwargs)
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


@pytest.mark.parametrize(
    'value, expected',
    (
            (None, b"default"),
            (b'local_value', b'local_value'),
    )
)
def test_parse_default_bytes_object(value, expected):
    """Signature is::

        def parse_default_bytes_object(b: bytes = b"default") -> bytes:
    """
    if value is None:
        result = cParseArgs.parse_default_bytes_object()
    else:
        result = cParseArgs.parse_default_bytes_object(value)
    assert result == expected


# @pytest.mark.parametrize(
#     'args, expected',
#     (
#             ((b'bytes', 123), (b'bytes', 123, 'default_string')),
#             ((b'bytes', 123, 'local_string'), (b'bytes', 123, 'local_string')),
#     )
# )
# def test_parse_pos_only_kwd_only(args, expected):
def test_parse_pos_only_kwd_only():
    """Signature is::

        def parse_pos_only_kwd_only(pos1: str, pos2: int, /, pos_or_kwd: bytes, *, kwd1: float, kwd2: int) -> typing.Tuple[typing.Any, ...]
    """
    result = cParseArgs.parse_pos_only_kwd_only('pos1', 12, b'pos_or_keyword')
    print()
    print(result)
    assert result == ('pos1', 12, b'pos_or_keyword', 256.0, -421)
    result = cParseArgs.parse_pos_only_kwd_only('pos1', 12, pos_or_kwd=b'pos_or_keyword')
    print()
    print(result)
    assert result == ('pos1', 12, b'pos_or_keyword', 256.0, -421)
    result = cParseArgs.parse_pos_only_kwd_only('pos1', 12, pos_or_kwd=b'pos_or_keyword', kwd1=8.0, kwd2=16)
    print()
    print(result)
    assert result == ('pos1', 12, b'pos_or_keyword', 8.0, 16)


@pytest.mark.skipif(not (7 <= sys.version_info.minor), reason='Python 3.7+')
@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            # Number of arguments.
            ((), {}, 'function takes at least 2 positional arguments (0 given)'),
            (('pos1', 12,), {}, "function missing required argument 'pos_or_kwd' (pos 3)"),
            (('pos1', 12, b'pos_or_keyword', 'kwd1'), {}, 'function takes at most 3 positional arguments (4 given)'),
            # See: test_parse_pos_only_kwd_only_raises_3_13
            # (('pos1', 12, b'pos_or_keyword'), {'pos1': 'pos1'},
            #  "'pos1' is an invalid keyword argument for this function"),
    )
)
def test_parse_pos_only_kwd_only_raises(args, kwargs, expected):
    """Signature is::

        def parse_pos_only_kwd_only(pos1: str, pos2: int, /, pos_or_kwd: bytes, *, kwd1: float, kwd2: int):
    """
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_pos_only_kwd_only(*args, **kwargs)
    assert err.value.args[0] == expected


@pytest.mark.skipif(not (sys.version_info.minor < 7), reason='Python < 3.7')
@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            # Number of arguments.
            ((), {}, 'Function takes at least 2 positional arguments (0 given)'),
            (('pos1', 12,), {}, "Required argument 'pos_or_kwd' (pos 3) not found"),
            (('pos1', 12, b'pos_or_keyword', 'kwd1'), {},
             'Function takes at most 3 positional arguments (4 given)'),
    )
)
def test_parse_pos_only_kwd_only_raises_pre_37(args, kwargs, expected):
    """Signature is::

        def parse_pos_only_kwd_only(pos1: str, pos2: int, /, pos_or_kwd: bytes, *, kwd1: float, kwd2: int):
    """
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_pos_only_kwd_only(*args, **kwargs)
    assert err.value.args[0] == expected


@pytest.mark.skipif(sys.version_info.minor >= 13, reason='Python 3.13 changed the error message.')
@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            (
                    ('pos1', 12, b'pos_or_keyword'), {'pos1': 'pos1'},
                    "'pos1' is an invalid keyword argument for this function",
            ),
    )
)
def test_parse_pos_only_kwd_only_raises_before_3_13(args, kwargs, expected):
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_pos_only_kwd_only(*args, **kwargs)
    assert err.value.args[0] == expected


@pytest.mark.skipif(sys.version_info.minor < 13, reason='Python 3.13 changed the error message.')
@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            (
                    ('pos1', 12, b'pos_or_keyword'), {'pos1': 'pos1'},
                    "this function got an unexpected keyword argument 'pos1'",
            ),
    )
)
def test_parse_pos_only_kwd_only_raises_after_3_13(args, kwargs, expected):
    with pytest.raises(TypeError) as err:
        cParseArgs.parse_pos_only_kwd_only(*args, **kwargs)
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
