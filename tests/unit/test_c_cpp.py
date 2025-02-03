import datetime
import sys
import zoneinfo

import psutil
import pytest

from cPyExtPatt.cpp import placement_new
from cPyExtPatt.cpp import cUnicode


# (PythonExtPatt3.11_A) $ PythonExtensionPatterns git:(develop) ✗ python
# Python 3.11.6 (v3.11.6:8b6ee5ba3b, Oct  2 2023, 11:18:21) [Clang 13.0.0 (clang-1300.0.29.30)] on darwin
# Type "help", "copyright", "credits" or "license" for more information.
# >>> from cPyExtPatt.cpp import placement_new
# >>> dir(placement_new)
# ['CppCtorDtorInPyObject', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__']
# >>> c = placement_new.CppCtorDtorInPyObject()
# -- CppCtorDtorInPyObject_new()
# Constructor at 0x10afea110 with argument "Default"
# Default constructor at 0x10afea110 with argument "Default"
# Initial self->Attr: Verbose object at 0x10afea110 m_str: "Default"
# Constructor at 0x60000043c060 with argument "pAttr"
# Initial self->pAttr: Verbose object at 0x60000043c060 m_str: "pAttr"
# >>> del c
# -- CppCtorDtorInPyObject_dealloc()
# self->Attr before delete: Verbose object at 0x10afea110 m_str: "Default"
# Destructor at 0x10afea110 m_str: "Default"
# self->pAttr before delete: Verbose object at 0x60000043c060 m_str: "pAttr"
# Destructor at 0x60000043c060 m_str: "pAttr"

def test_placement_new():
    obj = placement_new.CppCtorDtorInPyObject()
    del (obj)


@pytest.mark.parametrize(
    'count',
    (1, 4, 8,)
)
def test_placement_new_memory(count):
    """Tests repeated construction and destruction with a del call.
    TODO: This is  a flakey test measuring memory like this.
    """
    proc = psutil.Process()
    print()
    rss_start = proc.memory_info().rss
    print(f'RSS start: {rss_start:,d}')
    # Python 3.10: 65_044_684 < 10_485_760 on occasion.
    # Python 3.10: 280_023_244 < 10_485_760 on occasion.
    rss_margin = 300 * 1024 * 1024
    for i in range(count):
        obj = placement_new.CppCtorDtorInPyObject()
        buffer_size = obj.buffer_size()
        print(f'Buffer size: {buffer_size:,d}')
        rss = proc.memory_info().rss
        print(f'  RSS new: {rss:,d} {rss - rss_start:+,d}')
        assert abs(rss - rss_start - buffer_size) < rss_margin
        del (obj)
        rss = proc.memory_info().rss
        print(f'  RSS del: {rss:,d} {rss - rss_start:+,d}')
        assert abs(rss - rss_start) < rss_margin
    rss = proc.memory_info().rss
    print(f'  RSS end: {rss:,d} {rss - rss_start:+,d}')
    assert abs(rss - rss_start) < rss_margin


@pytest.mark.parametrize(
    'count',
    (1, 4, 8,)
)
def test_placement_new_memory_no_del(count):
    """Tests repeated construction and destruction with no del call.
    Within the loop the results are not really reproducible."""
    proc = psutil.Process()
    print()
    rss_start = proc.memory_info().rss
    print(f'RSS start: {rss_start:,d}')
    rss_margin = 3 * 60 * 1024 * 1024
    for i in range(count):
        obj = placement_new.CppCtorDtorInPyObject()
        buffer_size = obj.buffer_size()
        print(f'Buffer size: {buffer_size:,d}')
        rss = proc.memory_info().rss
        print(f'  RSS new: {rss:,d} {rss - rss_start:+,d}')
        # assert abs(rss - rss_start - buffer_size) < rss_margin
        rss = proc.memory_info().rss
        print(f'  RSS del: {rss:,d} {rss - rss_start:+,d}')
        # assert abs(rss - rss_start) < (rss_margin + buffer_size)
    rss = proc.memory_info().rss
    print(f'  RSS end: {rss:,d} {rss - rss_start:+,d}')
    assert abs(rss - rss_start) < (rss_margin + buffer_size)


@pytest.mark.parametrize(
    'input, expected',
    (
            ('String', 'String',),
            ("a\xac\u1234\u20ac\U00008000", 'a¬ሴ€耀',),
            ("a\xac\u1234\u20ac\U00018000", 'a¬ሴ€𘀀',),
    )
)
def test_unicode_to_string_and_back(input, expected):
    result = cUnicode.unicode_to_string_and_back(input)
    assert result == expected


@pytest.mark.parametrize(
    'input, expected',
    (
            ('String', 'String',),
            (b'String', b'String',),
            (bytearray('String', 'ascii'), bytearray(b'String'),),
    )
)
def test_py_object_to_string_and_back(input, expected):
    result = cUnicode.py_object_to_string_and_back(input)
    assert result == expected

