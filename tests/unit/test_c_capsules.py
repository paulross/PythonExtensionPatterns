import datetime
import sys
import zoneinfo

import pytest

from cPyExtPatt.Capsules import spam
from cPyExtPatt.Capsules import spam_capsule
from cPyExtPatt.Capsules import spam_client
from cPyExtPatt.Capsules import datetimetz


def test_spam():
    result = spam.system("ls -l")
    assert result == 0


def test_spam_capsule():
    result = spam_capsule.system("ls -l")
    assert result == 0


@pytest.mark.skipif(not (sys.version_info.minor <= 10), reason='Python 3.9, 3.10')
def test_spam_capsule__C_API_39_310():
    print()
    print(spam_capsule._C_API)
    assert str(spam_capsule._C_API).startswith('<capsule object "cPyExtPatt.Capsules.spam_capsule._C_API" at 0x')
    print(dir(spam_capsule._C_API))
    assert dir(spam_capsule._C_API) == ['__class__', '__delattr__', '__dir__', '__doc__', '__eq__', '__format__',
                                        '__ge__', '__getattribute__', '__gt__', '__hash__', '__init__',
                                        '__init_subclass__', '__le__', '__lt__', '__ne__', '__new__', '__reduce__',
                                        '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__',
                                        '__subclasshook__']
    print(type(spam_capsule._C_API).__mro__)
    assert str(type(spam_capsule._C_API).__mro__) == "(<class 'PyCapsule'>, <class 'object'>)"


@pytest.mark.skipif(not (sys.version_info.minor >= 11), reason='Python 3.11+')
def test_spam_capsule__C_API_311_plus():
    print()
    print(spam_capsule._C_API)
    assert str(spam_capsule._C_API).startswith('<capsule object "cPyExtPatt.Capsules.spam_capsule._C_API" at 0x')
    print(dir(spam_capsule._C_API))
    assert dir(spam_capsule._C_API) == ['__class__', '__delattr__', '__dir__', '__doc__', '__eq__', '__format__',
                                        '__ge__', '__getattribute__', '__getstate__', '__gt__', '__hash__', '__init__',
                                        '__init_subclass__', '__le__', '__lt__', '__ne__', '__new__', '__reduce__',
                                        '__reduce_ex__', '__repr__', '__setattr__', '__sizeof__', '__str__',
                                        '__subclasshook__']
    print(type(spam_capsule._C_API).__mro__)
    assert str(type(spam_capsule._C_API).__mro__) == "(<class 'PyCapsule'>, <class 'object'>)"


def test_spam_client():
    result = spam_client.system("ls -l")
    assert result == 0


def test_datetimetz_datetimetz_mro():
    mro = datetimetz.datetimetz.__mro__
    assert [str(v) for v in mro] == [
        "<class 'datetimetz.datetimetz'>",
        "<class 'datetime.datetime'>",
        "<class 'datetime.date'>",
        "<class 'object'>",
    ]


@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            (
                    (2024, 7, 15, 10, 21, 14),
                    {'tzinfo': zoneinfo.ZoneInfo('Europe/London')},
                    '2024-07-15 10:21:14+01:00',
            ),
    )
)
def test_datetimetz_datetimetz_str(args, kwargs, expected):
    d = datetimetz.datetimetz(*args, **kwargs)
    assert str(d) == expected


@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            (
                    (2024, 7, 15, 10, 21, 14),
                    {'tzinfo': zoneinfo.ZoneInfo('Europe/London')},
                    (
                            "datetimetz.datetimetz(2024, 7, 15, 10, 21, 14,"
                            " tzinfo=zoneinfo.ZoneInfo(key='Europe/London'))"
                    ),
            ),
    )
)
def test_datetimetz_datetimetz_repr(args, kwargs, expected):
    d = datetimetz.datetimetz(*args, **kwargs)
    assert repr(d) == expected


def test_datetimetz_datetimetz_get_attributes():
    d = datetimetz.datetimetz(
        2024, 7, 15, 10, 21, 14,
        tzinfo=zoneinfo.ZoneInfo('Europe/London')
    )
    assert d.year == 2024
    assert d.month == 7
    assert d.day == 15
    assert d.hour == 10
    assert d.minute == 21
    assert d.second == 14
    assert d.microsecond == 0
    assert d.tzinfo == zoneinfo.ZoneInfo('Europe/London')


def test_datetimetz_datetimetz_set_tzinfo_raises():
    d = datetimetz.datetimetz(
        2024, 7, 15, 10, 21, 14,
        tzinfo=zoneinfo.ZoneInfo('Europe/London')
    )
    with pytest.raises(AttributeError) as err:
        d.tzinfo = None
    assert err.value.args[0] == "attribute 'tzinfo' of 'datetime.datetime' objects is not writable"


@pytest.mark.skipif(
    sys.version_info.minor == 9,
    reason="Fails on Python 3.9 with \"Failed: DID NOT RAISE <class 'TypeError'>\"",
)
@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            (
                    (2024, 7, 15, 10, 21, 14),
                    {},
                    'No time zone provided.',
            ),
            (
                    (2024, 7, 15, 10, 21, 14),
                    {'tzinfo': None, },
                    'No time zone provided.',
            ),
    )
)
def test_datetimetz_datetimetz_raises(args, kwargs, expected):
    with pytest.raises(TypeError) as err:
        d = datetimetz.datetimetz(*args, **kwargs)
        print()
        print(f'ERROR: {repr(d)}')
    assert err.value.args[0] == expected


def test_datetimetz_datetimetz_equal():
    d_tz = datetimetz.datetimetz(
        2024, 7, 15, 10, 21, 14,
        tzinfo=zoneinfo.ZoneInfo('Europe/London'))
    d = datetime.datetime(
        2024, 7, 15, 10, 21, 14,
        tzinfo=zoneinfo.ZoneInfo('Europe/London')
    )
    assert d_tz == d


def test_datetime_datetime_equal_naive():
    d = datetime.datetime(
        2024, 7, 15, 10, 21, 14,
        tzinfo=zoneinfo.ZoneInfo('Europe/London')
    )
    d_no_tz = datetime.datetime(2024, 7, 15, 10, 21, 14)
    assert d_no_tz != d


def test_datetimetz_datetimetz_equal_naive():
    d_tz = datetimetz.datetimetz(
        2024, 7, 15, 10, 21, 14,
        tzinfo=zoneinfo.ZoneInfo('Europe/London')
    )
    d = datetime.datetime(2024, 7, 15, 10, 21, 14)
    assert d_tz != d


@pytest.mark.parametrize(
    'd_tz, d, expected',
    (
            (
                    datetimetz.datetimetz(2024, 7, 15, 10, 21, 14, tzinfo=zoneinfo.ZoneInfo('Europe/London')),
                    datetime.datetime(2024, 7, 15, 10, 21, 14, tzinfo=zoneinfo.ZoneInfo('Europe/London')),
                    datetime.timedelta(0),
            ),
            (
                    datetimetz.datetimetz(2024, 7, 15, 10, 21, 14, tzinfo=zoneinfo.ZoneInfo('Europe/London')),
                    datetime.datetime(2024, 7, 15, 10, 21, 14, tzinfo=zoneinfo.ZoneInfo('America/New_York')),
                    datetime.timedelta(seconds=-5 * 60 * 60),
            ),
            (
                    datetimetz.datetimetz(2024, 7, 15, 10, 21, 14, tzinfo=zoneinfo.ZoneInfo('America/New_York')),
                    datetime.datetime(2024, 7, 15, 10, 21, 14, tzinfo=zoneinfo.ZoneInfo('Europe/London')),
                    datetime.timedelta(seconds=5 * 60 * 60),
            ),
    )
)
def test_datetimetz_datetimetz_subtract(d_tz, d, expected):
    assert (d_tz - d) == expected


@pytest.mark.parametrize(
    'd_tz, d, expected',
    (
            (
                    datetimetz.datetimetz(2024, 7, 15, 10, 21, 14, tzinfo=zoneinfo.ZoneInfo('Europe/London')),
                    datetime.datetime(2024, 7, 15, 10, 21, 14),
                    '',
            ),
    )
)
def test_datetimetz_datetimetz_subtract_raises(d_tz, d, expected):
    with pytest.raises(TypeError) as err:
        d_tz - d
    assert err.value.args[0] == "can't subtract offset-naive and offset-aware datetimes"


def test_datetimetz_datetimetz_replace_year():
    d = datetimetz.datetimetz(
        2024, 7, 15, 10, 21, 14,
        tzinfo=zoneinfo.ZoneInfo('Europe/London')
    )
    d_replace = d.replace(year=2025)
    print()
    print(type(d_replace))
    assert type(d_replace) == datetimetz.datetimetz
    assert d_replace.tzinfo is not None
    assert d_replace.year == 2025
    assert d_replace == datetimetz.datetimetz(
        2025, 7, 15, 10, 21, 14,
        tzinfo=zoneinfo.ZoneInfo('Europe/London')
    )


def test_datetimetz_datetimetz_replace_raises_tzinfo():
    d = datetimetz.datetimetz(
        2024, 7, 15, 10, 21, 14,
        tzinfo=zoneinfo.ZoneInfo('Europe/London')
    )
    with pytest.raises(TypeError) as err:
        d.replace(tzinfo=None)
        print()
        print(f'ERROR: {repr(d)}')
        print(f'ERROR: {repr(d.tzinfo)}')
    assert err.value.args[0] == 'No time zone provided.'


@pytest.mark.skipif(not (sys.version_info.minor <= 9), reason='Python 3.9')
def test_datetimetz_datetimetz_replace_raises_year_none_39_310():
    d = datetimetz.datetimetz(
        2024, 7, 15, 10, 21, 14,
        tzinfo=zoneinfo.ZoneInfo('Europe/London')
    )
    with pytest.raises(TypeError) as err:
        d.replace(year=None)
    assert err.value.args[0] == "an integer is required (got type NoneType)"


@pytest.mark.skipif(not (sys.version_info.minor >= 10), reason='Python 3.10+')
def test_datetimetz_datetimetz_replace_raises_year_none():
    d = datetimetz.datetimetz(
        2024, 7, 15, 10, 21, 14,
        tzinfo=zoneinfo.ZoneInfo('Europe/London')
    )
    with pytest.raises(TypeError) as err:
        d.replace(year=None)
    assert err.value.args[0] == "'NoneType' object cannot be interpreted as an integer"
