.. highlight:: python
    :linenothreshold: 10

.. highlight:: c
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _gcov-label:

===============================================
Using gcov for C/C++ Code Coverage
===============================================

This is about how to run your C/C++ code using gcov to gather coverage metrics.

.. note::

    These instructions have been tested on Mac OS X 10.9 (Mavericks).
    They may or may not work on other OS's


.. _gcov-mac-osx-label:

---------------------------
gcov under Mac OS X
---------------------------


^^^^^^^^^^^^^^^^^
Configuring Xcode
^^^^^^^^^^^^^^^^^

`This document <https://developer.apple.com/library/mac/qa/qa1514/_index.html>`_ tells you how to configure Xcode for gcov (actually llvm-cov).

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Running and Analysing Code Coverage
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In Xcode select Product->Run then once that has completed note the GCov_build directory in Xcode by selecting Products-><project> in the left hand pane. In the right hand pane under 'Identity and Type' you should see 'Full Path' of something like::

    /Users/$USER/Library/Developer/Xcode/DerivedData/<project name and hash>/Build/Products/GCov_Build/<project>

In the Terminal navigate to the 'Build' directory:

.. code-block:: bash

    $ cd /Users/$USER/Library/Developer/Xcode/DerivedData/<project name and hash>/Build/

Now navigate to the Intermediates/ directory and in there you should find the code coverage data in a path such as this:

.. code-block:: bash

    cd Intermediates/<project>.build/GCov_Build/<project>.build/Objects-normal/x86_64

In there the interesting file has a .gcno extension. To convert this into something readable we need to run gcov on it. We can use xcrun to find gcov and this gives an overall code coverage number for each file (and its includes):

.. code-block:: bash

    $ xcrun gcov TimeBDT.gcno
    ...
    File 'TimeBDT.cpp'
    Lines executed:87.18% of 78
    TimeBDT.cpp:creating 'TimeBDT.cpp.gcov'
    ...

This has now generated a detailed file TimeBDT.cpp.gcov that contains line by line coverage:

.. code-block:: bash

    $ ls -l TimeBDT.*
    -rw-r--r--  1 paulross  staff   14516  6 Oct 18:48 TimeBDT.cpp.gcov
    -rw-r--r--  1 paulross  staff     248  6 Oct 18:38 TimeBDT.d
    -rw-r--r--  1 paulross  staff    1120  6 Oct 18:38 TimeBDT.dia
    -rw-r--r--  1 paulross  staff   32252  6 Oct 18:45 TimeBDT.gcda
    -rw-r--r--  1 paulross  staff  121444  6 Oct 18:38 TimeBDT.gcno
    -rw-r--r--  1 paulross  staff    3671  6 Oct 18:48 TimeBDT.h.gcov
    -rw-r--r--  1 paulross  staff  310496  6 Oct 18:38 TimeBDT.o
    $ cat TimeBDT.cpp.gcov | less
    ...
        -:  208:/* Returns the total number of seconds in this particular year. */
        -:  209:time_t TimeBDTMapEntry::secsInThisYear() const {
  6000000:  210:    return _isLeap ? SECONDS_IN_YEAR[1] : SECONDS_IN_YEAR[0];
        -:  211:}
    ...
        -:  289:        } else {
    #####:  290:            if (pTimeBDT->ttYearStart() > tt) {
    #####:  291:                --year;
    #####:  292:            } else {
    #####:  293:                ++year;
        -:  294:            }
        -:  295:        }

The first snippet shows that line 210 is executed 6000000 times. In the second snippet the ``#####`` shows that lines 290-293 have not executed at all.    
    
.. _gcov-cpython-code-label:

----------------------------------------------------------
Using gcov on CPython Extensions
----------------------------------------------------------

Whilst it is common to track code coverage in Python test code it gets a bit more tricky with Cpython extensions as Python code coverage tools can not track C/C++ extension code. The solution is to use ``gcov`` and run the tests in a C/C++ process by embedding the Python interpreter.

.. code-block:: c++

    #include <Python.h>

    int test_cpython_module_foo_functions(PyObject *pModule) {
        assert(pModule);
        int fail = 0;
    try:
        /* foo.function(...) */
        pFunc = PyObject_GetAttrString(pModule, "function");
        if (pFunc && PyCallable_Check(pFunc)) {
        
            pValue = PyObject_CallObject(pFunc, pArgs);
        }
        /*
         * Test other Foo functions here
         */
    except:
        assert(fail != 0);
    finally:
        return fail;
    }

    int test_cpython_module_foo() {
        int fail = 0;
        PyObject *pName = NULL;
    try:
        pName = PyUnicode_FromString("foo");
        if (! pName) {
            fail = 1;
            goto except;
        }
        pModule = PyImport_Import(pName);
        if (! pModule) {
            fail = 2;
            goto except;
        }
        /* foo.function(...) */
        pFunc = PyObject_GetAttrString(pModule, "function");
        if (pFunc && PyCallable_Check(pFunc)) {
        
            pValue = PyObject_CallObject(pFunc, pArgs);
        }
        /*
         * Test other Foo functions here
         */
    except:
        assert(fail != 0);
    finally:
        Py_XDECREF(pName);
        Py_XDECREF(pModule);
        return fail;
    }

    void test_cpython_code() {
        Py_SetProgramName("TestCPythonExtensions");  /* optional, recommended */
        Py_Initialize();
        test_cpython_module_foo();
        /*
         * Test other modules here
         */
        Py_Finalize();
    }

    int main(int argc, const char * argv[]) {
        std::cout << "Testing starting" << std::endl;
        test_cpython_code();
        /*
         * Non-CPython code tests here...
         */
        std::cout << "Testing DONE" << std::endl;
        return 0;
    }
