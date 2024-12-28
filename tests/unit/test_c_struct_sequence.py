import sys

import pytest

from cPyExtPatt import cStructSequence


def test_c_struct_sequence_dir():
    result = dir(cStructSequence)
    assert result == [
        '__doc__',
        '__file__',
        '__loader__',
        '__name__',
        '__package__',
        '__spec__',
    ]
