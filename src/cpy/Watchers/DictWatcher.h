//
// Created by Paul Ross on 30/01/2025.
//
// Explores a dict watcher: https://docs.python.org/3/c-api/dict.html#c.PyDict_AddWatcher

#ifndef PYTHONEXTENSIONPATTERNS_DICTWATCHER_H
#define PYTHONEXTENSIONPATTERNS_DICTWATCHER_H

#define PPY_SSIZE_T_CLEAN

#include "Python.h"

/* Version as a single 4-byte hex number, e.g. 0x010502B2 == 1.5.2b2
 * Therefore 0x030C0000 == 3.12.0
 */
#if PY_VERSION_HEX < 0x030C0000

#error "Required version of Python is 3.12+ (PY_VERSION_HEX >= 0x030C0000)"

#else

long get_static_dict_added(void);
long get_static_dict_modified(void);
long get_static_dict_deleted(void);
long get_static_dict_cloned(void);
long get_static_dict_cleared(void);
long get_static_dict_deallocated(void);

void dbg_PyDict_EVENT_ADDED(void);
void dbg_PyDict_EVENT_MODIFIED(void);
void dbg_PyDict_EVENT_MODIFIED_same_value_no_event(void);


int dict_watcher_verbose_add(PyObject *dict);

int dict_watcher_verbose_remove(int watcher_id, PyObject *dict);

#endif // #if PY_VERSION_HEX >= 0x030C0000

#endif //PYTHONEXTENSIONPATTERNS_DICTWATCHER_H
