from cPyExtPatt.SubClass import sublist


def test():
    s = sublist.SubList()
    s.append(8)
    print('test()')
    print(f'SubList.appends: {s.appends}')
    print(f'SubList now: {s}')
    return 1


if __name__ == '__main__':
    test()
