import pickle
import pickletools
import sys

import pytest

from cPyExtPatt import cPickle


def test_module_dir():
    assert dir(cPickle) == ['Custom', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__']


def test_pickle_getstate():
    custom = cPickle.Custom('FIRST', 'LAST', 11)
    pickled_value = pickle.dumps(custom)
    print()
    print(f'Pickled original is {pickled_value}')
    assert pickled_value == (b'\x80\x04\x95f\x00\x00\x00\x00\x00\x00\x00\x8c\x12cPyExtPatt.cPickle\x94'
                             b'\x8c\x06Custom\x94\x93\x94)\x81\x94}\x94(\x8c\x05first\x94\x8c\x05FIRST'
                             b'\x94\x8c\x04last\x94\x8c\x04LAST\x94\x8c\x06number\x94K\x0b\x8c\x0f_pickle_'
                             b'version\x94K\x01ub.')
    # result = pickle.loads(pickled_value)
