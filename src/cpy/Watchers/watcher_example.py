"""Example of using watchers."""

from cPyExtPatt import cWatchers


def dict_watcher() -> None:
    d = {}
    with cWatchers.PyDictWatcherContextManager(d):
        d['age'] = 42


def temp() -> None:
    d = {}
    cm = cWatchers.PyDictWatcherContextManager(d)
    cmm = cm.__enter__(d)
    d['age'] = 42
    d['age'] = 43
    cmm.__exit__()


def temp_2() -> None:
    d = {}
    watcher_id = cWatchers.py_dict_watcher_verbose_add(d)
    d['age'] = 22
    d['age'] = 23
    del d['age']
    cWatchers.py_dict_watcher_verbose_remove(watcher_id, d)


def main() -> int:
    # temp()
    temp_2()
    dict_watcher()
    return 0


if __name__ == '__main__':
    exit(main())
