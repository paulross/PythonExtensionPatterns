//
// Created by Paul Ross on 16/08/2025.
//

#ifndef PYTHONEXTENSIONPATTERNS_TRACKALLOCS_H
#define PYTHONEXTENSIONPATTERNS_TRACKALLOCS_H

#include "Python.h"

#include <map>
#include <string>

struct NewAndDeallocTrackedValue {
    std::string function;
    std::string file;
    int line;
};

class NewAndDeallocTracker {
public:
    int add_new(PyObject *op, const char *function, const char *file, int line);
    int add_dealloc(PyObject *op);
    std::string dump_remaining();
    size_t len() const { return m_tracker_map.size(); }
    size_t total_new() const { return m_total_new; }
    size_t total_tp_basicsize() const { return m_total_tp_basicsize; }
    size_t total_dealloc() const { return m_total_dealloc; }
    size_t max_allocs() const { return m_max_allocs; }
private:
    std::map<PyObject*, struct NewAndDeallocTrackedValue> m_tracker_map;
    size_t m_total_new = 0;
    size_t m_total_tp_basicsize = 0;
    size_t m_total_dealloc = 0;
    size_t m_max_allocs = 0;
};

#endif //PYTHONEXTENSIONPATTERNS_TRACKALLOCS_H
