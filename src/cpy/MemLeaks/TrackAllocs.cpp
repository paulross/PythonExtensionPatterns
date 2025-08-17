//
// Created by Paul Ross on 16/08/2025.
//

#include "TrackAllocs.h"

#include <iomanip>
#include <sstream>

int NewAndDeallocTracker::add_new(PyObject *op, const char *function, const char *file, int line) {
    if(!op) {
        return -1;
    }
    if (m_tracker_map.find(op) != m_tracker_map.end()) {
        return -2;
    }
    m_tracker_map[op] = {function, file, line};
    m_total_new++;
    m_total_tp_basicsize += op->ob_type->tp_basicsize;
    if (m_max_allocs < m_tracker_map.size()) {
        m_max_allocs = m_tracker_map.size();
    }
    return 0;
}

int NewAndDeallocTracker::add_dealloc(PyObject *op) {
    if(!op) {
        return -1;
    }
    if (m_tracker_map.find(op) == m_tracker_map.end()) {
        return -2;
    }
    m_tracker_map.erase(op);
    m_total_dealloc++;
    return 0;
}

std::string NewAndDeallocTracker::dump_remaining() {
    std::ostringstream os;
    int count = 0;
    os << "NewAndDeallocTracker.dump_remaining() [" << m_tracker_map.size() << "]" << std::endl;
    os << "Count new: " << m_total_new << " Sum tp_basicsize: " << m_total_tp_basicsize;
    os << " Count dealloc: " << m_total_dealloc << " Max: " << m_max_allocs;
    os << std::endl;
    size_t total_tp_basicsize = 0;
    for (const auto &iter: m_tracker_map) {
        total_tp_basicsize += iter.first->ob_type->tp_basicsize;
    }
    os << "Total tp_basicsize remaining: " << total_tp_basicsize;
    os << std::endl;
    for (const auto &iter: m_tracker_map) {
        os << "[" << std::setw(4) << count << "]";
        os << " PyObject: " << iter.first;
        os << " Type: " << Py_TYPE(iter.first)->tp_name;
        os << " Size: " << Py_TYPE(iter.first)->tp_basicsize;
        os << " From: " << iter.second.function;
        os << " " << iter.second.file;
        os << " " << iter.second.line;
        os << std::endl;
        count++;
    }
    os << "NewAndDeallocTracker.dump_remaining(): DONE";
    return os.str();
}
