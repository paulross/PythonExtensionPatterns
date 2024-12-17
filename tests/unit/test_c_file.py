import io
import sys
import pathlib
import typing

import pytest

from cPyExtPatt import cFile


@pytest.mark.parametrize(
    'arg, expected',
    (
            ('~/foo/bar.txt', '~/foo/bar.txt',),
            (pathlib.Path('~/foo/bar.txt'), '~/foo/bar.txt',),
    )
)
def test_parse_filesystem_argument(arg, expected):
    assert cFile.parse_filesystem_argument(arg) == expected


@pytest.mark.parametrize(
    'arg, expected',
    (
            ('~/foo/bar.txt', str,),
    )
)
def test_parse_filesystem_argument_return_type(arg, expected):
    assert type(cFile.parse_filesystem_argument(arg)) == expected


@pytest.mark.skipif(not (sys.version_info.minor >= 7), reason='Python 3.7+')
@pytest.mark.parametrize(
    'arg, expected',
    (
            # Number of arguments.
            (None, "function missing required argument 'path' (pos 1)"),
            ([1, 2.9], 'expected str, bytes or os.PathLike object, not list'),
    )
)
def test_parse_filesystem_argument_raises(arg, expected):
    with pytest.raises(TypeError) as err:
        if arg is None:
            cFile.parse_filesystem_argument()
        else:
            cFile.parse_filesystem_argument(arg)
    assert err.value.args[0] == expected


@pytest.mark.skipif(not (sys.version_info.minor < 7), reason='Python < 3.7')
@pytest.mark.parametrize(
    'arg, expected',
    (
            # Number of arguments.
            (None, "Required argument 'path' (pos 1) not found"),
            ([1, 2.9], 'expected str, bytes or os.PathLike object, not list'),
    )
)
def test_parse_filesystem_argument_raises_pre_37(arg, expected):
    with pytest.raises(TypeError) as err:
        if arg is None:
            cFile.parse_filesystem_argument()
        else:
            cFile.parse_filesystem_argument(arg)
    assert err.value.args[0] == expected


@pytest.mark.parametrize(
    'file_object, size, expected',
    (
            (io.BytesIO(b'Some bytes.'), 4, b'Some'),
            (io.BytesIO(b'Some bytes.'), None, b'Some bytes.'),
    )
)
def test_read_python_file_to_c(file_object, size, expected):
    if size is None:
        result = cFile.read_python_file_to_c(file_object)
    else:
        result = cFile.read_python_file_to_c(file_object, size)
    assert result == expected


@pytest.mark.parametrize(
    'bytes_to_write, expected',
    (
            (b'Some bytes.', len(b'Some bytes.')),
            (b'Some\0bytes.', len(b'Some\0bytes.')),
    )
)
def test_write_bytes_to_python_string_file(bytes_to_write, expected):
    file = io.StringIO()
    result = cFile.write_bytes_to_python_file(bytes_to_write, file)
    assert result == expected


@pytest.mark.parametrize(
    'bytes_to_write, expected',
    (
            ('Some string.', "a bytes-like object is required, not 'str'"),
    )
)
def test_write_bytes_to_python_string_file_raises(bytes_to_write, expected):
    file = io.StringIO()
    with pytest.raises(TypeError) as err:
        cFile.write_bytes_to_python_file(bytes_to_write, file)
    assert err.value.args[0] == expected


@pytest.mark.parametrize(
    'bytes_to_write, expected',
    (
            ('Some bytes.', "a bytes-like object is required, not 'str'"),
            (b'Some bytes.', "a bytes-like object is required, not 'str'"),
    )
)
def test_write_bytes_to_python_bytes_file_raises(bytes_to_write, expected):
    file = io.BytesIO()
    with pytest.raises(TypeError) as err:
        cFile.write_bytes_to_python_file(bytes_to_write, file)
    assert err.value.args[0] == expected

# TODO: Fix this. Why is position 420 when it is a 25 character write? String termination?
@pytest.mark.skip(
    reason=(
            "Fails on Python 3.9 and 3.11 with"
            " \"UnicodeDecodeError: 'ascii' codec can't decode byte 0xe3 in position 420: ordinal not in range(128)\""
    ),
)
def test_wrap_python_file():
    file = io.BytesIO()
    result = cFile.wrap_python_file(file)
    print()
    print(' Result '.center(75, '-'))
    print(result.decode('ascii'))
    print(' Result DONE '.center(75, '-'))
    print(' file.getvalue() '.center(75, '-'))
    get_value = file.getvalue()
    print(get_value)
    print(' file.getvalue() DONE '.center(75, '-'))
    assert get_value == b'Test write to python file'
    