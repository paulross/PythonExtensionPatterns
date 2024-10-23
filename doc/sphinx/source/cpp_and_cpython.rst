.. highlight:: cpp
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _cpp_and_cpython:

============================================
C++ RAII Wrappers Around ``PyObject*``
============================================

It is sometimes useful to wrap up a ``PyObject*`` in a class that will manage the reference count. Here is a base class that shows the general idea, it takes a ``PyObject *`` and provides:

* Construction with a ``PyObject *`` and access this with ``operator PyObject*() const``.
* ``PyObject **operator&()`` to reset the underlying pointer, for example when using it with ``PyArg_ParseTupleAndKeywords``.
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

============================================
Handling Default Arguments
============================================

Handling default, possibly mutable, arguments in a pythonic way is described here:
:ref:`cpython_default_mutable_arguments`.
It is quite complicated to get it right but C++ can ease the pain with a generic class to simplify handling default
arguments in CPython functions:

.. code-block:: cpp

    class DefaultArg {
    public:
        DefaultArg(PyObject *new_ref) : m_arg { NULL }, m_default { new_ref } {}
        // Allow setting of the (optional) argument with PyArg_ParseTupleAndKeywords
        PyObject **operator&() { m_arg = NULL; return &m_arg; }
        // Access the argument or the default if default.
        operator PyObject*() const { return m_arg ? m_arg : m_default; }
        // Test if constructed successfully from the new reference.
        explicit operator bool() { return m_default != NULL; }
    protected:
        PyObject *m_arg;
        PyObject *m_default;
    };

Suppose we have the Python function signature of ``def function(encoding='utf8', cache={}):`` then in C/C++ we can do this:

.. code-block:: cpp

    PyObject *
    function(PyObject * /* module */, PyObject *args, PyObject *kwargs) {
        /* ... */
        static DefaultArg encoding(PyUnicode_FromString("utf8"));
        static DefaultArg cache(PyDict_New());
        /* Check constructed OK. */
        if (! encoding || ! cache) {
            return NULL;
        }
        static const char *kwlist[] = { "encoding", "cache", NULL };
        if (! PyArg_ParseTupleAndKeywords(args, kwargs, "|OO", const_cast<char**>(kwlist), &encoding, &cache)) {
            return NULL;
        }
        /* Then just use encoding, cache as if they were a PyObject* (possibly
         * might need to be cast to some specific PyObject*). */
         
        /* ... */
    }

The full code is in ``src/cpy/cParseArgsHelper.cpp`` and the tests in ``tests/unit/test_c_parse_args_helper.py``.

============================================
Homogeneous Python Containers and C++
============================================

Here are some useful generic functions that can convert homogeneous Python containers to and from their C++ STL
equivalents in this project:
`Python/C++ homogeneous containers on GitHub <https://github.com/paulross/PyCppContainers>`_
The project uses a mixture of templates and code generation to provide 200+ functions to convert to and from
C++ and Python containers.

Here is the introduction to that project:

Python is well known for it's ability to handle *heterogeneous* data in containers such as lists.
But what if you need to interact with C++ containers such as ``std::vector<T>`` that require *homogeneous* data types?


This project is about converting Python containers (``list``, ``tuple``, ``dict``, ``set``, ``frozenset``) containing
homogeneous types (``bool``, ``int``, ``float``, ``complex``, ``bytes``, ``str``) or user defined types to and from
their C++ equivalent.


------------------------------
A Problematic Example
------------------------------

Suppose that you have a Python list of floats and need to pass it to a C++ library that expects a ``std::vector<double>``.
If the result of that call modifies the C++ vector, or creates a new one, you need to return a Python list of floats
from the result.

Your code might look like this:

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

The answer seems fairly simple; firstly ``write_to_vector`` converting a Python list to a C++ ``std::vector<double>``:

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


There is no error handling here, all errors would be runtime errors.

However if you need to support other object types, say lists of ``int``, ``str``, ``bytes`` then each one needs a pair of hand written functions.
It gets worse when you want to support other containers such as (``tuple``, ``list``, ``set``, ``frozenset``, ``dict``).
Then you have to write individual conversion functions for all the combinations of object types *and* containers.
This is tedious and error prone.

Why This Project
=========================

This project makes extensive use of C++ templates, partial template specialisation and code generation to reduce
dramatically the amount of hand maintained code.
It also converts many runtime errors to compile time errors.

This project supports two way conversion of this set of containers:

.. list-table:: Supported Containers.
   :widths: 50 50
   :header-rows: 1

   * - C++ Container
     - Python Equivalent
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

Which contain any of this set of types:

.. list-table:: Supported Object types.
   :widths: 30 30
   :header-rows: 1

   * - C++ Type
     - Python Equivalent
   * - ``bool``
     - ``True``, ``False``
   * - ``long``
     - ``int``
   * - ``double``
     - ``float``
   * - ``std::complex<double>``
     - ``complex``
   * - ``std::vector<char>``
     - ``bytes``
   * - ``std::string``
     - ``str``

The number of possible conversion functions is worse than the cartesian product of the types and containers as in the case of a
dict the types can appear as either a key or a value.

Supporting all these conversions would normally require 216 conversion functions to be written, tested and documented [#]_ .

This project simplifies this by using a mix of C++ templates and code generators to reduce this number to just
**six** hand written templates for all 216 cases.

* Two C++ templates for Python ``tuple`` / ``list`` to and from ``std::list`` or ``std::vector`` for all types.
* Two C++ templates for Python ``set`` / ``frozenset`` to and from ``std::unordered_set`` for all types.
* Two C++ templates for Python ``dict`` to and from ``std::map`` or ``std::unordered_map`` for all type combinations.

These six handwritten templates are fairly simple and comprehensible.
Then, for simplicity, a Python script is used to create the final, instantiated, 216 functions.

Hand Written Functions
=============================

There are only six non-trivial hand written functions along with a much larger of generated functions that successively
specialise these functions.

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
        // Handwritten
        // ...
    }

.. list-table:: ``very_generic_cpp_std_list_like_to_py_unary()`` template parameters.
   :widths: 20 50
   :header-rows: 1

   * - Template Parameter
     - Notes
   * - ``ListLike``
     - The C++ container type, either a ``std::vector<T>`` or ``std::list<T>``.
   * - ``T``
     - The C++ type of the objects in the target C++ container.
   * - ``ConvertCppToPy``
     - A pointer to a function that converts any C++ ``T`` to a ``PyObject *``, for example from ``double`` -> ``float``.
   * - ``PyUnaryContainer_New``
     - A pointer to a function that creates a new Python container, for example a ``list``, of a particular length.
   * - ``PyUnaryContainer_Set``
     - A pointer to a function that sets a ``PyObject *`` in the Python container at a given index.

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


Partial Specialisation to Convert a C++ ``std::vector<T>`` or ``std::list<T>`` to a Python ``list```
-------------------------------------------------------------------------------------------------------

As an example this is specialised for Python ``list`` with a handwritten oneliner:

.. code-block:: cpp

    template<
        typename T,
        PyObject *(*ConvertCppToPy)(const T &)
    >
    PyObject *
    generic_cpp_std_list_like_to_py_list(const std::vector<T> &container) {
        return very_generic_cpp_std_list_like_to_py_unary<
            std::vector,
            T,
            ConvertCppToPy,
            &py_list_new,
            &py_list_set
        >(container);
    }

.. note::

    The use of the function pointers to ``py_list_new``, and ``py_list_set`` that are defined in this
    project namespace.
    These are thin wrappers around existing functions or macros in ``"Python.h"``.

There is a similar partial specialisation for ``tuple``.

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
        // Hand written code.
        // ...
    }

This template has these parameters:

.. list-table:: ``very_generic_py_unary_to_cpp_std_list_like()`` template parameters.
   :widths: 20 50
   :header-rows: 1

   * - Template Parameter
     - Notes
   * - ``ListLike``
     - The C++ container type, either a ``std::vector<T>`` or ``std::list<T>``.
   * - ``T``
     - The C++ type of the objects in the target C++ container.
   * - ``PyObject_Check``
     - A pointer to a function that checks that any ``PyObject *`` in the Python container is the correct type, for example that it is a ``bytes`` object.
   * - ``PyObject_Convert``
     - A pointer to a function that converts any ``PyObject *`` in the Python container to the C++ type, for example from ``bytes`` -> ``std::vector<char>``.
   * - ``PyUnaryContainer_Check``
     - A pointer to a function that checks that the ``PyObject *`` argument is the correct container type, for example a ``tuple``.
   * - ``PyUnaryContainer_Size``
     - A pointer to a function that returns the size of the Python container.
   * - ``PyUnaryContainer_Get``
     - A pointer to a function that gets a ``PyObject *`` from the Python container at a given index.

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
            std::vector,
            T,
            PyObject_Check,
            PyObject_Convert,
            &py_list_check,
            &py_list_len,
            &py_list_get
        >(op, container);
    }

.. note::

    The use of the function pointers to ``py_list_check``, ``py_list_len`` and ``py_list_get`` that are defined in this
    project namespace.
    These are thin wrappers around existing functions or macros in ``"Python.h"``.

There is a similar partial specialisation for ``tuple``.

Generated Functions
=============================

The particular function specialisations are created by a script that takes the cartesian product of object types and
container types and creates functions for each container/object.

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

Usage
========================

Using the concrete function is as simple as this:

.. code-block:: cpp

    using namespace Python_Cpp_Containers;
    // Create a PyObject* representing a list of Python floats.
    PyObject *op = PyList_New(3);
    PyList_SetItem(op, 0, PyFloat_FromDouble(21.0));
    PyList_SetItem(op, 1, PyFloat_FromDouble(42.0));
    PyList_SetItem(op, 2, PyFloat_FromDouble(3.0));

    // Create the output vector...
    std::vector<double> cpp_vector;

    // Template specialisation will automatically invoke the appropriate
    // function call.
    // It will be a compile time error if the container/type function
    // is not available.
    // At run time this will return zero on success, non-zero on failure,
    // for example if op is not a Python tuple or members of op can not be
    // converted to C++ doubles.
    int err = py_list_to_cpp_std_list_like(op, cpp_vector);
    // Handle error checking...

    // Now convert back.
    // Again this will be a compile time error if the C++ type is not supported.
    PyObject *new_op  = cpp_std_list_like_to_py_list(cpp_vector);
    // new_op is a Python list of floats.
    // new_op will be null on failure and a Python exception will have been set.


.. rubric:: Footnotes
.. [#] There are six unary container pairings (``tuple`` <-> ``std::list``, ``tuple`` <-> ``std::vector``,
    ``list`` <-> ``std::list``, ``list`` <-> ``std::vector``,
    ``set`` <-> ``std::unordered_set``, ``frozenset`` <-> ``std::unordered_set``) with six types
    (``bool``, ``int``, ``float``, ``complex``, ``bytes``, ``str``).
    Each container/type combination requires two functions to give two way conversion from Python to C++ and back.
    Thus 6 (container pairings) * 6 (types) * 2 (way conversion) = 72 required functions.
    For ``dict`` there are two container pairings (``dict`` <-> ``std::map``, ``dict`` <-> ``std::unordered_map``)
    with the six types either of which can be the key or the value so 36 possible variations.
    Thus 2 (container pairings) * 36 (type pairs) * 2 (way conversion) = 144 required functions.
    Thus is a total of 72 + 144 = 216 functions.
