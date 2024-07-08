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
    assert pickled_value == b''
    # result = pickle.loads(pickled_value)
