def fibonacci(index: int) -> int:
    if index < 2:
        return index
    return fibonacci(index - 2) + fibonacci(index - 1)
