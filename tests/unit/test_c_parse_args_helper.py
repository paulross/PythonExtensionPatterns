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
