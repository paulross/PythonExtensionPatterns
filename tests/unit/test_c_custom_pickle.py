import io
import pickle
import pickletools
import sys

import pytest

from cPyExtPatt import cPickle


def test_module_dir():
    assert dir(cPickle) == ['Custom', '__doc__', '__file__', '__loader__', '__name__', '__package__', '__spec__']


ARGS_FOR_CUSTOM_CLASS = ('FIRST', 'LAST', 11)
PICKLE_BYTES_FOR_CUSTOM_CLASS = (b'\x80\x04\x95f\x00\x00\x00\x00\x00\x00\x00\x8c\x12cPyExtPatt.cPickle\x94'
                                 b'\x8c\x06Custom\x94\x93\x94)\x81\x94}\x94(\x8c\x05first\x94\x8c\x05FIRST'
                                 b'\x94\x8c\x04last\x94\x8c\x04LAST\x94\x8c\x06number\x94K\x0b\x8c\x0f_pickle_'
                                 b'version\x94K\x01ub.')


def test_pickle_getstate():
    custom = cPickle.Custom(*ARGS_FOR_CUSTOM_CLASS)
    pickled_value = pickle.dumps(custom)
    print()
    print(f'Pickled original is {pickled_value}')
    assert pickled_value == PICKLE_BYTES_FOR_CUSTOM_CLASS
    # result = pickle.loads(pickled_value)


def test_pickle_setstate():
    custom = pickle.loads(PICKLE_BYTES_FOR_CUSTOM_CLASS)
    assert custom.first == 'FIRST'
    assert custom.last == 'LAST'
    assert custom.number == 11


def test_pickle_round_trip():
    custom = cPickle.Custom(*ARGS_FOR_CUSTOM_CLASS)
    pickled_value = pickle.dumps(custom)
    result = pickle.loads(pickled_value)
    assert id(result) != id(custom)


def test_pickletools():
    outfile = io.StringIO()
    pickletools.dis(PICKLE_BYTES_FOR_CUSTOM_CLASS, out=outfile, annotate=1)
    result = outfile.getvalue()
    # print()
    # print(result)
    expected = """    0: \\x80 PROTO      4 Protocol version indicator.
    2: \\x95 FRAME      102 Indicate the beginning of a new frame.
   11: \\x8c SHORT_BINUNICODE 'cPyExtPatt.cPickle' Push a Python Unicode string object.
   31: \\x94 MEMOIZE    (as 0)                     Store the stack top into the memo.  The stack is not popped.
   32: \\x8c SHORT_BINUNICODE 'Custom'             Push a Python Unicode string object.
   40: \\x94 MEMOIZE    (as 1)                     Store the stack top into the memo.  The stack is not popped.
   41: \\x93 STACK_GLOBAL                          Push a global object (module.attr) on the stack.
   42: \\x94 MEMOIZE    (as 2)                     Store the stack top into the memo.  The stack is not popped.
   43: )    EMPTY_TUPLE                           Push an empty tuple.
   44: \\x81 NEWOBJ                                Build an object instance.
   45: \\x94 MEMOIZE    (as 3)                     Store the stack top into the memo.  The stack is not popped.
   46: }    EMPTY_DICT                            Push an empty dict.
   47: \\x94 MEMOIZE    (as 4)                     Store the stack top into the memo.  The stack is not popped.
   48: (    MARK                                  Push markobject onto the stack.
   49: \\x8c     SHORT_BINUNICODE 'first'          Push a Python Unicode string object.
   56: \\x94     MEMOIZE    (as 5)                 Store the stack top into the memo.  The stack is not popped.
   57: \\x8c     SHORT_BINUNICODE 'FIRST'          Push a Python Unicode string object.
   64: \\x94     MEMOIZE    (as 6)                 Store the stack top into the memo.  The stack is not popped.
   65: \\x8c     SHORT_BINUNICODE 'last'           Push a Python Unicode string object.
   71: \\x94     MEMOIZE    (as 7)                 Store the stack top into the memo.  The stack is not popped.
   72: \\x8c     SHORT_BINUNICODE 'LAST'           Push a Python Unicode string object.
   78: \\x94     MEMOIZE    (as 8)                 Store the stack top into the memo.  The stack is not popped.
   79: \\x8c     SHORT_BINUNICODE 'number'         Push a Python Unicode string object.
   87: \\x94     MEMOIZE    (as 9)                 Store the stack top into the memo.  The stack is not popped.
   88: K        BININT1    11                     Push a one-byte unsigned integer.
   90: \\x8c     SHORT_BINUNICODE '_pickle_version' Push a Python Unicode string object.
  107: \\x94     MEMOIZE    (as 10)                 Store the stack top into the memo.  The stack is not popped.
  108: K        BININT1    1                       Push a one-byte unsigned integer.
  110: u        SETITEMS   (MARK at 48)            Add an arbitrary number of key+value pairs to an existing dict.
  111: b    BUILD                                  Finish building an object, via __setstate__ or dict update.
  112: .    STOP                                   Stop the unpickling machine.
highest protocol among opcodes = 4
"""
    assert result == expected
