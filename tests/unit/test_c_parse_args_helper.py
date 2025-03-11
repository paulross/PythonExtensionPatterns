"""
Legacy code, see src/cpy/cParseArgsHelper.cpp for comments.
"""
import sys

import pytest

from cPyExtPatt import cParseArgsHelper
from cPyExtPatt import cPyRefs


def test_module_dir():
    assert dir(cParseArgsHelper) == [
        '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__',
        'parse_defaults_with_helper_class',
        'parse_defaults_with_helper_macro',
        'parse_mutable_defaults_with_helper_class',
        'parse_mutable_defaults_with_helper_macro',
    ]


@pytest.mark.parametrize(
    'args, expected',
    (
            (
                    (),
                    ('utf-8', 1024, 8.0),
            ),
            (
                    ('Encoding', 4219, 16.0),
                    ('Encoding', 4219, 16.0),
            ),
    ),
)
def test_parse_defaults_with_helper_macro(args, expected):
    print()
    assert cParseArgsHelper.parse_defaults_with_helper_macro(*args) == expected


@pytest.mark.parametrize(
    'args, expected',
    (
            (
                    (b'utf-8', 1024, 8.0),
                    'encoding_m must be "str", not "bytes"',
            ),
            (
                    ('utf-8', 1024.0, 8.0),
                    'the_id_m must be "int", not "float"',
            ),
            (
                    ('utf-8', 1024, 8),
                    'log_interval_m must be "float", not "int"',
            ),
    ),
)
def test_parse_defaults_with_helper_macro_raises_type_error(args, expected):
    with pytest.raises(TypeError) as err:
        cParseArgsHelper.parse_defaults_with_helper_macro(*args)
    assert err.value.args[0] == expected


def test_parse_mutable_defaults_with_helper_macro_python():
    """A local Python equivalent of cParseArgsHelper.parse_mutable_defaults_with_helper_macro()."""

    def parse_mutable_defaults_with_helper_macro(obj, default_list=[]):
        default_list.append(obj)
        return default_list

    result = parse_mutable_defaults_with_helper_macro(1)
    assert sys.getrefcount(result) == 3
    assert result == [1, ]
    result = parse_mutable_defaults_with_helper_macro(2)
    assert sys.getrefcount(result) == 3
    assert result == [1, 2]
    result = parse_mutable_defaults_with_helper_macro(3)
    assert sys.getrefcount(result) == 3
    assert result == [1, 2, 3]

    local_list = []
    assert sys.getrefcount(local_list) == 2
    assert parse_mutable_defaults_with_helper_macro(10, local_list) == [10]
    assert sys.getrefcount(local_list) == 2
    assert parse_mutable_defaults_with_helper_macro(11, local_list) == [10, 11]
    assert sys.getrefcount(local_list) == 2

    result = parse_mutable_defaults_with_helper_macro(4)
    assert result == [1, 2, 3, 4]
    assert sys.getrefcount(result) == 3


def test_parse_mutable_defaults_with_helper_macro_c():
    result = cParseArgsHelper.parse_mutable_defaults_with_helper_macro(1)
    assert sys.getrefcount(result) == 3
    assert result == [1, ]
    result = cParseArgsHelper.parse_mutable_defaults_with_helper_macro(2)
    assert sys.getrefcount(result) == 3
    assert result == [1, 2]
    result = cParseArgsHelper.parse_mutable_defaults_with_helper_macro(3)
    assert sys.getrefcount(result) == 3
    assert result == [1, 2, 3]

    local_list = []
    assert sys.getrefcount(local_list) == 2
    assert cParseArgsHelper.parse_mutable_defaults_with_helper_macro(10, local_list) == [10]
    assert sys.getrefcount(local_list) == 2
    assert cParseArgsHelper.parse_mutable_defaults_with_helper_macro(11, local_list) == [10, 11]
    assert sys.getrefcount(local_list) == 2

    result = cParseArgsHelper.parse_mutable_defaults_with_helper_macro(4)
    assert result == [1, 2, 3, 4]
    assert sys.getrefcount(result) == 3


# @pytest.mark.parametrize(
#     'args, expected',
#     (
#             (
#                     (),
#                     (4, 4, 4),
#             ),
#             (
#                     (),
#                     (4, 4, 4),
#             ),
#             (
#                     (),
#                     (4, 4, 4),
#             ),
#             (
#                     ('Encoding',),
#                     (6, 4, 4),
#             ),
#             (
#                     ('Encoding', 4219,),
#                     (6, 5, 4),
#             ),
#             (
#                     ('Encoding', 4219, 16.0),
#                     (6, 5, 4),
#             ),
#     ),
# )
# def test_parse_defaults_with_helper_macro_ref_counts(args, expected):
#     result = cParseArgsHelper.parse_defaults_with_helper_macro(*args)
#     ref_counts = tuple([cPyRefs.ref_count(v) for v in result])
#     assert ref_counts == expected
#     del result


@pytest.mark.parametrize(
    'args, expected',
    (
            (
                    (),
                    ('utf-8', 1024, 8.0),
            ),
            (
                    ('Encoding', 4219, 16.0),
                    ('Encoding', 4219, 16.0),
            ),
    ),
)
def test_parse_defaults_with_helper_class(args, expected):
    print()
    assert cParseArgsHelper.parse_defaults_with_helper_class(*args) == expected


@pytest.mark.parametrize(
    'args, expected',
    (
            (
                    (b'utf-8', 1024, 8.0),
                    'encoding_c must be "str", not "bytes"',
            ),
            (
                    ('utf-8', 1024.0, 8.0),
                    'the_id_c must be "int", not "float"',
            ),
            (
                    ('utf-8', 1024, 8),
                    'log_interval_c must be "float", not "int"',
            ),
    ),
)
def test_parse_defaults_with_helper_class_raises_type_error(args, expected):
    with pytest.raises(TypeError) as err:
        cParseArgsHelper.parse_defaults_with_helper_class(*args)
    assert err.value.args[0] == expected


# @pytest.mark.parametrize(
#     'args, expected',
#     (
#             (
#                     (),
#                     (4, 4, 4),
#             ),
#             (
#                     (),
#                     (4, 4, 4),
#             ),
#             (
#                     (),
#                     (4, 4, 4),
#             ),
#             (
#                     ('Encoding',),
#                     (6, 4, 4),
#             ),
#             (
#                     ('Encoding', 4219,),
#                     (6, 5, 4),
#             ),
#             (
#                     ('Encoding', 4219, 16.0),
#                     (6, 5, 4),
#             ),
#     ),
# )
# def test_parse_defaults_with_helper_class_ref_counts(args, expected):
#     result = cParseArgsHelper.parse_defaults_with_helper_class(*args)
#     ref_counts = tuple([cPyRefs.ref_count(v) for v in result])
#     assert ref_counts == expected
#     del result

def test_parse_mutable_defaults_with_helper_class_python():
    """A local Python equivalent of cParseArgsHelper.parse_mutable_defaults_with_helper_class()."""

    def parse_mutable_defaults_with_helper_class(obj, default_list=[]):
        default_list.append(obj)
        return default_list

    result = parse_mutable_defaults_with_helper_class(1)
    assert sys.getrefcount(result) == 3
    assert result == [1, ]
    result = parse_mutable_defaults_with_helper_class(2)
    assert sys.getrefcount(result) == 3
    assert result == [1, 2]
    result = parse_mutable_defaults_with_helper_class(3)
    assert sys.getrefcount(result) == 3
    assert result == [1, 2, 3]

    local_list = []
    assert sys.getrefcount(local_list) == 2
    assert parse_mutable_defaults_with_helper_class(10, local_list) == [10]
    assert sys.getrefcount(local_list) == 2
    assert parse_mutable_defaults_with_helper_class(11, local_list) == [10, 11]
    assert sys.getrefcount(local_list) == 2

    result = parse_mutable_defaults_with_helper_class(4)
    assert result == [1, 2, 3, 4]
    assert sys.getrefcount(result) == 3


def test_parse_mutable_defaults_with_helper_class_c():
    result = cParseArgsHelper.parse_mutable_defaults_with_helper_class(1)
    assert sys.getrefcount(result) == 3
    assert result == [1, ]
    result = cParseArgsHelper.parse_mutable_defaults_with_helper_class(2)
    assert sys.getrefcount(result) == 3
    assert result == [1, 2]
    result = cParseArgsHelper.parse_mutable_defaults_with_helper_class(3)
    assert sys.getrefcount(result) == 3
    assert result == [1, 2, 3]

    local_list = []
    assert sys.getrefcount(local_list) == 2
    assert cParseArgsHelper.parse_mutable_defaults_with_helper_class(10, local_list) == [10]
    assert sys.getrefcount(local_list) == 2
    assert cParseArgsHelper.parse_mutable_defaults_with_helper_class(11, local_list) == [10, 11]
    assert sys.getrefcount(local_list) == 2

    result = cParseArgsHelper.parse_mutable_defaults_with_helper_class(4)
    assert result == [1, 2, 3, 4]
    assert sys.getrefcount(result) == 3
