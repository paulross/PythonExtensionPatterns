import faulthandler
faulthandler.enable()

import cPyRefs

l = ['abc' * 200]
cPyRefs.popBAD(l)
