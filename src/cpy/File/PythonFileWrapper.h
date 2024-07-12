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

    /// Read a number of bytes from a Python file and load them into the result.
    /// Return zero on success, non-zero on failure.
    int read(Py_ssize_t number_of_bytes, std::vector<char> &result);

    /// Write a number of bytes to a Python file.
    /// Return zero on success, non-zero on failure.
    int write(const char *buffer, Py_ssize_t number_of_bytes);

    /// Move the file pointer to the given position.
    /// whence is:
    /// 0 – start of the stream (the default); offset should be zero or positive.
    /// 1 – current stream position; offset may be negative.
    /// 2 – end of the stream; offset is usually negative.
    /// Returns the new absolute position.
    long seek(Py_ssize_t pos, int whence = 0);

    /// Returns the current absolute position.
    long tell();
    /// Returns a multi-line string that describes the class state.
    std::string str_pointers();
    /// Returns a Python multi-line bytes object that describes the class state.
    PyObject *py_str_pointers();
    /// Destructor, this decrements the held references.
    virtual ~PythonFileObjectWrapper();

protected:
    PyObject *m_python_file_object = NULL;
    PyObject *m_python_read_method = NULL;
    PyObject *m_python_write_method = NULL;
    PyObject *m_python_seek_method = NULL;
    PyObject *m_python_tell_method = NULL;
};

#endif //PYTHONEXTENSIONSBASIC_PYTHONFILEWRAPPER_H
