.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 2


**********************
File Paths and Files
**********************

This chapter describes reading and writing files from C extensions.

====================================
File Paths
====================================

There are several builtin functions that allow conversion between Python and C described in the
`File System Encoding <https://docs.python.org/3/c-api/unicode.html#file-system-encoding>`_
API which uses the
`filesystem encoding and error handler <https://docs.python.org/3/glossary.html#term-filesystem-encoding-and-error-handler>`_,
see also `File Objects <https://docs.python.org/3/c-api/file.html>`_

In summary:

- ``PyUnicode_FSConverter`` Converts a Python a ``str`` or *path-like* object to a Python ``bytes`` object.
- ``PyUnicode_FSDecoder`` Converts a Python ``bytes`` object to a Python ``str``.
- ``PyUnicode_DecodeFSDefaultAndSize`` Takes a C string and length and returns a Python ``str``.
- ``PyUnicode_DecodeFSDefault`` Takes a null terminated C string and length and returns a Python ``str``.
- ``PyUnicode_EncodeFSDefault`` Takes a Python ``str`` and return a Python ``bytes`` object.

The example code is in ``src/cpy/cFile.cpp``, ``src/cpy/PythonFileWrapper.h`` and
``src/cpy/PythonFileWrapper.cpp`` and the tests are in ``tests/unit/test_c_file.py``.

----------------------------------------
Parsing File Paths as Arguments
----------------------------------------

The Python API provides functionality for converting Python file paths (a ``str`` or *path-like* object)
to C file paths (``char *``).
From Python to C;
`PyUnicode_FSConverter <https://docs.python.org/3/c-api/unicode.html#c.PyUnicode_FSConverter>`_
and the reverse from C to Python
`PyUnicode_DecodeFSDefaultAndSize <https://docs.python.org/3/c-api/unicode.html#c.PyUnicode_DecodeFSDefaultAndSize>`_

Here is an example of taking a Python Unicode string representing a file path, converting it to C and then back
to Python. The stages are:

- Use ``PyArg_ParseTupleAndKeywords`` and ``PyUnicode_FSConverter`` to convert the path-like Python object to
  a Python ``bytes`` object. Note the use of the ``"O&"`` formatting string that takes a Python object and a
  conversion function.
- Extract the raws bytes to use as a C path.
- Take a C path and convert it to a Python Unicode ``str`` and return it.

The Python signature is::

    def parse_filesystem_argument(path: typing.Union[str, pathlib.Path]) -> str:

Here is the C code:

.. code-block:: c

    static PyObject *
    parse_filesystem_argument(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwargs) {
        assert(!PyErr_Occurred());
        assert(args || kwargs);

        PyBytesObject *py_path = NULL;
        char *c_path = NULL;
        Py_ssize_t path_size;
        PyObject *ret = NULL;

        /* Parse arguments */
        static char *kwlist[] = {"path", NULL};
        /* Can be optional output path with "|O&". */
        if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&", kwlist, PyUnicode_FSConverter,
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
        // Decref temporary locals.
        Py_XDECREF(py_path);
        return ret;
    }

=============================
Files
=============================

This section describes how to interoperate between Python files, C ``FILE*`` and C++ ``iostream`` objects.


----------------------------
Reading a Python File
----------------------------

Here is an example of reading from a Python file in C.
The Python signature is::

    def read_python_file_to_c(file_object: typing.IO, size: int = -1) -> bytes:

The technique is to get the ``read()`` method from the file object with ``PyObject_GetAttrString`` then call it with the
appropriate arguments using ``PyObject_Call``.
Here is the C code:

.. code-block:: c

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
        // Check that this is a readable file, well does it have a read method?
        /* Get the read method of the passed object */
        py_read_meth = PyObject_GetAttrString(py_file_object, "read"); // New reference
        if (py_read_meth == NULL) {
            PyErr_Format(PyExc_ValueError,
                         "Argument of type %s does not have a read() method.",
                         Py_TYPE(py_file_object)->tp_name);
            goto except;
        }
        if (!PyCallable_Check(py_read_meth)) {
            PyErr_Format(PyExc_ValueError,
                         "read attribute of type %s is not callable.",
                         Py_TYPE(py_file_object)->tp_name);
            goto except;
        }
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
        /* Check for EOF */
        if (bytes_to_read >= 0 && PySequence_Length(py_read_data) != bytes_to_read) {
            assert(PyErr_Occurred());
            PyErr_Format(PyExc_IOError,
                         "Reading file object gives EOF. Requested bytes %ld, got %ld.",
                         bytes_to_read, PySequence_Length(py_read_data));
            goto except;
        }
        c_bytes_data = PyBytes_AsString(py_read_data);
        if (c_bytes_data == NULL) {
            // TypeError already set.
            goto except;
        }
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

----------------------------
Writing to a Python File
----------------------------

A similar technique can be used to write to a file, however there are a couple of C functions for writing directly to a
Python file:

``PyFile_WriteObject()``
----------------------------------

This writes a Python object to a Python file using the objects ``__str__`` method
(if `Py_PRINT_RAW <https://docs.python.org/3/c-api/object.html#c.Py_PRINT_RAW>`_ is given as the flags argument or
the objects ``__repr__`` method if flags is zero.


``PyFile_WriteString()``
----------------------------------

This will write a C ``char *`` to a Python file.

.. note::

    ``PyFile_WriteString()`` creates a unicode string and then calls ``PyFile_WriteObject()``
    so the Python file object must be capable of writing strings.

Here is an example of taking a Python bytes object, extracting the ``char *`` C buffer and writing that to a Python
file.
The Python function signature is::

    def write_bytes_to_python_file(bytes_to_write: bytes, file_object: typing.IO) -> int:

Here is the C code:

.. code-block:: c

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
        /* NOTE: PyFile_WriteString() creates a unicode string and then
         * calls PyFile_WriteObject() so the py_file_object must be
         * capable of writing strings. */
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

A C++ Python File Wrapper
----------------------------------

In ``src/cpy/PythonFileWrapper.h`` and ``src/cpy/PythonFileWrapper.cpp`` there is a C++ class that takes a Python file
and extracts the ``read()``, ``write()``, ``seek()`` and ``tell()`` methods that can then be used to read and write to
the Python file from C++. Some example code is in ``src/cpy/cFile.cpp`` and some tests are in
``tests/unit/test_c_file.py``.

Here is the class:

.. code-block:: c++

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


