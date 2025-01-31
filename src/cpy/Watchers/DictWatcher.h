//
// Created by Paul Ross on 30/01/2025.
//
// Explores a dict watcher: https://docs.python.org/3/c-api/dict.html#c.PyDict_AddWatcher

#ifndef PYTHONEXTENSIONPATTERNS_DICTWATCHER_H
#define PYTHONEXTENSIONPATTERNS_DICTWATCHER_H

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12

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

#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 12

#endif //PYTHONEXTENSIONPATTERNS_DICTWATCHER_H
