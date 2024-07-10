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
            (b'Some\0bytes.', len(b'Some bytes.')),
            ('Some bytes.', len(b'Some bytes.')),
    )
)
def test_write_bytes_to_python_file(bytes_to_write, expected):
    file = io.StringIO()
    result = cFile.write_bytes_to_python_file(bytes_to_write, file)
    assert result == expected
