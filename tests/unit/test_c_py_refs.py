import pytest

from cPyExtPatt import cPyRefs


def test_module_dir():
    assert dir(cPyRefs) == ['__doc__',
                            '__file__',
                            '__loader__',
                            '__name__',
                            '__package__',
                            '__spec__',
                            'access_after_free',
                            'dec_ref',
                            'inc_ref',
                            'leak_new_reference',
                            'make_tuple',
                            'pop_and_print_BAD',
                            'pop_and_print_OK',
                            'ref_count',
                            'subtract_two_longs',
                            ]


def test_ref_count():
    s = ''.join(dir(cPyRefs))
    assert cPyRefs.ref_count(s) == 2


def test_ref_count_inc():
    s = ''.join(dir(cPyRefs))
    original_refcount = cPyRefs.ref_count(s)
    assert original_refcount == 2
    assert cPyRefs.inc_ref(s) == original_refcount
    assert cPyRefs.ref_count(s) == original_refcount + 1
    assert cPyRefs.dec_ref(s) == original_refcount + 1
    assert cPyRefs.ref_count(s) == original_refcount


def test_subtract_two_longs():
    assert cPyRefs.subtract_two_longs() == (421 - 17)


def test_make_tuple():
    assert cPyRefs.make_tuple() == (1, 2, 'three')
