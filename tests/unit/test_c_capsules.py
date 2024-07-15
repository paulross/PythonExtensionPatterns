import datetime
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


def test_spam_capsule__C_API():
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
    assert str(
        mro) == "(<class 'datetimetz.datetimetz'>, <class 'datetime.datetime'>, <class 'datetime.date'>, <class 'object'>)"


@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            ((2024, 7, 15, 10, 21, 14), {'tzinfo': zoneinfo.ZoneInfo('Europe/London')}, '2024-07-15 10:21:14+01:00',),
    )
)
def test_datetimetz_datetimetz_str(args, kwargs, expected):
    d = datetimetz.datetimetz(*args, **kwargs)
    assert str(d) == expected


@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            (
                    (2024, 7, 15, 10, 21, 14), {'tzinfo': zoneinfo.ZoneInfo('Europe/London')},
                    "datetimetz.datetimetz(2024, 7, 15, 10, 21, 14, tzinfo=zoneinfo.ZoneInfo(key='Europe/London'))",
            ),
    )
)
def test_datetimetz_datetimetz_repr(args, kwargs, expected):
    d = datetimetz.datetimetz(*args, **kwargs)
    assert repr(d) == expected


@pytest.mark.parametrize(
    'args, kwargs, expected',
    (
            (
                    (2024, 7, 15, 10, 21, 14), {}, 'No time zone provided (self->datetime.tzinfo == NULL).',
            ),
            (
                    (2024, 7, 15, 10, 21, 14), {'tzinfo': None, },
                    'No time zone provided (self->datetime.tzinfo == NULL).',
            ),
    )
)
def test_datetimetz_datetimetz_raises(args, kwargs, expected):
    with pytest.raises(TypeError) as err:
        datetimetz.datetimetz(*args, **kwargs)
    assert err.value.args[0] == expected


def test_datetimetz_datetimetz_equal():
    d_tz = datetimetz.datetimetz(2024, 7, 15, 10, 21, 14, tzinfo=zoneinfo.ZoneInfo('Europe/London'))
    d = datetime.datetime(2024, 7, 15, 10, 21, 14, tzinfo=zoneinfo.ZoneInfo('Europe/London'))
    assert d_tz == d


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
