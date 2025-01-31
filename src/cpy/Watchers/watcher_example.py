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


def main() -> int:
    temp()
    # dict_watcher()
    return 0


if __name__ == '__main__':
    exit(main())
