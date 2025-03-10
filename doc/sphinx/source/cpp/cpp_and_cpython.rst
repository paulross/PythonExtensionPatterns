.. highlight:: cpp
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _cpp_and_cpython:

.. index::
    single: C++; PyObject* Wrappers
    single: C++; References
    single: C++; Reference Counts

============================================
C++ RAII Wrappers Around ``PyObject*``
============================================

It is sometimes useful to wrap up a ``PyObject*`` in a class that will manage the reference count.
Here is a base class that shows the general idea, it takes a ``PyObject *`` and provides:

* Construction with a ``PyObject *`` and access this with ``operator PyObject*() const``.
* ``PyObject **operator&()`` to reset the underlying pointer, for example when using it with
  ``PyArg_ParseTupleAndKeywords``.
* Decrementing the reference count on destruction (potentially freeing the object).

.. code-block:: cpp

    /** General wrapper around a PyObject*.
     * This decrements the reference count on destruction.
     */
    class DecRefDtor {
    public:
        DecRefDtor(PyObject *ref) : m_ref { ref } {}
        Py_ssize_t ref_count() const { return m_ref ? Py_REFCNT(m_ref) : 0; }
        // Allow setting of the (optional) argument with PyArg_ParseTupleAndKeywords
        PyObject **operator&() {
            Py_XDECREF(m_ref);
            m_ref = NULL;
            return &m_ref;
        }
        // Access the argument
        operator PyObject*() const { return m_ref; }
        // Test if constructed successfully from the new reference.
        explicit operator bool() { return m_ref != NULL; }
        ~DecRefDtor() { Py_XDECREF(m_ref); }
    protected:
        PyObject *m_ref;
    };

.. index::
    single: C++; Borrowed PyObject* Wrappers
    single: C++; Borrowed References

-------------------------------------------------
C++ RAII Wrapper for a Borrowed ``PyObject*``
-------------------------------------------------

There are two useful sub-classes, one for borrowed references, one for new references which are intended to be temporary. Using borrowed references: 

.. code-block:: cpp

    /** Wrapper around a PyObject* that is a borrowed reference.
     * This increments the reference count on construction and
     * decrements the reference count on destruction.
     */
    class BorrowedRef : public DecRefDtor {
    public:
        BorrowedRef(PyObject *borrowed_ref) : DecRefDtor(borrowed_ref) {
            Py_XINCREF(m_ref);
        }
    };

This can be used with borrowed references as follows:

.. code-block:: cpp

    void function(PyObject *obj) {
        BorrowedRef(obj); // Increment reference here.
        // ...
    } // Decrement reference here.


.. index::
    single: C++; New PyObject* Wrappers
    single: C++; New References

-------------------------------------------------
C++ RAII Wrapper for a New ``PyObject*``
-------------------------------------------------

Here is a sub-class that wraps a new reference to a ``PyObject *`` and ensures it is free'd when the wrapper goes out of scope:

.. code-block:: cpp

    /** Wrapper around a PyObject* that is a new reference.
     * This owns the reference so does not increment it on construction but
     * does decrement it on destruction.
     */
    class NewRef : public DecRefDtor {
    public:
        NewRef(PyObject *new_ref) : DecRefDtor(new_ref) {}
    };

This new reference wrapper can be used as follows:

.. code-block:: cpp

    void function() {
        NewRef(PyLongFromLong(9)); // New reference here.
        // Use static_cast<PyObject*>(NewRef) ...
    } // Decrement the new reference here.


.. _cpp_and_cpython.handling_default_arguments:

.. index::
    single: Parsing Arguments Example; Default Mutable Arguments
    single: Default Mutable Arguments; C++

============================================
Handling Default Arguments
============================================

Handling default, possibly mutable, arguments in a pythonic way is described here:
:ref:`cpython_default_mutable_arguments`.
It is quite complicated to get it right but C++ can ease the pain with a generic class to simplify handling default
arguments in CPython functions.

The actual code is in ``src/cpy/ParseArgs/cParseArgsHelper.cpp`` but here it is, simplified to its essentials:

.. code-block:: cpp

    class DefaultArg {
    public:
        DefaultArg(PyObject *new_ref) : m_arg(NULL), m_default(new_ref) {}
        /// Allow setting of the (optional) argument with
        /// PyArg_ParseTupleAndKeywords
        PyObject **operator&() {
            m_arg = NULL;
            return &m_arg;
        }
        /// Access the argument or the default if default.
        operator PyObject *() const {
            return m_arg ? m_arg : m_default;
        }
        PyObject *obj() const {
            return m_arg ? m_arg : m_default;
        }
        /// Test if constructed successfully from the new reference.
        explicit operator bool() { return m_default != NULL; }
    protected:
        PyObject *m_arg;
        PyObject *m_default;
    };

---------------------------
Immutable Default Arguments
---------------------------

Suppose we have the Python function equivalent to the Python function:

.. code-block:: python

    def parse_defaults_with_helper_class(
        encoding_m: str = "utf-8",
        the_id_m: int = 1024,
        log_interval_m: float = 8.0):
        return encoding_m, the_id_m, log_interval_m

Here it is in C:

.. code-block:: cpp

    static PyObject *
    parse_defaults_with_helper_class(PyObject *Py_UNUSED(module), PyObject *args, PyObject *kwds) {
        PyObject *ret = NULL;
        /* Initialise default arguments. */
        static DefaultArg encoding_c(PyUnicode_FromString("utf-8"));
        static DefaultArg the_id_c(PyLong_FromLong(DEFAULT_ID));
        static DefaultArg log_interval_c(PyFloat_FromDouble(DEFAULT_FLOAT));

        /* Check that the defaults are non-NULL i.e. succesful. */
        if (!encoding_c || !the_id_c || !log_interval_c) {
            return NULL;
        }

        static const char *kwlist[] = {"encoding", "the_id", "log_interval", NULL};
        /* &encoding etc. accesses &m_arg in DefaultArg because of PyObject **operator&() */
        if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OOO",
                                         const_cast<char **>(kwlist),
                                         &encoding_c, &the_id_c, &log_interval_c)) {
            return NULL;
        }

        PY_DEFAULT_CHECK(encoding_c, PyUnicode_Check, "str");
        PY_DEFAULT_CHECK(the_id_c, PyLong_Check, "int");
        PY_DEFAULT_CHECK(log_interval_c, PyFloat_Check, "float");

        /*
         * Use encoding, the_id, must_log from here on as PyObject* since we have
         * operator PyObject*() const ...
         *
         * So if we have a function:
         * set_encoding(PyObject *obj) { ... }
         */
        // set_encoding(encoding);
        /* ... */

        /* Py_BuildValue("O") increments the reference count. */
        ret = Py_BuildValue("OOO", encoding_c.obj(), the_id_c.obj(), log_interval_c.obj());
        return ret;
    }

The full code is in ``src/cpy/cParseArgsHelper.cpp`` and the tests in ``tests/unit/test_c_parse_args_helper.py``.

Here is an example test:

.. code-block:: python

    @pytest.mark.parametrize(
        'args, expected',
        (
                (
                        (),
                        ('utf-8', 1024, 8.0),
                ),
                (
                        ('Encoding', 4219, 16.0),
                        ('Encoding', 4219, 16.0),
                ),
        ),
    )
    def test_parse_defaults_with_helper_class(args, expected):
        assert cParseArgsHelper.parse_defaults_with_helper_class(*args) == expected

-------------------------
Mutable Default Arguments
-------------------------

The same class can be used for mutable arguments.
The following emulates this Python function:

.. code-block:: python

    def parse_mutable_defaults_with_helper_class(obj, default_list=[]):
        default_list.append(obj)
        return default_list

Here it is in C:

.. code-block:: c

    /** Parse the args where we are simulating mutable default of an empty list.
     * This uses the helper class.
     *
     * This is equivalent to:
     *
     *  def parse_mutable_defaults_with_helper_class(obj, default_list=[]):
     *      default_list.append(obj)
     *      return default_list
     *
     * This adds the object to the list and returns None.
     *
     * This imitates the Python way of handling defaults.
     */
    static PyObject *parse_mutable_defaults_with_helper_class(PyObject *Py_UNUSED(module),
                                                              PyObject *args) {
        PyObject *ret = NULL;
        /* Pointers to the non-default argument, initialised by PyArg_ParseTuple below. */
        PyObject *arg_0 = NULL;
        static DefaultArg list_argument_c(PyList_New(0));

        if (!PyArg_ParseTuple(args, "O|O", &arg_0, &list_argument_c)) {
            goto except;
        }
        PY_DEFAULT_CHECK(list_argument_c, PyList_Check, "list");

        /* Your code here...*/

        /* Append the first argument to the second.
         * PyList_Append() increments the refcount of arg_0. */
        if (PyList_Append(list_argument_c, arg_0)) {
            PyErr_SetString(PyExc_RuntimeError, "Can not append to list!");
            goto except;
        }

        /* Success. */
        assert(!PyErr_Occurred());
        /* This increments the default or the given argument. */
        Py_INCREF(list_argument_c);
        ret = list_argument_c;
        goto finally;
    except:
        assert(PyErr_Occurred());
        Py_XDECREF(ret);
        ret = NULL;
    finally:
        return ret;
    }

The code is in ``src/cpy/ParseArgs/cParseArgsHelper.cpp``.

Here are some tests from ``tests/unit/test_c_parse_args_helper.py``.
Firstly establish the known Python behaviour:

.. code-block:: python

    def test_parse_mutable_defaults_with_helper_class_python():
        """A local Python equivalent of cParseArgsHelper.parse_mutable_defaults_with_helper_class()."""

        def parse_mutable_defaults_with_helper_class(obj, default_list=[]):
            default_list.append(obj)
            return default_list

        result = parse_mutable_defaults_with_helper_class(1)
        assert sys.getrefcount(result) == 3
        assert result == [1, ]
        result = parse_mutable_defaults_with_helper_class(2)
        assert sys.getrefcount(result) == 3
        assert result == [1, 2]
        result = parse_mutable_defaults_with_helper_class(3)
        assert sys.getrefcount(result) == 3
        assert result == [1, 2, 3]

        local_list = []
        assert sys.getrefcount(local_list) == 2
        assert parse_mutable_defaults_with_helper_class(10, local_list) == [10]
        assert sys.getrefcount(local_list) == 2
        assert parse_mutable_defaults_with_helper_class(11, local_list) == [10, 11]
        assert sys.getrefcount(local_list) == 2

        result = parse_mutable_defaults_with_helper_class(4)
        assert result == [1, 2, 3, 4]
        assert sys.getrefcount(result) == 3

And now the equivalent in C:

.. code-block:: python

    from cPyExtPatt import cParseArgsHelper

    def test_parse_mutable_defaults_with_helper_class_c():
        result = cParseArgsHelper.parse_mutable_defaults_with_helper_class(1)
        assert sys.getrefcount(result) == 3
        assert result == [1, ]
        result = cParseArgsHelper.parse_mutable_defaults_with_helper_class(2)
        assert sys.getrefcount(result) == 3
        assert result == [1, 2]
        result = cParseArgsHelper.parse_mutable_defaults_with_helper_class(3)
        assert sys.getrefcount(result) == 3
        assert result == [1, 2, 3]

        local_list = []
        assert sys.getrefcount(local_list) == 2
        assert cParseArgsHelper.parse_mutable_defaults_with_helper_class(10, local_list) == [10]
        assert sys.getrefcount(local_list) == 2
        assert cParseArgsHelper.parse_mutable_defaults_with_helper_class(11, local_list) == [10, 11]
        assert sys.getrefcount(local_list) == 2

        result = cParseArgsHelper.parse_mutable_defaults_with_helper_class(4)
        assert result == [1, 2, 3, 4]
        assert sys.getrefcount(result) == 3



















.. index::
    single: C++; Homogeneous Containers
    single: C++; Project PyCppContainers

============================================
Homogeneous Python Containers and C++
============================================

Here are some useful generic functions that can convert homogeneous Python containers to and from their C++ STL
equivalents in this project:
`Python/C++ homogeneous containers on GitHub <https://github.com/paulross/PyCppContainers>`_
The project uses a mixture of templates and code generation to provide 300+ functions to convert to and from
C++ and Python containers.

Here is the introduction to that project:

Python is well known for it's ability to handle *heterogeneous* data in containers such as lists like:

.. code-block:: python

    >>> l = [1, 2.0, "some string", ]

But what if you need to interact with C++ containers such as ``std::vector<T>`` that require *homogeneous* data types?

This project is about converting Python containers such as ``list``, ``tuple``, ``dict``, ``set``, ``frozenset``
containing homogeneous types such as ``bool``, ``int``, ``float``, ``complex``, ``bytes``, ``str`` or user defined
types to and from their C++ equivalent.

Here is a general example of the use of this library where Python data needs to be passed to and from a C++ library and
those results need to be presented in Python.
Like this, visually:

.. code-block:: text

          Python        |   This Library (C++/Python)   |  Some C++ Library
    ------------------- . ----------------------------- . ------------------
            |           .                               .
     Get Python data    .                               .
            |           .                               .
            \---------------------->\                   .
                        .           |                   .
                        .  Convert Python data to C++   .
                        .           |                   .
                        .           \---------------------------->\
                        .                               .         |
                        .                               .  Process C++ data
                        .                               .         |
                        .           /<----------------------------/
                        .           |                   .
                        .  Convert C++ data to Python   .
                        .           |                   .
            /<----------------------/                    .
            |           .                               .
    Process Python data .                               .
            |           .                               .

Here is a, problematic, example of how to do this:

.. raw:: latex

    \pagebreak

--------------------------------
A Problematic Example
--------------------------------

Suppose that you have a Python list of floats and need to pass it to a C++ library that expects a
``std::vector<double>``.
If the result of that call modifies the C++ vector, or creates a new one, you need to return a Python list of floats
from the result.

Your C++ code might look like this (error checking omitted):

.. code-block:: cpp

    PyObject *example(PyObject *op) {
        std::vector<double> vec;
        // Populate the vector, function to be defined...
        write_to_vector(op, vec);
        // Do something in C++ with the vector
        // ...
        // Convert the vector back to a Python list.
        // Function to be defined...
        return read_from_vector(vec);
    }

What should the implementation of ``write_to_vector()`` and ``read_from_vector()`` look like?

The answer seems fairly simple; firstly ``write_to_vector`` converting a Python list to a C++ ``std::vector<double>``
with Pythons C-API:

.. code-block:: cpp

    void write_to_vector(PyObject *op, std::vector<double> &vec) {
        vec.clear();
        for (Py_ssize_t i = 0; i < PyList_Size(op); ++i) {
            vec.push_back(PyFloat_AsDouble(PyList_GET_ITEM(op, i)));
        }
    }

And the inverse, ``read_from_vector`` creating a new Python list from a C++ ``std::vector<double>``:

.. code-block:: cpp

    PyObject *read_from_vector(const std::vector<double> &vec) {
        PyObject *ret = PyList_New(vec.size());
        for (size_t i = 0; i < vec.size(); ++i) {
            PyList_SET_ITEM(ret, i, PyFloat_FromDouble(vec[i]));
        }
        return ret;
    }


There is no error handling shown here, and all errors would be runtime errors.

However if you need to support other object types, say lists of ``int``, ``str``, ``bytes`` then each one needs a pair
of hand written functions; Python to C++ and C++ to Python.
It gets worse when you want to support other containers such as ``tuple``, ``list``, ``set``, ``frozenset``, ``dict``.
You end up with hundreds of functions, all individually named, to handle all the combinations.
Then you have to write individual conversion functions, and their tests, for all the combinations of object types *and*
containers.

This is tedious and error prone and hard to extend in the general case.

Why This Project
=========================

This project simplifies the problem of converting data from Python to C++ and vice versa *in general*.

The project makes extensive use of C++ templates, partial template specialisation and code generation to dramatically
reduce the amount of hand maintained code.
It also converts many runtime errors to compile time errors.

The types and containers this library supports are:

.. list-table:: **Supported Object types.**
   :widths: 20 10 45
   :header-rows: 1

   * - **C++ Type**
     - **Python Type**
     - **Notes**
   * - ``bool``
     - ``True``, ``False``
     -
   * - ``long``
     - ``int``
     -
   * - ``double``
     - ``float``
     -
   * - ``std::complex<double>``
     - ``complex``
     -
   * - ``std::vector<char>``
     - ``bytes``
     - ``bytearray`` is not supported as we need hashable types for ``set`` and ``dict`` containers.
   * - ``std::string``
     - ``str``
     - Specifically a ``PyUnicode_1BYTE_KIND`` [#f1]_.
       `Python documentation <https://docs.python.org/3/c-api/unicode.html>`_
   * - ``std::u16string``
     - ``str``
     - Specifically a ``PyUnicode_2BYTE_KIND``.
       `Python documentation <https://docs.python.org/3/c-api/unicode.html>`_
   * - ``std::u32string``
     - ``str``
     - Specifically a ``PyUnicode_4BYTE_KIND``.
       `Python documentation <https://docs.python.org/3/c-api/unicode.html>`_

Used in these containers:

.. list-table:: **Supported Containers.**
   :widths: 50 50
   :header-rows: 1

   * - **C++ Container**
     - **Python Equivalent**
   * - ``std::vector``
     - Either a ``tuple`` or ``list``
   * - ``std::list``
     - Either a ``tuple`` or ``list``
   * - ``std::unordered_set``
     - Either a ``set`` or ``frozenset``
   * - ``std::unordered_map``
     - ``dict``
   * - ``std::map``
     - ``dict``

The number of possible conversion functions is worse than the cartesian product of the types and containers as in the
case of a dict the types can appear as either a key or a value.

Supporting all these conversions would normally require 352 conversion functions to be written, tested and documented
[#f2]_ .

This project simplifies this by using a mix of C++ templates and code generators to reduce this number to just
**six** hand written templates for all 352 cases.

Using This Library
========================

Python to C++
-------------------

Using the library is as simple as this, suppose you have data in Python that needs to be passed to a C++ library:

.. code-block:: text

          Python        |   This Library (C++/Python)   |  Some C++ Library
    ------------------- . ----------------------------- . ------------------
            |           .                               .
    Python data source  .                               .
            |           .                               .
            \---------------------->\                   .
                        .           |                   .
                        .  Convert Python data to C++   .
                        .           |                   .
                        .           \------------------------------>\
                        .                               .           |
                        .                               .    Process C++ data

The C++ code using this library looks like this:

C++ Code
^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

    #include "python_convert.h"

    // Create a Python list of floats: [21.0, 42.0, 3.0]
    PyObject *op = Py_BuildValue("[ddd]", 21.0, 42.0, 3.0);

    // Create the C++ vector that we want to convert this data to...
    std::vector<double> cpp_vector;

    // The template specialisation will automatically invoke the appropriate
    // function call.
    // It will be a compile time error if the container/type function
    // is not supported.
    // At run time this will return zero on success, non-zero on failure,
    // for example if op is not a Python tuple or members of op can not be
    // converted to C++ doubles.
    int err = Python_Cpp_Containers::py_list_to_cpp_std_list_like(op, cpp_vector);
    // Handle error checking if err is non-zero...

.. note::

    If you were to change the C++ container to a ``std::list<double>`` the function call
    ``py_list_to_cpp_std_list_like()`` would be the same.
    Of course ``py_list_to_cpp_std_list_like()`` would then dispatch to code handling a ``std::list<double>``.

Another example, suppose the Python data source is a ``typing.Dict[int, str]`` and this needs to be converted to a
C++ ``std::map<long, std::string>>`` then a function using the conversion code using this library is as simple as this:

.. code-block:: cpp

    #include "python_convert.h"

    void convert_py_data_to_cpp(PyObject *arg) {
        std::map<long, std::string> map;
        if (Python_Cpp_Containers::py_dict_to_cpp_std_map_like(arg, map)) {
            // Handle error...
        } else {
            // Use the map...
        }
    }

.. note::

    If you were to change the C++ container to a ``std::unordered_map<long, std::string>`` the function call
    ``py_dict_to_cpp_std_map_like()`` would be the same.
    Of course ``py_dict_to_cpp_std_map_like()`` would then dispatch to code handling a
    ``std::unordered_map<long, std::string>``.


C++ to Python
-------------------

Suppose that you have data from a C++ library and this data needs to be represented in Python:

.. code-block:: text

          Python        |   This Library (C++/Python)   |  Some C++ Library
    ------------------- . ----------------------------- . ------------------
                        .                               .  C++ data source
                        .                               .        |
                        .           /<---------------------------/
                        .           |                   .
                        .  Convert C++ data to Python   .
                        .           |                   .
            /<----------------------/                    .
            |           .                               .
        Python data     .                               .
            |           .                               .

The C++ code using this library looks like this:

.. code-block:: cpp

    #include "python_convert.h"

    std::vector<double> cpp_vector;
    // Populate the C++ vector...
    cpp_vector.push_back(21.0);
    cpp_vector.push_back(42.0);
    cpp_vector.push_back(3.0);

    // Now convert to Python.
    // This will be a compile time error if the C++ type is not supported.
    PyObject *op  = Python_Cpp_Containers::cpp_std_list_like_to_py_list(cpp_vector);
    // op is a Python list of floats: [21.0, 42.0, 3.0]
    // op will be null on failure and a Python exception will have been set.

.. note::

    If you were to change the C++ container to a ``std::list<double>`` the function call
    ``cpp_std_list_like_to_py_list()`` would be the same.
    Of course ``cpp_std_list_like_to_py_list()`` would then dispatch to code handling a ``std::list<double>``.

Another example, suppose the C++ data source is a ``std::map<long, std::string>>`` and we need this a Python dict
``typing.Dict[int, str]`` then the conversion code in this library is as simple as this:

.. code-block:: cpp

    #include "python_convert.h"

    PyObject *convert_cpp_data_to_py() {
        std::map<long, std::string> map;
        // Populate map from the C++ data source
        // ...
        // Now convert to a Python dict:
        return Python_Cpp_Containers::cpp_std_map_like_to_py_dict(map);
    }

The Hand Written Functions
=============================

At the heart off this library here are just six non-trivial hand written functions along with a much larger of
generated functions that successively specialise these handwritten functions.
They are defined as templates in ``src/cpy/python_object_convert.h``.

* Two C++ templates for Python ``tuple`` / ``list`` to and from ``std::list`` or ``std::vector`` for all types.
* Two C++ templates for Python ``set`` / ``frozenset`` to and from ``std::unordered_set`` for all types.
* Two C++ templates for Python ``dict`` to and from ``std::map`` or ``std::unordered_map`` for all type pairs.

These six handwritten templates are short, simple and comprehensible.
Then, for simplicity, a Python script is used to create the final, instantiated, 352 functions.

As an example, here how the function is developed that converts a Python list of ``float`` to and from a C++
``std::vector<double>`` or ``std::list<double>``.

First C++ to Python.

Converting a C++ ``std::vector<T>`` or ``std::list<T>`` to a Python ``tuple`` or ``list``
--------------------------------------------------------------------------------------------------------------------

The generic function signature looks like this:

.. code-block:: cpp

    template<
        template<typename ...> class ListLike,
        typename T,
        PyObject *(*ConvertCppToPy)(const T &),
        PyObject *(*PyUnaryContainer_New)(size_t),
        int(*PyUnaryContainer_Set)(PyObject *, size_t, PyObject *)
    >
    PyObject *
    very_generic_cpp_std_list_like_to_py_unary(const ListLike<T> &list_like) {
        // Handwritten code, see "C++ to Python Implementation" below.
        // ...
    }

.. list-table:: ``very_generic_cpp_std_list_like_to_py_unary()`` template parameters.
   :widths: 30 70
   :header-rows: 1

   * - Template Parameter
     - Notes
   * - ``ListLike``
     - The C++ container type, either a ``std::vector`` or ``std::list``.
   * - ``T``
     - The C++ type of the objects in the target C++ container.
   * - ``ConvertCppToPy``
     - A pointer to a function that converts any C++ ``T`` to a ``PyObject *``, for example from ``double`` -> ``float``.
       The function signature is ``PyObject *ConvertCppToPy(const T&)``.
       This returns NULL on failure.
   * - ``PyUnaryContainer_New``
     - A pointer to a function that creates a new Python container, for example a ``list``, of a particular length.
       The function signature is ``PyObject *PyUnaryContainer_New(Py_ssize_t)``.
       This returns NULL on failure.
   * - ``PyUnaryContainer_Set``
     - A pointer to a function that sets a ``PyObject *`` in the Python container at a given index.
       The function signature is ``int PyUnaryContainer_Set(PyObject *container, size_t pos, PyObject *value))``.
       This returns 0 on success.

And the function has the following parameters.

.. list-table:: ``very_generic_cpp_std_list_like_to_py_unary()`` parameters.
   :widths: 20 20 50
   :header-rows: 1

   * - Type
     - Name
     - Notes
   * - ``ListLike<T> &``
     - ``list_like``
     - The C++ list like container to read from to.

The return value is non-NULL on success or NULL if there is a runtime error.
These errors could be:

* ``PyObject *`` container can not be created.
* A member of the Python container can not be created from the C++ type ``T``.
* The ``PyObject *`` can not be inserted into the Python container.

C++ to Python Implementation
--------------------------------

The implementation is fairly straightforward in ``src/cpy/python_object_convert.h`` (lightly edited):

.. code-block:: cpp

    template<
            template<typename ...> class ListLike,
            typename T,
            PyObject *(*ConvertCppToPy)(const T &),
            PyObject *(*PyUnaryContainer_New)(size_t),
            int(*PyUnaryContainer_Set)(PyObject *, size_t, PyObject *)
    >
    PyObject *
    very_generic_cpp_std_list_like_to_py_unary(const ListLike<T> &list_like) {
        assert(!PyErr_Occurred());
        PyObject *ret = PyUnaryContainer_New(list_like.size());
        if (ret) {
            size_t i = 0;
            for (const auto &val: list_like) {
                PyObject *op = (*ConvertCppToPy)(val);
                if (!op) {
                    // Failure, do not need to decref the contents as that will
                    // be done when decref'ing the container.
                    // e.g. tupledealloc():
                    // https://github.com/python/cpython/blob/main/Objects/tupleobject.c
                    PyErr_Format(PyExc_ValueError, "C++ value of can not be converted.");
                    goto except;
                }
                // PyUnaryContainer_Set wraps a function returning non-zero on error.
                if (PyUnaryContainer_Set(ret, i++, op)) { // Stolen reference.
                    PyErr_Format(PyExc_RuntimeError, "Can not set unary value.");
                    goto except;
                }
            }
        } else {
            PyErr_Format(
                PyExc_ValueError,
                "Can not create Python container of size %ld",
                list_like.size()
            );
            goto except;
        }
        assert(!PyErr_Occurred());
        assert(ret);
        goto finally;
    except:
        Py_XDECREF(ret);
        assert(PyErr_Occurred());
        ret = NULL;
    finally:
        return ret;
    }

Partial Specialisation to Convert a C++ ``std::vector<T>`` or ``std::list<T>`` to a Python ``list```
-------------------------------------------------------------------------------------------------------

As an example this is specialised for a C++ ``std::vector`` and a Python ``list`` with a handwritten oneliner:

.. code-block:: cpp

    template<
        typename T,
        PyObject *(*ConvertCppToPy)(const T &)
    >
    PyObject *
    generic_cpp_std_list_like_to_py_list(const std::vector<T> &container) {
        return very_generic_cpp_std_list_like_to_py_unary<
            std::vector, T, ConvertCppToPy, &py_list_new, &py_list_set
        >(container);
    }

.. note::

    The use of the function pointers to ``py_list_new``, and ``py_list_set`` that are defined in this
    project namespace.
    These are thin wrappers around existing functions or macros in ``"Python.h"``.
    There is no error checking in these functions.

    For example:

    .. code-block:: c

            PyObject *py_list_new(size_t len) {
                return PyList_New(len);
            }
            int py_list_set(PyObject *list_p, size_t pos, PyObject *op) {
                // No error checking, always "succeeds".
                PyList_SET_ITEM(list_p, pos, op);
                return 0;
            }

There is a similar partial specialisation for a Python ``tuple``:

.. code-block:: cpp

    template<
        typename T,
        PyObject *(*ConvertCppToPy)(const T &)
    >
    PyObject *
    generic_cpp_std_list_like_to_py_list(const std::vector<T> &container) {
        return very_generic_cpp_std_list_like_to_py_unary<
            std::vector, T, ConvertCppToPy, &py_tuple_new, &py_tuple_set
        >(container);
    }

And the tuple functions are trivial and look like the list ones in the note above.
There is no error checking in these functions:

.. code-block:: c

    PyObject *py_tuple_new(size_t len) {
        return PyTuple_New(len);
    }
    int py_tuple_set(PyObject *tuple_p, size_t pos, PyObject *op) {
        // No error checking, always "succeeds".
        PyTuple_SET_ITEM(tuple_p, pos, op);
        return 0;
    }

Converting a Python ``tuple`` or ``list`` to a C++ ``std::vector<T>`` or ``std::list<T>``
--------------------------------------------------------------------------------------------------

The reverse is converting Python to C++.
This generic function that converts unary Python indexed containers (``tuple`` and ``list``) to a C++ ``std::vector<T>``
or ``std::list<T>`` for any type has this signature:

.. code-block:: cpp

    template<
            template<typename ...> class ListLike,
            typename T,
            int (*PyObject_Check)(PyObject *),
            T (*PyObject_Convert)(PyObject *),
            int(*PyUnaryContainer_Check)(PyObject *),
            Py_ssize_t(*PyUnaryContainer_Size)(PyObject *),
            PyObject *(*PyUnaryContainer_Get)(PyObject *, size_t)>
    int very_generic_py_unary_to_cpp_std_list_like(
        PyObject *op, ListLike<T> &list_like
    ) {
        // Handwritten code, see "Python to C++ Implementation" below.
        // ...
    }

This template has these parameters:

.. list-table:: ``very_generic_py_unary_to_cpp_std_list_like()`` template parameters.
   :widths: 20 50
   :header-rows: 1

   * - Template Parameter
     - Notes
   * - ``ListLike``
     - The C++ container type, either a ``std::vector`` or ``std::list``.
   * - ``T``
     - The C++ type of the objects in the target C++ container.
   * - ``PyObject_Check``
     - A pointer to a function that checks that any ``PyObject *`` in the Python container is the correct type,
       for example that it is a ``bytes`` object.
       The function signature is ``int PyObject_Check(PyObject *)``.
       This returns non-zero if the Python object is as expected.
   * - ``PyObject_Convert``
     - A pointer to a function that converts any ``PyObject *`` in the Python container to the C++ type, for example
       from ``bytes`` -> ``std::vector<char>``.
       The function signature is ``T PyObject_Convert(PyObject *)``.
   * - ``PyUnaryContainer_Check``
     - A pointer to a function that checks that the ``PyObject *`` argument is the correct container type, for example
       a ``tuple``.
       The function signature is ``int PyUnaryContainer_Check(PyObject *)``.
       This returns non-zero if the Python container is not as expected.
   * - ``PyUnaryContainer_Size``
     - A pointer to a function that returns the size of the Python container.
       The function signature is ``Py_ssize_t PyUnaryContainer_Size(PyObject *op)``.
       This returns the size of the the Python container.
   * - ``PyUnaryContainer_Get``
     - A pointer to a function that gets a ``PyObject *`` from the Python container at a given index.
       The function signature is ``PyObject *PyUnaryContainer_Get(PyObject *, size_t)``.

And the function has the following parameters.

.. list-table:: ``generic_py_unary_to_cpp_std_list_like()`` parameters.
   :widths: 20 20 50
   :header-rows: 1

   * - Type
     - Name
     - Notes
   * - ``PyObject *``
     - ``op``
     - The Python container to read from.
   * - ``ListLike<T> &``
     - ``list_like``
     - The C++ list like container to write to.

The return value is zero on success or non-zero if there is a runtime error.
These errors could be:

* ``PyObject *op`` is not a container of the required type.
* A member of the Python container can not be converted to the C++ type ``T`` (``PyObject_Check`` fails).

Python to C++ Implementation
----------------------------------

The implementation is fairly straightforward in ``src/cpy/python_object_convert.h`` (lightly edited):

.. code-block:: cpp

    template<
            template<typename ...> class ListLike,
            typename T,
            int (*PyObject_Check)(PyObject *),
            T (*PyObject_Convert)(PyObject *),
            int(*PyUnaryContainer_Check)(PyObject *),
            Py_ssize_t(*PyUnaryContainer_Size)(PyObject *),
            PyObject *(*PyUnaryContainer_Get)(PyObject *, size_t)
    >
    int very_generic_py_unary_to_cpp_std_list_like(PyObject *op, ListLike<T> &list_like) {
        assert(!PyErr_Occurred());
        int ret = 0;
        list_like.clear();
        Py_INCREF(op); // Increment borrowed reference
        if (!PyUnaryContainer_Check(op)) {
            PyErr_Format(
                PyExc_ValueError,
                "Can not convert Python container of type %s",
                op->ob_type->tp_name
            );
            ret = -1;
            goto except;
        }
        for (Py_ssize_t i = 0; i < PyUnaryContainer_Size(op); ++i) {
            PyObject *value = PyUnaryContainer_Get(op, i);
            if (!value) {
                ret = -2;
                goto except;
            }
            if (!(*PyObject_Check)(value)) {
                list_like.clear();
                PyErr_Format(
                        PyExc_ValueError,
                        "Python value of type %s can not be converted",
                        value->ob_type->tp_name
                );
                ret = -3;
                goto except;
            }
            list_like.push_back((*PyObject_Convert)(value));
            // Check !PyErr_Occurred() which could never happen as we check first.
        }
        assert(!PyErr_Occurred());
        goto finally;
    except:
        assert(PyErr_Occurred());
        list_like.clear();
    finally:
        Py_DECREF(op); // Decrement borrowed reference
        return ret;
    }

Partial Specialisation to Convert a Python ``list`` to a C++ ``std::vector<T>`` or ``std::list<T>``
-------------------------------------------------------------------------------------------------------

This template can be partially specialised for converting Python *lists* of any type to C++ ``std::vector<T>`` or ``std::list<T>``.
This is hand written code but it is trivial by wrapping a single function call.

In the particular case of a ``std::vector`` we can use ``.reserve()`` as an optimisations to avoid excessive re-allocations.

.. code-block:: cpp

    template<
        typename T,
        int (*PyObject_Check)(PyObject *),
        T (*PyObject_Convert)(PyObject *)
    >
    int generic_py_list_to_cpp_std_list_like(
        PyObject *op, std::vector<T> &container
    ) {
        // Reserve the vector, but only if it is a list.
        // If it is any other Python object then ignore it as py_list_len()
        // may give undefined behaviour.
        // Leave it to very_generic_py_unary_to_cpp_std_list_like() to error
        if (py_list_check(op)) {
            container.reserve(py_list_len(op));
        }
        return very_generic_py_unary_to_cpp_std_list_like<
            std::vector, T, PyObject_Check, PyObject_Convert,
            &py_list_check, &py_list_len, &py_list_get
        >(op, container);
    }

.. note::

    The use of the function pointers to ``py_list_check``, ``py_list_len`` and ``py_list_get`` that are defined in this
    project namespace.
    These are thin wrappers around existing functions or macros in ``"Python.h"``.
    There is no error checking in these functions.

    For example:

    .. code-block:: c

            int py_list_check(PyObject *op) {
                return PyList_Check(op);
            }
            Py_ssize_t py_list_len(PyObject *op) {
                return PyList_Size(op);
            }
            PyObject *py_list_get(PyObject *list_p, size_t pos) {
                return PyList_GET_ITEM(list_p, pos);
            }

There is a similar partial specialisation for the Python ``tuple``:

.. code-block:: cpp

    template<typename T, int (*PyObject_Check)(PyObject *), T (*PyObject_Convert)(PyObject *)>
    int generic_py_tuple_to_cpp_std_list_like(PyObject *op, std::vector<T> &container) {
        // Reserve the vector, but only if it is a tuple.
        // If it is any other Python object then ignore it as py_tuple_len()
        // may give undefined behaviour.
        // Leave it to very_generic_py_unary_to_cpp_std_list_like() to error
        if (py_tuple_check(op)) {
            container.reserve(py_tuple_len(op));
        }
        return very_generic_py_unary_to_cpp_std_list_like<
                std::vector, T, PyObject_Check, PyObject_Convert,
                &py_tuple_check, &py_tuple_len, &py_tuple_get
        >(op, container);
    }

The functions ``py_tuple_len`` and ``py_tuple_get`` are thin wrappers round existing functions or macros in
``"Python.h"`` as above.

Generated Functions
=============================

The particular function specialisations are created by a Python script that takes the cartesian product of object types
and container types and creates functions for each container/object.

C++ to Python
----------------------------

For example, to convert a C++ ``std::vector<double>`` to a Python ``list`` of ``float`` the following are created:

A base declaration in *auto_py_convert_internal.h*:

.. code-block:: cpp

    template<typename T>
    PyObject *
    cpp_std_list_like_to_py_list(const std::vector<T> &container);

And a concrete declaration for each C++ target type ``T`` in *auto_py_convert_internal.h*:

.. code-block:: cpp

    template <>
    PyObject *
    cpp_std_list_like_to_py_list<double>(const std::vector<double> &container);

And the concrete definition is in *auto_py_convert_internal.cpp*, this simply calls the generic function:

.. code-block:: cpp

    template <>
    PyObject *
    cpp_std_list_like_to_py_list<double>(const std::vector<double> &container) {
        return generic_cpp_std_list_like_to_py_list<
            double, &cpp_double_to_py_float
        >(container);
    }

Here is the function hierarchy for converting lists to C++ ``std::vector<T>`` or ``std::list<T>``:
This is the function hierarchy for the code that converts C++ ``std::vector<T>`` or ``std::list<T>`` to Python
``list`` and ``tuple`` for all supported object types.

.. code-block:: none

                      very_generic_cpp_std_list_like_to_py_unary <-- Hand written
                                           |
                            /--------------------------\
                            |                          |             Hand written partial
            generic_cpp_std_list_like_to_py_list    tuples...    <-- specialisation for
                            |                          |             std::vector
                            |                          |             and std::list
                            |                          |             (generally trivial).
                            |                          |
              cpp_std_list_like_to_py_list<T>         ...        <-- Generated
                            |                          |
            /-------------------------------\      /-------\
            |                               |      |       |         Generated declaration
    cpp_std_list_like_to_py_list<double>   ...    ...     ...    <-- and implementation
                                                                     (one liners)

Python to C++
----------------------------

For example, to convert a Python ``list`` of ``float`` to a C++ ``std::vector<double>`` the following are generated:

A base declaration in *auto_py_convert_internal.h*:

.. code-block:: cpp

    template<typename T>
    int
    py_list_to_cpp_std_list_like(PyObject *op, std::list<T> &container);

And a concrete declaration for each C++ target type ``T`` in *auto_py_convert_internal.h*:

.. code-block:: cpp

    template <>
    int
    py_list_to_cpp_std_list_like<double>(PyObject *op, std::list<double> &container);


And the concrete definition is in *auto_py_convert_internal.cpp*:

.. code-block:: cpp

    template <>
    int
    py_list_to_cpp_std_list_like<double>(PyObject *op, std::vector<double> &container) {
        return generic_py_list_to_cpp_std_list_like<
            double, &py_float_check, &py_float_to_cpp_double
        >(op, container);
    }


.. raw:: latex

    [Continued on the next page]

    \pagebreak

This is the function hierarchy for the code that converts Python ``list`` and ``tuple`` to C++ ``std::vector<T>`` or
``std::list<T>`` for all supported object types.

.. code-block:: none

                      very_generic_py_unary_to_cpp_std_list_like <-- Hand written
                                           |
                            /--------------------------\
                            |                          |             Hand written partial
            generic_py_list_to_cpp_std_list_like    tuples...    <-- specialisation for
                            |                          |             std::vector
                            |                          |             and std::list
                            |                          |             (generally trivial).
                            |                          |
              py_list_to_cpp_std_list_like<T>         ...        <-- Generated
                            |                          |
            /-------------------------------\      /-------\
            |                               |      |       |         Generated declaration
    py_list_to_cpp_std_list_like<double>   ...    ...     ...    <-- and implementation
                                                                     (one liners)


More information can be found from this project
`Python/C++ homogeneous containers on GitHub <https://github.com/paulross/PyCppContainers>`_.


.. rubric:: Footnotes

.. [#f1] We are currently targeting C++14 so we use ``std::string`` which is defined as ``std::basic_string<char>``.
    C++20 allows a stricter, and more desirable, definition ``std::basic_string<char8_t>`` that we could use here.
    See `C++ reference for std::string <https://en.cppreference.com/w/cpp/string>`_
.. [#f2] There are six unary container pairings (``tuple`` <-> ``std::list``, ``tuple`` <-> ``std::vector``,
    ``list`` <-> ``std::list``, ``list`` <-> ``std::vector``,
    ``set`` <-> ``std::unordered_set``, ``frozenset`` <-> ``std::unordered_set``) with eight types
    (``bool``, ``int``, ``float``, ``complex``, ``bytes``, ``str[1]``, ``str[2]``, ``str[4]``).
    Each container/type combination requires two functions to give two way conversion from Python to C++ and back.
    Thus 6 (container pairings) * 8 (types) * 2 (way conversion) = 96 required functions.
    For ``dict`` there are two container pairings (``dict`` <-> ``std::map``, ``dict`` <-> ``std::unordered_map``)
    with the eight types either of which can be the key or the value so 64 (8**2) possible variations.
    Thus 2 (container pairings) * 64 (type pairs) * 2 (way conversion) = 256 required functions.
    Thus is a total of 96 + 256 = 352 functions.
