//
// Created by Paul Ross on 08/07/2021.
//

#include "PythonFileWrapper.h"

#include <strstream>

/**
 * Macro that gets the given method and checks that it is callable.
 * If not an ExceptionPythonFileObjectWrapper is thrown.
 */
#define EXTRACT_METHOD_AND_CHECK(name)                                                          \
    m_python_##name##_method = PyObject_GetAttrString(python_file_object, #name); /* New ref. */\
    if (!m_python_##name##_method) {                                                            \
        std::ostrstream oss;                                                                    \
        oss << "PythonFileObjectWrapper: can not get method: " << #name << std::endl;           \
        Py_XDECREF(python_file_object);                                                         \
        Py_XDECREF(m_python_read_method);                                                       \
        Py_XDECREF(m_python_write_method);                                                      \
        Py_XDECREF(m_python_seek_method);                                                       \
        Py_XDECREF(m_python_tell_method);                                                       \
        throw ExceptionPythonFileObjectWrapper(oss.str());                                      \
    }                                                                                           \
    if (!PyCallable_Check(m_python_##name##_method)) {                                          \
        std::ostrstream oss;                                                                    \
        oss << "PythonFileObjectWrapper: method: " << #name << " is not callable" << std::endl; \
        Py_XDECREF(m_python_file_object);                                                       \
        Py_XDECREF(m_python_read_method);                                                       \
        Py_XDECREF(m_python_write_method);                                                      \
        Py_XDECREF(m_python_seek_method);                                                       \
        Py_XDECREF(m_python_tell_method);                                                       \
        throw ExceptionPythonFileObjectWrapper(oss.str());                                      \
    }

PythonFileObjectWrapper::PythonFileObjectWrapper(PyObject *python_file_object) : m_python_file_object(
        python_file_object),
                                                                                 m_python_read_method(NULL),
                                                                                 m_python_write_method(NULL),
                                                                                 m_python_seek_method(NULL),
                                                                                 m_python_tell_method(NULL) {
    assert(python_file_object);
    Py_INCREF(m_python_file_object);
    /* Get the read and write methods of the passed object */
    EXTRACT_METHOD_AND_CHECK(read);
    EXTRACT_METHOD_AND_CHECK(write);
    EXTRACT_METHOD_AND_CHECK(seek);
    EXTRACT_METHOD_AND_CHECK(tell);
}

int PythonFileObjectWrapper::read_py_write_cpp(Py_ssize_t number_of_bytes, std::iostream &ios) {
    assert(!PyErr_Occurred());
    assert(m_python_file_object);
    assert(m_python_read_method);
    assert(m_python_write_method);
    int ret = 0;
#if DEBUG_PYEXT_COMMON
    fprintf(stdout, "%s(): %s#%d number_of_bytes=%ld\n", __FUNCTION__, __FILE__, __LINE__, number_of_bytes);
#endif
    PyObject * read_args = Py_BuildValue("(i)", number_of_bytes);
    PyObject * read_value = PyObject_Call(m_python_read_method, read_args, NULL);
    if (read_value == NULL) {
        ret = -1;
        goto except;
    } else {
        /* Check for EOF */
        if (number_of_bytes >= 0 && PySequence_Length(read_value) != number_of_bytes) {
            ret = -2; /* Signal EOF. */
            goto except;
        }
        if (PyBytes_Check(read_value)) {
            ios.write(PyBytes_AsString(read_value), PyBytes_Size(read_value));
        } else if (PyUnicode_Check(read_value)) {
            Py_ssize_t size;
            const char *buffer = PyUnicode_AsUTF8AndSize(read_value, &size);
            ios.write(buffer, size);
        } else {
            ret = -3;
            goto except;
        }
    }
    goto finally;
    except:
    /* Handle every abnormal condition and clean up. */
    assert(ret);
    finally:
    /* Clean up under normal conditions and return an appropriate value. */
    Py_XDECREF(read_args);
    Py_XDECREF(read_value);
#if DEBUG_PYEXT_COMMON
    fprintf(stdout, "%s(): %s#%d ret=%d\n", __FUNCTION__, __FILE__, __LINE__, ret);
#endif
    return ret;
}

int PythonFileObjectWrapper::read_cpp_write_py(std::iostream &ios, Py_ssize_t number_of_bytes) {
    assert(!PyErr_Occurred());
    assert(m_python_file_object);
    assert(m_python_read_method);
    assert(m_python_write_method);
    int ret = 0;
    PyObject *py_bytes = NULL;
    PyObject *write_args = NULL;
    PyObject *write_result = NULL;
#if DEBUG_PYEXT_COMMON
    fprintf(stdout, "%s(): %s#%d number_of_bytes=%ld\n", __FUNCTION__, __FILE__, __LINE__, number_of_bytes);
#endif
    if (!ios.good()) {
        PyErr_SetString(PyExc_ValueError, "C++ stream not capable of being read.");
        goto except;
    }
    // Read from ios, write to Python file.
    // Create a Python bytes object, read into it.
    py_bytes = PyBytes_FromStringAndSize(NULL, number_of_bytes);
    ios.read(PyBytes_AsString(py_bytes), number_of_bytes);
    if (!ios.good()) {
        PyErr_SetString(PyExc_ValueError, "Can not read from C++ stream.");
        goto except;
    }
    write_args = Py_BuildValue("(O)", py_bytes);
    write_result = PyObject_Call(m_python_write_method, write_args, NULL);
    if (write_result == NULL) {
        ret = -1;
        goto except;
    }
    if (PyLong_AsLong(write_result) != number_of_bytes) {
        ret = -2;
        goto except;
    }
    goto finally;
    except:
    /* Handle every abnormal condition and clean up. */
    assert(ret);
    finally:
    /* Clean up under normal conditions and return an appropriate value. */
    Py_XDECREF(py_bytes);
    Py_XDECREF(write_args);
    Py_XDECREF(write_result);
#if DEBUG_PYEXT_COMMON
    fprintf(stdout, "%s(): %s#%d ret=%d\n", __FUNCTION__, __FILE__, __LINE__, ret);
#endif
    return ret;
}


int PythonFileObjectWrapper::read(Py_ssize_t number_of_bytes, std::vector<char> &result) {
    assert(!PyErr_Occurred());
    assert(m_python_file_object);
    assert(m_python_read_method);
    assert(m_python_write_method);
    int ret = 0;
#if DEBUG_PYEXT_COMMON
    fprintf(stdout, "%s(): %s#%d number_of_bytes=%ld\n", __FUNCTION__, __FILE__, __LINE__, number_of_bytes);
#endif
    result.clear();
    PyObject * read_args = Py_BuildValue("(i)", number_of_bytes);
    PyObject * read_value = PyObject_Call(m_python_read_method, read_args, NULL);
    if (read_value == NULL) {
        ret = -1;
        goto except;
    } else {
        /* Check for EOF */
        if (number_of_bytes >= 0 && PySequence_Length(read_value) != number_of_bytes) {
            ret = -2; /* Signal EOF. */
            goto except;
        }
        const char *buffer;
        Py_ssize_t size;
        if (PyBytes_Check(read_value)) {
            buffer = PyBytes_AsString(read_value);
            size =PyBytes_Size(read_value);
        } else if (PyUnicode_Check(read_value)) {
            buffer = PyUnicode_AsUTF8AndSize(read_value, &size);
        } else {
            ret = -3;
            goto except;
        }
        for (Py_ssize_t i = 0; i < size; ++i) {
            result.push_back(buffer[i]);
        }
    }
    goto finally;
    except:
    /* Handle every abnormal condition and clean up. */
    assert(ret);
    finally:
    /* Clean up under normal conditions and return an appropriate value. */
    Py_XDECREF(read_args);
    Py_XDECREF(read_value);
#if DEBUG_PYEXT_COMMON
    fprintf(stdout, "%s(): %s#%d ret=%d\n", __FUNCTION__, __FILE__, __LINE__, ret);
#endif
    return ret;
}

int PythonFileObjectWrapper::write(const char *buffer, Py_ssize_t number_of_bytes) {
    assert(!PyErr_Occurred());
    assert(m_python_file_object);
    assert(m_python_read_method);
    assert(m_python_write_method);
    int ret = 0;
    PyObject * py_bytes = NULL;
    PyObject * write_args = NULL;
    PyObject * write_result = NULL;
#if DEBUG_PYEXT_COMMON
    fprintf(stdout, "%s(): %s#%d number_of_bytes=%ld\n", __FUNCTION__, __FILE__, __LINE__, number_of_bytes);
#endif
    // Create a Python bytes object, read into it.
    py_bytes = PyBytes_FromStringAndSize(buffer, number_of_bytes);
    write_args = Py_BuildValue("(O)", py_bytes);
    write_result = PyObject_Call(m_python_write_method, write_args, NULL);
    if (write_result == NULL) {
        ret = -1;
        goto except;
    }
    if (PyLong_AsLong(write_result) != number_of_bytes) {
        ret = -2;
        goto except;
    }
    goto finally;
except:
    /* Handle every abnormal condition and clean up. */
    assert(ret);
finally:
    /* Clean up under normal conditions and return an appropriate value. */
    Py_XDECREF(py_bytes);
    Py_XDECREF(write_args);
    Py_XDECREF(write_result);
#if DEBUG_PYEXT_COMMON
    fprintf(stdout, "%s(): %s#%d ret=%d\n", __FUNCTION__, __FILE__, __LINE__, ret);
#endif
    return ret;
}

long PythonFileObjectWrapper::seek(Py_ssize_t pos, int whence) {
    assert(!PyErr_Occurred());
    assert(m_python_file_object);
    assert(m_python_seek_method);

    PyObject * arguments = Py_BuildValue("ni", pos, whence);
    PyObject * result = PyObject_Call(m_python_seek_method, arguments, NULL);
    return PyLong_AsLong(result);
}

long PythonFileObjectWrapper::tell() {
    assert(!PyErr_Occurred());
    assert(m_python_file_object);
    assert(m_python_tell_method);

    PyObject * result = PyObject_CallNoArgs(m_python_tell_method);
    return PyLong_AsLong(result);
}

std::string PythonFileObjectWrapper::str_pointers() {
    std::ostrstream oss;
    oss << "PythonFileObjectWrapper:" << std::endl;
    oss << "m_python_file_object  " << std::hex << m_python_file_object << " type: "
        << Py_TYPE(m_python_file_object)->tp_name << " ref count=" << std::dec << m_python_file_object->ob_refcnt
        << std::endl;
    oss << "m_python_read_method  " << std::hex << m_python_read_method << " type: "
        << Py_TYPE(m_python_read_method)->tp_name << " ref count=" << std::dec << m_python_read_method->ob_refcnt
        << std::endl;
    oss << "m_python_write_method " << std::hex << m_python_write_method << " type: "
        << Py_TYPE(m_python_write_method)->tp_name << " ref count=" << std::dec << m_python_write_method->ob_refcnt
        << std::endl;
    oss << "m_python_seek_method  " << std::hex << m_python_seek_method << " type: "
        << Py_TYPE(m_python_seek_method)->tp_name << " ref count=" << std::dec << m_python_seek_method->ob_refcnt
        << std::endl;
    oss << "m_python_tell_method  " << std::hex << m_python_tell_method << " type: "
        << Py_TYPE(m_python_tell_method)->tp_name << " ref count=" << std::dec << m_python_tell_method->ob_refcnt
        << std::endl;
    return oss.str();
}

PyObject *PythonFileObjectWrapper::py_str_pointers() {
    std::ostrstream oss;
    oss << "PythonFileObjectWrapper:" << std::endl;
    oss << "m_python_file_object  " << std::hex << m_python_file_object << " type: "
        << Py_TYPE(m_python_file_object)->tp_name << " ref count=" << std::dec << m_python_file_object->ob_refcnt
        << std::endl;
    oss << "m_python_read_method  " << std::hex << m_python_read_method << " type: "
        << Py_TYPE(m_python_read_method)->tp_name << " ref count=" << std::dec << m_python_read_method->ob_refcnt
        << std::endl;
    oss << "m_python_write_method " << std::hex << m_python_write_method << " type: "
        << Py_TYPE(m_python_write_method)->tp_name << " ref count=" << std::dec << m_python_write_method->ob_refcnt
        << std::endl;
    oss << "m_python_seek_method  " << std::hex << m_python_seek_method << " type: "
        << Py_TYPE(m_python_seek_method)->tp_name << " ref count=" << std::dec << m_python_seek_method->ob_refcnt
        << std::endl;
    oss << "m_python_tell_method  " << std::hex << m_python_tell_method << " type: "
        << Py_TYPE(m_python_tell_method)->tp_name << " ref count=" << std::dec << m_python_tell_method->ob_refcnt
        << std::endl;
    std::string str_result = oss.str();
    return PyBytes_FromStringAndSize(str_result.c_str(), str_result.size());
}

PythonFileObjectWrapper::~PythonFileObjectWrapper() {
    Py_XDECREF(m_python_read_method);
    Py_XDECREF(m_python_write_method);
    Py_XDECREF(m_python_seek_method);
    Py_XDECREF(m_python_tell_method);
    Py_XDECREF(m_python_file_object);
}
