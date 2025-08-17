//
// Created by Paul Ross on 16/08/2025.
//

#ifndef PYTHONEXTENSIONPATTERNS_TRACKALLOCS_H
#define PYTHONEXTENSIONPATTERNS_TRACKALLOCS_H

#include "Python.h"

#include <map>
#include <string>

/** POD class that contains the location of the new instance creation. */
struct NewAndDeallocTrackedValue {
    std::string function;
    std::string file;
    int line;
};

class NewAndDeallocTracker {
public:
    /** Add a new instance of an object to the map. */
    int add_new(PyObject *op, const char *function, const char *file, int line);

    /** Remove an instance of an object from the map. */
    int add_dealloc(PyObject *op);

    /** Returns a summary string of all the currently live objects. */
    std::string dump_remaining();

    /** Access methods. */
    /** Return the total number of currently live objects. */
    size_t len() const { return m_tracker_map.size(); }

    /** Return the total number of new objects ever created. */
    size_t total_new() const { return m_total_new; }

    /** Return the total tp_basicsize of new objects ever created. */
    size_t total_tp_basicsize() const { return m_total_tp_basicsize; }

    /** Return the total number of objects ever de-allocated. */
    size_t total_dealloc() const { return m_total_dealloc; }

    /** Return the maximum number of allocations at any time. */
    size_t max_allocs() const { return m_max_allocs; }

protected:
    std::map<PyObject *, struct NewAndDeallocTrackedValue> m_tracker_map;
    size_t m_total_new = 0;
    size_t m_total_tp_basicsize = 0;
    size_t m_total_dealloc = 0;
    size_t m_max_allocs = 0;
};

#endif //PYTHONEXTENSIONPATTERNS_TRACKALLOCS_H
