'''
Created on 26 May 2015

@author: paulross
'''
import cPyRefs

s = ' ' * 1024**3
cPyRefs.incref(s)
del s
