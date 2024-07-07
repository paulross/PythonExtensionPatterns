"""
Legacy code, see src/cpy/cParseArgsHelper.cpp for comments.
"""
import pytest

from cPyExtPatt import cParseArgsHelper


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
                    ('utf-8', 0, True),
            ),
            (
                    ('Encoding', 421, False),
                    ('Encoding', 421, False),
            ),
    ),
)
def test_parse_defaults_with_helper_macro(args, expected):
    assert cParseArgsHelper.parse_defaults_with_helper_macro(*args) == expected


@pytest.mark.parametrize(
    'args, expected',
    (
            (
                    (),
                    ('utf-8', 0, True),
            ),
            (
                    ('Encoding', 421, False),
                    ('Encoding', 421, False),
            ),
    ),
)
def test_parse_defaults_with_helper_class(args, expected):
    assert cParseArgsHelper.parse_defaults_with_helper_class(*args) == expected

