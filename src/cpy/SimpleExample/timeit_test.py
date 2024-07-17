import timeit

index = 32
number = 20

print(f'Index: {index} number of times: {number}')
print('Version A, no cacheing:')
# Use pFibA
ti_py = timeit.timeit(f'pFibA.fibonacci({index})', setup='import pFibA', number=number)
print(f'Python timeit: {ti_py:8.6f}')

# Use cFibA
ti_c = timeit.timeit(f'cFibA.fibonacci({index})',
                     setup='from cPyExtPatt.SimpleExample import cFibA', number=number)
print(f'     C timeit: {ti_c:8.6f}')

print(
    f'C is {ti_py / ti_c if ti_py > ti_c else ti_c / ti_py:.1f}'
    f' times {"FASTER" if ti_py > ti_c else "SLOWER"}.'
)

print()
print('Version A with Python cache, no C cache:')
# Use pFibB
ti_py = timeit.timeit(f'pFibB.fibonacci({index})', setup='import pFibB', number=number)
print(f'Python timeit: {ti_py:8.6f}')

# Use cFibB
ti_c = timeit.timeit(f'cFibA.fibonacci({index})',
                     setup='from cPyExtPatt.SimpleExample import cFibA', number=number)
print(f'     C timeit: {ti_c:8.6f}')

print(
    f'C is {ti_py / ti_c if ti_py > ti_c else ti_c / ti_py:.1f}'
    f' times {"FASTER" if ti_py > ti_c else "SLOWER"}.'
)

print()
print('Version B, both are cached:')
# Use pFibB
ti_py = timeit.timeit(f'pFibB.fibonacci({index})', setup='import pFibB', number=number)
print(f'Python timeit: {ti_py:8.6f}')

# Use cFibB
ti_c = timeit.timeit(f'cFibB.fibonacci({index})',
                     setup='from cPyExtPatt.SimpleExample import cFibB', number=number)
print(f'     C timeit: {ti_c:8.6f}')

print(
    f'C is {ti_py / ti_c if ti_py > ti_c else ti_c / ti_py:.1f}'
    f' times {"FASTER" if ti_py > ti_c else "SLOWER"}.'
)
