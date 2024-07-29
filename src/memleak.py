"""
Created on 26 May 2015

@author: paulross

Deliberately create a 1GB memory leak.
"""
from cPyExtPatt import cPyRefs

s = ' ' * 1024**3
cPyRefs.incref(s)
del s
