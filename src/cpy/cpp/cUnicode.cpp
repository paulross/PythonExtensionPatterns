//
//  cUnicode.cpp
//  PyCppUnicode
//
//  Created by Paul Ross on 16/04/2018.
//  Copyright (c) 2018-2024 Paul Ross. All rights reserved.
//

#include <Python.h>

#include <assert.h>
#include <iomanip>
#include <iostream>
#include <string>

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

static PyObject *
py_object_to_string_and_back(PyObject *Py_UNUSED(module), PyObject *args) {
    PyObject *py_object = NULL;

    if (!PyArg_ParseTuple(args, "O", &py_object)) {
        return NULL;
    }
    std::string str_result;
    int err_code = py_object_to_std_string(py_object, str_result, false);
    if (err_code) {
        PyErr_Format(PyExc_ValueError,
                     "In %s \"py_object_to_std_string\" failed with error code %d",
                     __FUNCTION__,
                     err_code
        );
        return NULL;
    }
    if (PyBytes_Check(py_object)) {
        return std_string_to_py_bytes(str_result);
    }
    if (PyByteArray_Check(py_object)) {
        return std_string_to_py_bytearray(str_result);
    }
    if (PyUnicode_Check(py_object)) {
        return std_string_to_py_utf8(str_result);
    }
    PyErr_Format(PyExc_ValueError,
                 "In %s does not support python type %s",
                 __FUNCTION__,
                 Py_TYPE(py_object)->tp_name
    );
    return NULL;
}

template<typename T>
static void dump_string(const std::basic_string<T> &str) {
    std::cout << "String size: " << str.size();
    std::cout << " word size: " << sizeof(T) << std::endl;
    for (size_t i = 0; i < str.size(); ++i) {
        std::cout << "0x" << std::hex << std::setfill('0');
        std::cout << std::setw(8) << static_cast<int>(str[i]);
        std::cout << std::setfill(' ');
        std::cout << " " << std::dec << std::setw(8) << static_cast<int>(str[i]);
        std::cout << " \"" << str[i] << "\"" << std::endl;
    }
}

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

static void
unicode_dump_as_1byte_string(PyObject *py_str) {
    Py_ssize_t len = PyUnicode_GET_LENGTH(py_str) * PyUnicode_KIND(py_str);
    std::string result = std::string((char *) PyUnicode_1BYTE_DATA(py_str), len);
    std::cout << "unicode_dump_as_1byte_string();" << std::endl;
    dump_string(result);
}

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

static PyMethodDef cUnicode_Methods[] = {
        {
                "unicode_to_string_and_back",
                (PyCFunction) unicode_to_string_and_back,
                     METH_VARARGS,
                "Convert a Python unicode string to std::string and back."
        },
        {
                "py_object_to_string_and_back",
                (PyCFunction) py_object_to_string_and_back,
                     METH_VARARGS,
                "Convert a Python unicode string, bytes, bytearray to std::string and back."
        },
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
PyInit_cUnicode(void) {
    PyObject *m;

    m = PyModule_Create(&cUnicodemodule);
    if (m == NULL)
        return NULL;
    return m;
}
