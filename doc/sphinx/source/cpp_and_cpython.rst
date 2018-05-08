.. highlight:: cpp
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

============================================
C++ RAII Wrappers Around ``PyObject*``
============================================

It is sometimes useful to wrap up a ``PyObject*`` in a class that will manage the reference count. Here is a base class that shows the general idea, it takes a ``PyObject *`` and provides:

* Construction with a ``PyObject *`` and access this with ``operator PyObject*() const``.
* ``PyObject **operator&()`` to reset the underlying pointer, for example when using it with ``PyArg_ParseTupleAndKeywords``.
* Decrementing the reference count on destruction (potientially freeing the object).

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


============================================
Handling Default Arguments
============================================

Handling default, possibly mutable, arguments in a pythonic way is described here: :ref:`cpython_default_arguments`. It is quite complicated to get it right but C++ can ease the pain with a generic class to simplify handling default arguments in CPython functions:

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

============================================
Homogeneous Python Containers and C++
============================================

Here are some useful generic functions that can convert homogeneous Python containers to and from their C++ STL equivalents. They use templates to identify the C++ type and function pointers to convert from Python to C++ objects and back. These functions must have a these characteristics on error:

* Converting from C++ to Python, on error set a Python error (e.g with ``PyErr_SetString`` or ``PyErr_Format``) and return NULL.
* Converting from Python to C++ set a Python error and return a default C++ object (for example an empty ``std::string``).

For illustration here are a couple of such functions that convert ``PyBytesObject*`` to and from ``std::string``:

.. code-block:: cpp

    std::string py_bytes_to_std_string(PyObject *py_str) {
        std::string r;
        if (PyBytes_Check(py_str)) {
            r = std::string(PyBytes_AS_STRING(py_str));
        } else {
            PyErr_Format(PyExc_TypeError,
                         "Argument %s must be bytes not \"%s\"",
                         __FUNCTION__, Py_TYPE(py_str)->tp_name);
        }
        return r;
    }

    PyObject *std_string_to_py_bytes(const std::string &str) {
        return PyBytes_FromStringAndSize(str.c_str(), str.size());
    }

We can use this for a variety of containers, first Python lists of ``bytes``.

--------------------------------------------
Python Lists and C++ ``std::vector<T>``
--------------------------------------------

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Python ``list`` to C++ ``std::vector<T>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This converts a python list to a ``std::vector<T>``. ``ConvertToT`` is a function pointer to a function that takes a ``PyObject*`` and returns an instance of a ``T`` type. On failure to to convert a ``PyObject*`` this function should set a Python error (making ``PyErr_Occurred()`` non-NULL) and return a default ``T``. On failure this function sets ``PyErr_Occurred()`` and the return value will be an empty vector.

.. code-block:: cpp

    template <typename T>
    std::vector<T>
    py_list_to_std_vector(PyObject *py_list, T (*ConvertToT)(PyObject *)) {
        assert(cpython_asserts(py_list));
        std::vector<T> cpp_vector;

        if (PyList_Check(py_list)) {
            cpp_vector.reserve(PyList_GET_SIZE(py_list));
            for (Py_ssize_t i = 0; i < PyList_GET_SIZE(py_list); ++i) {
                cpp_vector.emplace(cpp_vector.end(),
                                   (*ConvertToT)(PyList_GetItem(py_list, i)));
                if (PyErr_Occurred()) {
                   cpp_vector.clear();
                   break;
                }
            }
        } else {
            PyErr_Format(PyExc_TypeError,
                         "Argument \"py_list\" to %s must be list not \"%s\"",
                         __FUNCTION__, Py_TYPE(py_list)->tp_name);
        }
        return cpp_vector;
    }

If we have a function ``std::string py_bytes_to_std_string(PyObject *py_str);`` (above) we can use this thus, we have to specify the C++ template specialisation:

.. code-block:: cpp

    std::vector<std::string> result = py_list_to_std_vector<std::string>(py_list, &py_bytes_to_std_string);
    if (PyErr_Occurred()) {
        // Handle error condition.
    } else {
        // All good.
    }


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
C++ ``std::vector<T>`` to Python ``list``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

And the inverse that takes a C++ ``std::vector<T>`` and makes a Python ``list``. ``ConvertToPy`` is a pointer to a function that takes an instance of a ``T`` type and returns a ``PyObject*``, this should return NULL on failure and set ``PyErr_Occurred()``. On failure this function sets ``PyErr_Occurred()`` and returns NULL.

.. code-block:: cpp

    template <typename T>
    PyObject*
    std_vector_to_py_list(const std::vector<T> &cpp_vec,
                          PyObject *(*ConvertToPy)(const T&)
                          ) {
        assert(cpython_asserts());
        PyObject *r = PyList_New(cpp_vec.size());
        if (! r) {
            goto except;
        }
        for (Py_ssize_t i = 0; i < cpp_vec.size(); ++i) {
            PyObject *item = (*ConvertToPy)(cpp_vec[i]);
            if (! item || PyErr_Occurred() || PyList_SetItem(r, i, item)) {
                goto except;
            }
        }
        assert(! PyErr_Occurred());
        assert(r);
        goto finally;
    except:
        assert(PyErr_Occurred());
        // Clean up list
        if (r) {
            // No PyList_Clear().
            for (Py_ssize_t i = 0; i < PyList_GET_SIZE(r); ++i) {
                Py_XDECREF(PyList_GET_ITEM(r, i));
            }
            Py_DECREF(r);
            r = NULL;
        }
    finally:
        return r;
    }

If we have a function ``PyObject *std_string_to_py_bytes(const std::string &str);`` (above) we can use this thus:

.. code-block:: cpp

    std::vector<std::string> cpp_vector;
    // Initialise cpp_vector...
    PyObject *py_list = std_vector_to_py_list(cpp_vector, &std_string_to_py_bytes);
    if (! py_list) {
        // Handle error condition.
    } else {
        // All good.
    }

------------------------------------------------------------
Python Sets, Frozensets and C++ ``std::unordered_set<T>``
------------------------------------------------------------

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Python ``set`` to C++ ``std::unordered_set<T>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Convert a Python ``set`` or ``frozenset`` to a ``std::unordered_set<T>``. ``ConvertToT`` is a function pointer to a function that takes a ``PyObject*`` and returns an instance of a ``T`` type. This function should make ``PyErr_Occurred()`` true on failure to convert a ``PyObject*`` and return a default ``T``. On failure this sets ``PyErr_Occurred()`` and the return value will be an empty container.

.. code-block:: cpp

    template <typename T>
    std::unordered_set<T>
    py_set_to_std_unordered_set(PyObject *py_set, T (*ConvertToT)(PyObject *)) {
        assert(cpython_asserts(py_set));
        std::unordered_set<T> cpp_set;

        if (PySet_Check(py_set) || PyFrozenSet_Check(py_set)) {
            // The C API does not allow direct access to an item in a set so we
            // make a copy and pop from that.
            PyObject *set_copy = PySet_New(py_set);
            if (set_copy) {
                while (PySet_GET_SIZE(set_copy)) {
                    PyObject *item = PySet_Pop(set_copy);
                    if (! item || PyErr_Occurred()) {
                        PySet_Clear(set_copy);
                        cpp_set.clear();
                        break;
                    }
                    cpp_set.emplace((*ConvertToT)(item));
                    Py_DECREF(item);
                }
                Py_DECREF(set_copy);
            } else {
                assert(PyErr_Occurred());
            }
        } else {
            PyErr_Format(PyExc_TypeError,
                 "Argument \"py_set\" to %s must be set or frozenset not \"%s\"",
                 __FUNCTION__, Py_TYPE(py_set)->tp_name);
        }
        return cpp_set;
    }


^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
C++ ``std::unordered_set<T>`` to Python ``set`` or ``frozenset``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Convert a ``std::unordered_set<T>`` to a new Python ``set`` or ``frozenset``. ``ConvertToPy`` is a pointer to a function that takes an instance of a ``T`` type and returns a ``PyObject*``, this function should return NULL on failure. On failure this function sets ``PyErr_Occurred()`` and returns NULL.

.. code-block:: cpp

    template <typename T>
    PyObject*
    std_unordered_set_to_py_set(const std::unordered_set<T> &cpp_set,
                                PyObject *(*ConvertToPy)(const T&),
                                bool is_frozen=false) {
        assert(cpython_asserts());
        PyObject *r = NULL;
        if (is_frozen) {
            r = PyFrozenSet_New(NULL);
        } else {
            r = PySet_New(NULL);
        }
        if (! r) {
            goto except;
        }
        for (auto &iter: cpp_set) {
            PyObject *item = (*ConvertToPy)(iter);
            if (! item || PyErr_Occurred() || PySet_Add(r, item)) {
                goto except;
            }
        }
        assert(! PyErr_Occurred());
        assert(r);
        goto finally;
    except:
        assert(PyErr_Occurred());
        // Clean up set
        if (r) {
            PySet_Clear(r);
            Py_DECREF(r);
            r = NULL;
        }
    finally:
        return r;
    }

-----------------------------------------------------
Python Dicts and C++ ``std::unordered_map<K, V>``
-----------------------------------------------------

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Python ``dict`` to C++ ``std::unordered_map<K, V>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Convert a Python ``dict`` to a ``std::unordered_map<K, V>``. ``PyKeyConvertToK`` and ``PyKeyConvertToK`` are function pointers to functions that takes a ``PyObject*`` and returns an instance of a ``K`` or ``V`` type. On failure to convert a ``PyObject*`` this function should make ``PyErr_Occurred()`` true and return a default value.

On failure this function will make ``PyErr_Occurred()`` non-NULL and return an empty map.

.. code-block:: cpp

    template <typename K, typename V>
    std::unordered_map<K, V>
    py_dict_to_std_unordered_map(PyObject *dict,
                                 K (*PyKeyConvertToK)(PyObject *),
                                 V (*PyValConvertToV)(PyObject *)
                                 ) {
        Py_ssize_t pos = 0;
        PyObject *key = NULL;
        PyObject *val = NULL;
        std::unordered_map<K, V> cpp_map;

        if (! PyDict_Check(dict)) {
            PyErr_Format(PyExc_TypeError,
                         "Argument \"dict\" to %s must be dict not \"%s\"",
                         __FUNCTION__, Py_TYPE(dict)->tp_name);
            return cpp_map;
        }
        while (PyDict_Next(dict, &pos, &key, &val)) {
            K cpp_key = (*PyKeyConvertToK)(key);
            if (PyErr_Occurred()) {
                cpp_map.clear();
                break;
            }
            V cpp_val = (*PyValConvertToV)(val);
            if (PyErr_Occurred()) {
                cpp_map.clear();
                break;
            }
            cpp_map.emplace(cpp_key, cpp_val);
        }
        return cpp_map;
    }

The following expects a Python dict of ``{bytes : bytes}`` and will convert it to a ``std::unordered_map<std::string, std::string>``:

.. code-block:: cpp

    std::unordered_map<std::string, std::string> result;
    result = py_dict_to_std_unordered_map(py_dict,
                                          &py_bytes_to_std_string,
                                          &py_bytes_to_std_string);
    if (PyErr_Occurred()) {
        // Handle failure...
    } else {
        // Success...
    }

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
C++ ``std::unordered_map<K, V>`` to Python ``dict``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This generic function converts a ``std::unordered_map<K, V>`` to a new Python ``dict``. ``KeyConvertToPy``, ``ValConvertToPy`` are pointers to functions that takes an instance of a ``K`` or ``V`` type and returns a ``PyObject*``. These should return a new reference on success, NULL on failure.

.. code-block:: cpp

    template <typename K, typename V>
    PyObject*
    std_unordered_map_to_py_dict(const std::unordered_map<K, V> &cpp_map,
                                 PyObject *(*KeyConvertToPy)(const K&),
                                 PyObject *(*ValConvertToPy)(const V&)
                                 ) {
        PyObject *key = NULL;
        PyObject *val = NULL;
        PyObject *r = PyDict_New();

        if (!r) {
            goto except;
        }
        for (auto &iter: cpp_map) {
            key = (*KeyConvertToPy)(iter.first);
            if (! key || PyErr_Occurred()) {
                goto except;
            }
            val = (*ValConvertToPy)(iter.second);
            if (! val || PyErr_Occurred()) {
                goto except;
            }
            if (PyDict_SetItem(r, key, val)) {
                goto except;
            }
        }
        assert(! PyErr_Occurred());
        assert(r);
        goto finally;
    except:
        assert(PyErr_Occurred());
        // Clean up dict
        if (r) {
            PyDict_Clear(r);
            Py_DECREF(r);
        }
        r = NULL;
    finally:
        return r;
    }

The following will convert a ``std::unordered_map<std::string, std::string>`` to a Python dict ``{bytes : bytes}``:

.. code-block:: cpp

    std::unordered_map<std::string, std::string> cpp_map {
        {"Foo", "Bar"}
    };
    PyObject *py_dict = std_unordered_map_to_py_dict<std::string, std::string>(
            cpp_map,
            &std_string_to_py_bytes,
            &std_string_to_py_bytes
    );
    if (! py_dict) {
        // Handle failure...
    } else {
        // All good...
    }
 
