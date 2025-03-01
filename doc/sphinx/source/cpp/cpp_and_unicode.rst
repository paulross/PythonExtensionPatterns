.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2

.. _cpp_and_unicode:

.. index::
    pair: C++; Unicode

====================================
Python Unicode Strings and C++
====================================

This section looks at how you can bridge between Python and C++ unicode in Python extensions.

Whilst Python is Unicode aware C++ is not, well C++11 added ``std::basic_string`` specialisations for 2 and 4 byte
'Unicode' characters but these are just containers, they have no real awareness of what they contain.

------------------------------------
Basic Handling of Unicode
------------------------------------

The task here is to:

#. Take any Python Unicode string as an argument.
#. Convert it into an appropriate C++ container.
#. Dump that C++ container out to ``std::cout``.
#. Create and new Python Unicode string from that C++ container and return it.

This is just show that we can round-trip between the internal representations of the two languages.

Here is the despatch function that takes a single Unicode argument (note the ``"U"`` specification) and calls the
appropriate handling function:

.. code-block:: cpp

    /* Handler functions, defined later. */
    PyObject *unicode_1_to_string_and_back(PyObject *py_str);
    PyObject *unicode_2_to_string_and_back(PyObject *py_str);
    PyObject *unicode_4_to_string_and_back(PyObject *py_str);

    static PyObject *
    unicode_to_string_and_back(PyObject *Py_UNUSED(module), PyObject *args) {
        PyObject *py_str = NULL;
        PyObject *ret_val = NULL;
        if (! PyArg_ParseTuple(args, "U", &py_str)) {
            return NULL;
        }
        unicode_dump_as_1byte_string(py_str);
        std::cout << "Native:" << std::endl;
        switch (PyUnicode_KIND(py_str)) {
            case PyUnicode_1BYTE_KIND:
                ret_val = unicode_1_to_string_and_back(py_str);
                break;
            case PyUnicode_2BYTE_KIND:
                ret_val = unicode_2_to_string_and_back(py_str);
                break;
            case PyUnicode_4BYTE_KIND:
                ret_val = unicode_4_to_string_and_back(py_str);
                break;
            default:
                PyErr_Format(PyExc_ValueError,
                             "In %s argument is not recognised as a Unicode 1, 2, 4 byte string",
                             __FUNCTION__);
                ret_val = NULL;
                break;
        }
        return ret_val;
    }

The three handler functions are here, they use ``std::string``, ``std::u16string`` and ``std::u32string`` as appropriate:

.. code-block:: c

    static PyObject *
    unicode_1_to_string_and_back(PyObject *py_str) {
        assert(PyUnicode_KIND(py_str) == PyUnicode_1BYTE_KIND);
        std::string result = std::string((char *) PyUnicode_1BYTE_DATA(py_str));
        dump_string(result);
        return PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND,
                                         result.c_str(),
                                         result.size());
    }

    static PyObject *
    unicode_2_to_string_and_back(PyObject *py_str) {
        assert(PyUnicode_KIND(py_str) == PyUnicode_2BYTE_KIND);
        // NOTE: std::u16string is a std::basic_string<char16_t>
        std::u16string result = std::u16string((char16_t *) PyUnicode_2BYTE_DATA(py_str));
        dump_string(result);
        return PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND,
                                         result.c_str(),
                                         result.size());
    }

    static PyObject *
    unicode_4_to_string_and_back(PyObject *py_str) {
        assert(PyUnicode_KIND(py_str) == PyUnicode_4BYTE_KIND);
        // NOTE: std::u32string is a std::basic_string<char32_t>
        std::u32string result = std::u32string((char32_t *) PyUnicode_4BYTE_DATA(py_str));
        dump_string(result);
        return PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND,
                                         result.c_str(),
                                         result.size());
    }

Each of these calls ``dump_string`` which is a template function that spits out the individual character values:

.. code-block:: cpp

    template <typename T>
    void dump_string(const std::basic_string<T> &str) {
        std::cout << "String size: " << str.size();
        std::cout << " word size: " << sizeof(T) << std::endl;
        for (size_t i = 0; i < str.size(); ++i) {
            std::cout << std::setfill('0');
            std::cout << "0x" << std::hex;
            std::cout << std::setw(2 * sizeof(T)) << static_cast<int>(str[i]);
            std::cout << " " << std::dec << std::setw(8) << static_cast<int>(str[i]);
            std::cout << std::setfill(' ');
            std::cout << " \"" << str[i] << "\""<< std::endl;
        }
    }

For completeness here is the module code that creates a ``cUnicode`` module with a single ``show()`` function:

.. code-block:: c

    static PyMethodDef cUnicode_Methods[] = {
        {"show", (PyCFunction)unicode_to_string_and_back, METH_VARARGS,
            "Convert a Python unicode string to std::string and back."},
        {NULL, NULL, 0, NULL}        /* Sentinel */
    };

    static PyModuleDef cUnicodemodule = {
        PyModuleDef_HEAD_INIT,
        "cUnicode",
        "cUnicode works with unicode strings.",
        -1,
        cUnicode_Methods,
        NULL, NULL, NULL, NULL
    };

    PyMODINIT_FUNC
    PyInit_cUnicode(void)
    {
        PyObject* m;

        m = PyModule_Create(&cUnicodemodule);
        if (m == NULL)
            return NULL;
        return m;
    }


The full code is in ``src/cpy/cpp/cUnicode.cpp`` and the tests are in ``tests/unit/test_c_cpp.py``.
Here is an example of using this module:

.. code-block:: py

    >>> from cPyExtPatt.cpp import cUnicode
    >>> cUnicode.show('Hello')
    String size: 5 word size: 1
    0x00000048       72 "H"
    0x00000065      101 "e"
    0x0000006c      108 "l"
    0x0000006c      108 "l"
    0x0000006f      111 "o"
    'Hello'
    >>> s = "a\xac\u1234\u20ac\U00008000"
    >>> r = cUnicode.show(s)
    String size: 5 word size: 2
    0x00000061       97 "97"
    0x000000ac      172 "172"
    0x00001234     4660 "4660"
    0x000020ac     8364 "8364"
    0x00008000    32768 "32768"
    >>> r == s
    True
    >>> s = "a\xac\u1234\u20ac\U00018000"
    >>> r = cUnicode.show(s)
    String size: 5 word size: 4
    0x00000061       97 "97"
    0x000000ac      172 "172"
    0x00001234     4660 "4660"
    0x000020ac     8364 "8364"
    0x00018000    98304 "98304"
    >>> r == s
    True

.. index::
    single: C++; bytes
    single: C++; bytearray

-----------------------------------------------------------------------
Working with ``bytes``, ``bytearray`` and UTF-8 Unicode Arguments
-----------------------------------------------------------------------

It is fairly common to want to convert an argumennt that is ``bytes``, ``bytearray`` or UTF-8 to a ``std::string``.
This function will do just that:

.. code-block:: c

    /** Converting Python bytes and Unicode to and from std::string
     * Convert a PyObject to a std::string and return 0 if successful.
     * If py_str is Unicode than treat it as UTF-8.
     * This works with Python 2.7 and Python 3.4 onwards.
     */
    static int
    py_object_to_std_string(const PyObject *py_object, std::string &result, bool utf8_only = true) {
        result.clear();
        if (PyBytes_Check(py_object)) {
            result = std::string(PyBytes_AS_STRING(py_object));
            return 0;
        }
        if (PyByteArray_Check(py_object)) {
            result = std::string(PyByteArray_AS_STRING(py_object));
            return 0;
        }
        // Must be unicode then.
        if (!PyUnicode_Check(py_object)) {
            PyErr_Format(PyExc_ValueError,
                         "In %s \"py_str\" failed PyUnicode_Check()",
                         __FUNCTION__);
            return -1;
        }
        if (PyUnicode_READY(py_object)) {
            PyErr_Format(PyExc_ValueError,
                         "In %s \"py_str\" failed PyUnicode_READY()",
                         __FUNCTION__);
            return -2;
        }
        if (utf8_only && PyUnicode_KIND(py_object) != PyUnicode_1BYTE_KIND) {
            PyErr_Format(PyExc_ValueError,
                         "In %s \"py_str\" not utf-8",
                         __FUNCTION__);
            return -3;
        }
        result = std::string((char *) PyUnicode_1BYTE_DATA(py_object));
        return 0;
    }

And these three do the reverse:

.. code-block:: c

    static PyObject *
    std_string_to_py_bytes(const std::string &str) {
        return PyBytes_FromStringAndSize(str.c_str(), str.size());
    }

    static PyObject *
    std_string_to_py_bytearray(const std::string &str) {
        return PyByteArray_FromStringAndSize(str.c_str(), str.size());
    }

    static PyObject *
    std_string_to_py_utf8(const std::string &str) {
        // Equivelent to:
        // PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, str.c_str(), str.size());
        return PyUnicode_FromStringAndSize(str.c_str(), str.size());
    }
