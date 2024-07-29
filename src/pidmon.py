"""
Created on 5 May 2015

@author: paulross

A simple memory monitoring tool.
"""
import sys
import time

import psutil


def memory_monitor(process_id: int, frequency: float = 1.0) -> None:
    """Print out memory usage of a process at regular intervals."""
    proc = psutil.Process(process_id)
    print(proc.memory_info_ex())
    prev_mem = None
    while True:
        try:
            mem = proc.memory_info().rss / 1e6
            if prev_mem is None:
                print('{:10.3f} [Mb]'.format(mem))
            else:
                print('{:10.3f} [Mb] {:+10.3f} [Mb]'.format(mem, mem - prev_mem))
            prev_mem = mem
            time.sleep(frequency)
        except KeyboardInterrupt:
            try:
                input(' Pausing memMon, <cr> to continue, Ctrl-C to end...')
            except KeyboardInterrupt:
                print('\n')
                return


def main() -> int:
    if len(sys.argv) < 2:
        print('Usage: python pidmon.py <PID>')
        return -1
    pid = int(sys.argv[1])
    memory_monitor(pid)
    return 0


if __name__ == '__main__':
    exit(main())
