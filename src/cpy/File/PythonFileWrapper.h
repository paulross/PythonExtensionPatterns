//
// Created by Paul Ross on 08/07/2021.
//

#ifndef PYTHONEXTENSIONSBASIC_PYTHONFILEWRAPPER_H
#define PYTHONEXTENSIONSBASIC_PYTHONFILEWRAPPER_H
#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "structmember.h"

#include <iostream>
#include <exception>
#include <string>
#include <utility>
//#include <utility>

class ExceptionPythonFileObjectWrapper : public std::exception {
public:
    explicit ExceptionPythonFileObjectWrapper(std::string in_msg) : m_msg(std::move(in_msg)) {}

    [[nodiscard]] const std::string &message() const { return m_msg; }

    [[nodiscard]] const char *what() const

    noexcept override{return m_msg.c_str();}
protected:
    std::string m_msg{};
};


/// Class that is created with a PyObject* that looks like a Python File.
/// This can then read from that file object ans write to a user provided C++ stream or read from a user provided C++
/// stream and write to the give Python file like object.
class PythonFileObjectWrapper {
public:
    explicit PythonFileObjectWrapper(PyObject *python_file_object);

    /// Read from a Python file and write to the C++ stream.
    /// Return zero on success, non-zero on failure.
    int read_py_write_cpp(Py_ssize_t number_of_bytes, std::iostream &ios);

    /// Read from a C++ stream and write to a Python file.
    /// Return zero on success, non-zero on failure.
    int read_cpp_write_py(std::iostream &ios, Py_ssize_t number_of_bytes);

    long seek(Py_ssize_t pos, int whence = 0);

    long tell();

    std::string str_pointers();

    virtual ~PythonFileObjectWrapper();

protected:
    PyObject *m_python_file_object = NULL;
    PyObject *m_python_read_method = NULL;
    PyObject *m_python_write_method = NULL;
    PyObject *m_python_seek_method = NULL;
    PyObject *m_python_tell_method = NULL;
};

#endif //PYTHONEXTENSIONSBASIC_PYTHONFILEWRAPPER_H
