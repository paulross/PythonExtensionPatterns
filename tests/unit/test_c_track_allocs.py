import psutil
import pytest

from cPyExtPatt import cTrackAllocs


@pytest.mark.parametrize(
    'sizes, expected',
    (
            (
                    (8, 4, 7),
                    'String',
            ),
            (
                    (8 * 1024**2, 4 * 1024**2, 7 * 1024**2),
                    'String',
            ),
    )
)
def test_object_with_bytes_simple(sizes, expected):
    """"""
    assert hasattr(cTrackAllocs, "dump_remaining")
    assert hasattr(cTrackAllocs, "statistics")
    proc = psutil.Process()
    print()
    rss_start = proc.memory_info().rss
    print(f'RSS start: {rss_start:,d}')
    list_objects = []
    for size in sizes:
        list_objects.append(cTrackAllocs.ObjectWithBytes(length=size))
        print(f'cTrackAllocs.dump_remaining(): {cTrackAllocs.dump_remaining()}')
        print(f'Statistics: {cTrackAllocs.statistics()}')
        ids = [f'0x{id(v):x}' for v in list_objects]
        print(f'IDs: {ids}')
    rss = proc.memory_info().rss
    print(f'RSS mid: {rss:,d} {rss - rss_start:+,d}')
    while len(list_objects):
        list_objects.pop()
        print(f'cTrackAllocs.dump_remaining(): {cTrackAllocs.dump_remaining()}')
        print(f'Statistics: {cTrackAllocs.statistics()}')
    print('DONE...')
    print(f'cTrackAllocs.dump_remaining(): {cTrackAllocs.dump_remaining()}')
    print(f'Statistics: {cTrackAllocs.statistics()}')
    rss = proc.memory_info().rss
    print(f'RSS end: {rss:,d} {rss - rss_start:+,d}')

