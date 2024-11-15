import sys

import psutil
import pytest

from cPyExtPatt import cCtxMgr


def test_module_dir():
    assert dir(cCtxMgr) == ['BUFFER_LENGTH', 'ContextManager', '__doc__', '__file__', '__loader__', '__name__',
                            '__package__', '__spec__']


def test_module_BUFFER_LENGTH():
    assert cCtxMgr.BUFFER_LENGTH == 128 * 1024**2


def test_very_simple():
    print()
    with cCtxMgr.ContextManager():
        pass


def test_simple():
    print()
    with cCtxMgr.ContextManager() as context:
        assert sys.getrefcount(context) == 3
        assert context.len_buffer_lifetime() == cCtxMgr.BUFFER_LENGTH
        assert context.len_buffer_context() == cCtxMgr.BUFFER_LENGTH
    assert sys.getrefcount(context) == 2
    assert context.len_buffer_lifetime() == cCtxMgr.BUFFER_LENGTH
    assert context.len_buffer_context() == 0
    del context


def test_memory():
    proc = psutil.Process()
    print()
    print(f'RSS START: {proc.memory_info().rss:12,d}')
    for i in range(8):
        print(f'RSS START {i:5d}: {proc.memory_info().rss:12,d}')
        with cCtxMgr.ContextManager() as context:
            print(f'RSS START CTX: {proc.memory_info().rss:12,d}')
            # Does not work in the debugger due to introspection.
            # assert sys.getrefcount(context) == 3
            assert context.len_buffer_lifetime() == cCtxMgr.BUFFER_LENGTH
            assert context.len_buffer_context() == cCtxMgr.BUFFER_LENGTH
            print(f'RSS   END CTX: {proc.memory_info().rss:12,d}')
        # Does not work in the debugger due to introspection.
        # assert sys.getrefcount(context) == 2
        assert context.len_buffer_lifetime() == cCtxMgr.BUFFER_LENGTH
        assert context.len_buffer_context() == 0
        del context
        print(f'RSS   END {i:5d}: {proc.memory_info().rss:12,d}')
    print(f'RSS  END: {proc.memory_info().rss:12,d}')
    # assert 0
