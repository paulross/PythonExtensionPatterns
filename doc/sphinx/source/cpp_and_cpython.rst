.. highlight:: cpp
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _cpp_and_cpython:

********************************************
Using C++ to Interface With CPython Code 
********************************************

.. code-block:: cpp

    /* Returns non zero if Python is initialised and there is no Python error set.
     * The second version also checks that the given pointer is non-NULL
     * Use this thus, it will do nothing if NDEBUG is defined:
     *
     * assert(cpython_asserts());
     * assert(cpython_asserts(p));
     */
    int cpython_asserts() {
        return Py_IsInitialized() && PyErr_Occurred() == NULL;
    }

    int cpython_asserts(PyObject *pobj) {
        return cpython_asserts() && pobj != NULL;
    }


============================================
C++ RAII Wrappers Around ``PyObject*``
============================================

It is sometimes useful to wrap up a ``PyObject*`` in a class that will manage the reference count: 

.. code-block:: cpp

    /** General wrapper around a PyObject*.
     * This decrements the reference count on destruction.
     */
    class DecRefDtor {
    public:
        DecRefDtor(PyObject *ref) : m_ref { ref } {}
        Py_ssize_t ref_count() const { return m_ref ? Py_REFCNT(m_ref) : 0; }
        // Allow setting of the (optional) argument with
        // PyArg_ParseTupleAndKeywords
        PyObject **operator&() {
            Py_XDECREF(m_ref);
            m_ref = NULL;
            return &m_ref;
        }
        // Access the argument or the default if default.
        operator PyObject*() const {
            return m_ref;
        }
        // Test if constructed successfully from the new reference.
        explicit operator bool() { return m_ref != NULL; }
        ~DecRefDtor() { Py_XDECREF(m_ref); }
    protected:
        PyObject *m_ref;
    };

There are two useful sub-classes, one for borrowed references, one for new references that are temporary: 

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

    /** Wrapper around a PyObject* that is a new reference.
     * This owns the reference so does not increment it on construction but
     * does decrement it on destruction.
     */
    class NewRef : public DecRefDtor {
    public:
        NewRef(PyObject *new_ref) : DecRefDtor(new_ref) {}
    };


============================================
Homogeneous Python Containers and C++
============================================

Here are some useful generic functions that can convert homogeneous Python containers to and from their C++ STL equivalents. They use function pointers to identify the conversion function from Python to C++ objects.

Here are a couple of such functions that convert ``PyObject*`` to and from ``std::string``:

.. code-block:: cpp

    PyObject *std_string_to_py_utf8(const std::string &str) {
        return PyUnicode_FromStringAndSize(str.c_str(), str.size());
    }

    std::string py_utf8_to_std_string(PyObject *py_str) {
        assert(cpython_asserts(py_str));
        std::string r;

        if (PyUnicode_Check(py_str)) {
            if (! PyUnicode_READY(py_str)) {
                if (PyUnicode_KIND(py_str) == PyUnicode_1BYTE_KIND) {
                    // Python 3 and its minor versions (they vary)
                    //    const Py_UCS1 *pChrs = PyUnicode_1BYTE_DATA(pyStr);
                    //    result = std::string(reinterpret_cast<const char*>(pChrs));
    #if PY_MAJOR_VERSION >= 3
                    r = std::string((char*)PyUnicode_1BYTE_DATA(py_str));
    #else
                    // Nasty cast away constness because PyString_AsString takes non-const in Py2
                    r = std::string((char*)PyString_AsString(const_cast<PyObject *>(py_str)));
    #endif
                } else {
                    PyErr_Format(PyExc_ValueError, "In %s \"py_str\" not utf-8", __FUNCTION__);
                }
            } else {
                PyErr_Format(PyExc_ValueError, "In %s \"py_str\" failed PyUnicode_READY()", __FUNCTION__);
            }
        } else {
            PyErr_Format(PyExc_TypeError, "Argument to %s must be unicode string not \"%s\"",
                         __FUNCTION__, Py_TYPE(py_str)->tp_name);
        }
        return r;
    }



--------------------------------------------
Python Lists and C++ ``std::vector<T>``
--------------------------------------------

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Python ``list`` to C++ ``std::vector<T>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This converts a python list to a ``std::vector<T>``. ``ConvertToT`` is a function pointer to a function that takes a ``PyObject*`` and returns an instance of a ``T`` type. On failure to to convert a ``PyObject*`` this function should set a Python error (making ``PyErr_Occurred()`` non-NULL) and return a default ``T``. On failure this function sets PyErr_Occurred() and the return value will be empty.

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

If we have a function ``PyObject *std_string_to_py_utf8(const std::string &);`` we can use this thus:

.. code-block:: cpp

    std::vector<std::string> cpp_vector;
    // Initialise cpp_vector...
    PyObject *py_list = std_vector_to_py_list(cpp_vector, &std_string_to_py_utf8);
    if (! py_list) {
        // Handle error condition.
    } else {
        // All good.
    }

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
C++ ``std::vector<T>`` to Python ``list``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

And the inverse that takes a C++ ``std::vector<T>`` and makes a Python ``list``. ``ConvertToPy`` is a pointer to a function that takes an instance of a ``T`` type and returns a ``PyObject*``, this should return NULL on failure and set ``PyErr_Occurred()``.

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

If we have a function ``std::string py_utf8_to_std_string(PyObject *);`` we can use this thus, we have to specify the C++ template specialisation:

.. code-block:: cpp

    std::vector<std::string> result = py_list_to_std_vector<std::string>(py_list, &py_utf8_to_std_string);
    if (PyErr_Occurred()) {
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

Convert a Python ``set`` or ``frozenset`` to a ``std::unordered_set<T>``. ``ConvertToT`` is a function pointer to a function that takes a ``PyObject*`` and returns an instance of a ``T`` type. This function should make ``PyErr_Occurred()`` true on failure to convert a ``PyObject*`` and return a default ``T``.

On failure sets ``PyErr_Occurred()`` and the return value will be empty.

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

Convert a ``std::unordered_set<T>`` to a new Python ``set`` or ``frozenset``. ``ConvertToPy`` is a pointer to a function that takes an instance of a ``T`` type and returns a ``PyObject*``, this should return NULL on failure.

Returns a new reference on success, NULL on failure.

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

Convert a Python ``dict`` to a ``std::unordered_map<K, V>``. ``PyKeyConvertToK`` and ``PyKeyConvertToK`` are function pointers to functions that takes a ``PyObject*`` and returns an instance of a ``K`` or ``V`` type. This function should make ``PyErr_Occurred()`` true on failure to convert a ``PyObject*`` and return a default value.

On failure this sets ``PyErr_Occurred()`` and the return value will be empty.

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

The following expects a Python dict of ``{str : str}`` and will convert it to a ``std::unordered_map<std::string, std::string>``:

.. code-block:: cpp

    std::unordered_map<std::string, std::string> result;
    result = py_dict_to_std_unordered_map(py_dict,
                                          &py_utf8_to_std_string,
                                          &py_utf8_to_std_string);
    if (PyErr_Occurred()) {
        // Handle failure...
    } else {
        // Success...
    }

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
C++ ``std::unordered_map<K, V>`` to Python ``dict``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Convert a ``std::unordered_map<K, V>`` to a new Python ``dict``. ``KeyConvertToPy``, ``ValConvertToPy`` are pointers to functions that takes an instance of a ``K`` or ``V`` type and returns a ``PyObject*``, this should return NULL on failure.

Returns a new reference on success, NULL on failure.

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

The following will convert a ``std::unordered_map<std::string, std::string>`` to a Python dict:

.. code-block:: cpp

    std::unordered_map<std::string, std::string> cpp_map {
        {"Foo", "Bar"}
    };
    PyObject *py_dict = std_unordered_map_to_py_dict<std::string, std::string>(
            cpp_map,
            &std_string_to_py_utf8,
            &std_string_to_py_utf8
    );
    if (! py_dict) {
        // Handle failure...
    } else {
        // All good...
    }
 
