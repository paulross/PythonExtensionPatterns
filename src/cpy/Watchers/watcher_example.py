"""Example of using watchers."""

from cPyExtPatt import cWatchers


def dict_watcher_add() -> None:
    print('dict_watcher_add():')
    d = {}
    with cWatchers.PyDictWatcherContextManager(d):
        d['age'] = 42


def dict_watcher_add_and_replace() -> None:
    print('dict_watcher_add_and_replace():')
    d = {}
    d['age'] = 42
    with cWatchers.PyDictWatcherContextManager(d):
        d['age'] = 43


def dict_watcher_add_and_del() -> None:
    print('dict_watcher_add_and_del():')
    d = {}
    d['age'] = 42
    with cWatchers.PyDictWatcherContextManager(d):
        del d['age']


def dict_watcher_add_and_clear() -> None:
    print('dict_watcher_add_and_clear():')
    d = {}
    d['age'] = 42
    with cWatchers.PyDictWatcherContextManager(d):
        d.clear()


def dict_watcher_del() -> None:
    print('dict_watcher_del():')
    d = {}
    d['age'] = 42
    with cWatchers.PyDictWatcherContextManager(d):
        del d


def dict_watcher_cloned() -> None:
    print('dict_watcher_cloned():')
    d = {}
    with cWatchers.PyDictWatcherContextManager(d):
        dd = {'age': 42, }
        d.update(dd)


def dict_watcher_deallocated() -> None:
    print('dict_watcher_deallocated():')
    d = {'age': 42, }
    dd = d
    with cWatchers.PyDictWatcherContextManager(dd):
        del d
        del dd


# def temp() -> None:
#     d = {}
#     cm = cWatchers.PyDictWatcherContextManager(d)
#     cmm = cm.__enter__(d)
#     d['age'] = 42
#     d['age'] = 43
#     cmm.__exit__()
#
#
# def temp_2() -> None:
#     d = {}
#     watcher_id = cWatchers.py_dict_watcher_verbose_add(d)
#     d['age'] = 22
#     d['age'] = 23
#     del d['age']
#     cWatchers.py_dict_watcher_verbose_remove(watcher_id, d)


def main() -> int:
    # temp()
    # temp_2()
    dict_watcher_add()
    dict_watcher_add_and_replace()
    dict_watcher_add_and_del()
    dict_watcher_add_and_clear()
    dict_watcher_del()
    dict_watcher_cloned()
    dict_watcher_deallocated()
    return 0


if __name__ == '__main__':
    exit(main())
