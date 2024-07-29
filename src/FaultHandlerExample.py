import faulthandler
faulthandler.enable()

from cPyExtPatt import cPyRefs

a_list = ['abc' * 200]
cPyRefs.pop_and_print_BAD(a_list)
