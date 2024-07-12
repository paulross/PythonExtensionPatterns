//
//  cFile.c
//  PythonExtensionPatterns
//
//  Created by Paul Ross on 10/07/2024.
//  Copyright (c) 2024 Paul Ross. All rights reserved.
//

#define PY_SSIZE_T_CLEAN

#include "Python.h"
#include "PythonFileWrapper.h"
#include "time.h"

#define FPRINTF_DEBUG 0

/** Example of changing a Python string representing a file path to a C string and back again.
 *
 * The Python signature is:
 *
 * def parse_filesystem_argument(path: typing.Union[str, pathlib.Path]) -> str:
 */
static PyObject *
parse_filesystem_argument(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwargs) {
    assert(!PyErr_Occurred());
    assert(args || kwargs);

    PyBytesObject *py_path = NULL;
    char *c_path = NULL;
    Py_ssize_t path_size;
    PyObject *ret = NULL;

    /* Parse arguments */
    static const char *kwlist[] = {"path", NULL};
    /* Can be optional output path with "|O&". */
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&", const_cast<char **>(kwlist), PyUnicode_FSConverter,
                                     &py_path)) {
        goto except;
    }
    /* Check arguments. */
    assert(py_path);
    /* Grab a reference to the internal bytes buffer. */
    if (PyBytes_AsStringAndSize((PyObject *) py_path, &c_path, &path_size)) {
        /* Should have a TypeError or ValueError. */
        assert(PyErr_Occurred());
        assert(PyErr_ExceptionMatches(PyExc_TypeError)
               || PyErr_ExceptionMatches(PyExc_ValueError));
        goto except;
    }
    assert(c_path);
    /* Use the C path. */

    /* Now convert the C path to a Python object, a string. */
    ret = PyUnicode_DecodeFSDefaultAndSize(c_path, path_size);
    if (!ret) {
        goto except;
    }
    assert(!PyErr_Occurred());
    goto finally;
    except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    ret = NULL;
    finally:
    // Assert all temporary locals are NULL and thus have been transferred if used.
    Py_XDECREF(py_path);
    return ret;
}


/**
 * Take a Python file object and and an integer and read that number of bytes and access this data in C.
 * This returns the bytes read as a bytes object.
 *
 * Python signature:
 *
 * def read_python_file_to_c(file_object: typing.IO, size: int = -1) -> bytes:
 */
static PyObject *
read_python_file_to_c(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    assert(!PyErr_Occurred());
    static const char *kwlist[] = {"file_object", "size", NULL};
    PyObject *py_file_object = NULL;
    Py_ssize_t bytes_to_read = -1;
    PyObject *py_read_meth = NULL;
    PyObject *py_read_args = NULL;
    PyObject *py_read_data = NULL;
    char *c_bytes_data = NULL;
    PyObject *ret = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|n", (char **) (kwlist),
                                     &py_file_object, &bytes_to_read)) {
        return NULL;
    }
#if FPRINTF_DEBUG
    fprintf(stdout, "Got a file object of type \"%s\" and bytes to read of %ld\n", Py_TYPE(py_file_object)->tp_name,
            bytes_to_read);
#endif
    // Check that this is a readable file, well does it have a read method?
    /* Get the read method of the passed object */
    py_read_meth = PyObject_GetAttrString(py_file_object, "read"); // New reference
    if (py_read_meth == NULL) {
        PyErr_Format(PyExc_ValueError,
                     "Argument of type %s does not have a read() method.",
                     Py_TYPE(py_file_object)->tp_name);
        goto except;
    }
#if FPRINTF_DEBUG
    fprintf(stdout, "Have read attribute of type \"%s\"\n", Py_TYPE(py_read_meth)->tp_name);
#endif
    if (!PyCallable_Check(py_read_meth)) {
        PyErr_Format(PyExc_ValueError,
                     "read attribute of type %s is not callable.",
                     Py_TYPE(py_file_object)->tp_name);
        goto except;
    }
#if FPRINTF_DEBUG
    fprintf(stdout, "Read attribute is callable.\n");
#endif
    // Call read(VisibleRecord::NUMBER_OF_HEADER_BYTES) to get a Python bytes object.
    py_read_args = Py_BuildValue("(i)", bytes_to_read);
    if (!py_read_args) {
        goto except;
    }
    // This should advance that readable file pointer.
    py_read_data = PyObject_Call(py_read_meth, py_read_args, NULL);
    if (py_read_data == NULL) {
        goto except;
    }
#if FPRINTF_DEBUG
    fprintf(stdout, "read_data is type \"%s\"\n", Py_TYPE(py_read_data)->tp_name);
#endif
    /* Check for EOF */
    if (bytes_to_read >= 0 && PySequence_Length(py_read_data) != bytes_to_read) {
        assert(PyErr_Occurred());
        PyErr_Format(PyExc_IOError,
                     "Reading file object gives EOF. Requested bytes %ld, got %ld.",
                     bytes_to_read, PySequence_Length(py_read_data));
        goto except;
    }
#if FPRINTF_DEBUG
    fprintf(stdout, "read_data is length is: %ld\n", PySequence_Length(py_read_data));
#endif
    c_bytes_data = PyBytes_AsString(py_read_data);
    if (c_bytes_data == NULL) {
        // TypeError already set.
        goto except;
    }
#if FPRINTF_DEBUG
    fprintf(stdout, "Data is \"%s\"\n", c_bytes_data);
#endif
    ret = py_read_data;
    goto finally;
except:
    /* Handle every abnormal condition and clean up. */
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    /* Clean up under normal conditions and return an appropriate value. */
    Py_XDECREF(py_read_meth);
    Py_XDECREF(py_read_args);
    return ret;
}


/**
 * Take a Python bytes object, extract the bytes as a C char* and write to the python file object.
 * This returns the number of bytes written.
 *
 * Python signature:
 *
 * def write_bytes_to_python_file(bytes_to_write: bytes, file_object: typing.IO) -> int:
 */
static PyObject *
write_bytes_to_python_file(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    assert(!PyErr_Occurred());
    static const char *kwlist[] = {"bytes_to_write", "file_object", NULL};
    PyObject *py_file_object = NULL;
    Py_buffer c_buffer;
    PyObject *ret = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "y*O", (char **) (kwlist),
                                     &c_buffer, &py_file_object)) {
        return NULL;
    }
#if FPRINTF_DEBUG
    fprintf(stdout, "Calling PyFile_WriteString() with bytes \"%s\"\n", (char *)c_buffer.buf);
#endif
    /* NOTE: PyFile_WriteString() creates a unicode string and then calls PyFile_WriteObject()
     * so the py_file_object must be capable of writing strings. */
    int result = PyFile_WriteString((char *)c_buffer.buf, py_file_object);
    if (result != 0) {
        goto except;
    }
    ret = Py_BuildValue("n", c_buffer.len);
    goto finally;
except:
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    return ret;
}

/**
 * Wraps a Python file object.
 */
static PyObject *
wrap_python_file(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
    assert(!PyErr_Occurred());
    static const char *kwlist[] = {"file_object", NULL};
    PyObject *py_file_object = NULL;
//    PyObject *ret = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **) (kwlist),
                                     &py_file_object)) {
        return NULL;
    }
    PythonFileObjectWrapper py_file_wrapper(py_file_object);

    /* Exercise ths wrapper by writing, reading etc. */
    py_file_wrapper.write("Test write to python file", 25);

//    std::string str_pointers = py_file_wrapper.str_pointers();
//    return PyBytes_FromStringAndSize(str_pointers.c_str(), str_pointers.size());
    return py_file_wrapper.py_str_pointers();
}

#if 0
/**
 * Returns an integer file descriptor from a Python file object.
 */
int python_file_object_as_file_description(PyObject *op) {
    int fd = PyObject_AsFileDescriptor(op);
    if (fd < 0) {
        return -1;
    }
    return fd;
}

/** fd is an already open file. */
PyObject *c_file_descriptor_as_python_file(int fd, const char *filename) {
    PyObject *op = PyFile_FromFd(fd, filename, "r", -1, NULL, NULL, NULL, 1);
    return op;
}

/*
 * fileno() man page:
 * https://www.man7.org/linux/man-pages/man3/fileno.3.html
 */
PyObject *c_file_path_as_python_file(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    int fd = fileno(file);
    PyObject *op = PyFile_FromFd(fd, filename, "r", -1, NULL, NULL, NULL, 1);
    return op;
}
#endif

static PyMethodDef cFile_methods[] = {
        {
                "parse_filesystem_argument",
                (PyCFunction) parse_filesystem_argument,
                METH_VARARGS | METH_KEYWORDS,
                "Parsing an argument that is a file path."
        },
        {
                "read_python_file_to_c",
                (PyCFunction) read_python_file_to_c,
                METH_VARARGS | METH_KEYWORDS,
                "Read n bytes from a Python file."
        },
        {
                "write_bytes_to_python_file",
                (PyCFunction) write_bytes_to_python_file,
                METH_VARARGS | METH_KEYWORDS,
                "Wrote bytes to a Python file."
        },
        {
                "wrap_python_file",
                (PyCFunction) wrap_python_file,
                METH_VARARGS | METH_KEYWORDS,
                "Wrap a Python file."
        },
        {
                NULL,
                NULL,
                0,
                NULL
        } /* Sentinel */
};

static PyModuleDef cFile_module = {
        PyModuleDef_HEAD_INIT,
        "cFile",
        "Examples of handling file paths and files in a Python 'C' extension.",
        -1,
        cFile_methods,
        NULL, /* inquiry m_reload */
        NULL, /* traverseproc m_traverse */
        NULL, /* inquiry m_clear */
        NULL, /* freefunc m_free */
};

PyMODINIT_FUNC PyInit_cFile(void) {
    return PyModule_Create(&cFile_module);
}
/****************** END: Parsing arguments. ****************/
