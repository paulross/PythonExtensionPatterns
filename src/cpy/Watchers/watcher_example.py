"""Example of using watchers."""
import sys

import pytest

from cPyExtPatt import cWatchers


def dict_watcher_demo() -> None:
    print('dict_watcher_demo():')
    d = {}
    with cWatchers.PyDictWatcher(d):
        dd = {'age': 17, }
        d.update(dd)
        d['age'] = 42
        del d['age']
        d['name'] = 'Python'
        d.clear()
        del d


def dict_watcher_demo_refcount() -> None:
    """Checks that the reference count of the dictionary is managed correctly by the context manager."""
    print('dict_watcher_demo_refcount():')
    d = {}
    print(f'Ref count pre  {sys.getrefcount(d)}')
    ref_count = sys.getrefcount(d)
    # assert ref_count == 1
    with cWatchers.PyDictWatcher(d):
        d['age'] = 42
    print(f'Ref count post  {sys.getrefcount(d)}')
    assert sys.getrefcount(d) == ref_count


def dict_watcher_add() -> None:
    print('dict_watcher_add():')
    d = {}
    with cWatchers.PyDictWatcher(d):
        d['age'] = 42


def dict_watcher_add_and_replace() -> None:
    print('dict_watcher_add_and_replace():')
    d = {}
    d['age'] = 42
    with cWatchers.PyDictWatcher(d):
        d['age'] = 43


def dict_watcher_add_and_del() -> None:
    print('dict_watcher_add_and_del():')
    d = {}
    d['age'] = 42
    with cWatchers.PyDictWatcher(d):
        del d['age']


def dict_watcher_add_and_clear() -> None:
    print('dict_watcher_add_and_clear():')
    d = {}
    d['age'] = 42
    with cWatchers.PyDictWatcher(d):
        d.clear()


def dict_watcher_del() -> None:
    print('dict_watcher_del():')
    d = {}
    d['age'] = 42
    with cWatchers.PyDictWatcher(d):
        del d


def dict_watcher_cloned() -> None:
    print('dict_watcher_cloned():')
    d = {}
    with cWatchers.PyDictWatcher(d):
        dd = {'age': 42, }
        d.update(dd)


def dict_watcher_deallocated() -> None:
    print('dict_watcher_deallocated():')
    d = {'age': 42, }
    dd = d
    with cWatchers.PyDictWatcher(dd):
        del d
        del dd


def dict_watcher_add_no_context_manager() -> None:
    print('dict_watcher_add_no_context_manager():')
    d = {}
    watcher_id = cWatchers.py_dict_watcher_verbose_add(d)
    d['age'] = 42
    cWatchers.py_dict_watcher_verbose_remove(watcher_id, d)


def dict_watcher_dealloc_no_context_manager() -> None:
    print('dict_watcher_dealloc_no_context_manager():')
    d = {1: "42"}
    watcher_id = cWatchers.py_dict_watcher_verbose_add(d)
    del d  # Generates a PyDict_EVENT_DEALLOCATED
    # cWatchers.py_dict_watcher_verbose_remove(watcher_id, {})
    with pytest.raises(UnboundLocalError) as err:
        cWatchers.py_dict_watcher_verbose_remove(watcher_id, d)
    assert err.value.args[0] == "cannot access local variable 'd' where it is not associated with a value"


def main() -> int:
    dict_watcher_demo()
    dict_watcher_demo_refcount()
    dict_watcher_add()
    dict_watcher_add_and_replace()
    dict_watcher_add_and_del()
    dict_watcher_add_and_clear()
    dict_watcher_del()
    dict_watcher_cloned()
    dict_watcher_deallocated()
    dict_watcher_add_no_context_manager()
    dict_watcher_dealloc_no_context_manager()
    return 0


if __name__ == '__main__':
    exit(main())
