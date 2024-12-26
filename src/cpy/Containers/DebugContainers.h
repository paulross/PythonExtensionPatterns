//
// Created by Paul Ross on 15/12/2024.
//

#ifndef PYTHONEXTENSIONPATTERNS_DEBUGCONTAINERS_H
#define PYTHONEXTENSIONPATTERNS_DEBUGCONTAINERS_H

#define ACCEPT_SIGSEGV 0

PyObject *new_unique_string(const char *function_name, const char *suffix);
#pragma mark - Tuples
void dbg_PyTuple_SetItem_steals(void);
void dbg_PyTuple_SET_ITEM_steals(void);
void dbg_PyTuple_SetItem_steals_replace(void);
void dbg_PyTuple_SET_ITEM_steals_replace(void);
void dbg_PyTuple_SetIem_NULL(void);
void dbg_PyTuple_SET_ITEM_NULL(void);
void dbg_PyTuple_SetIem_NULL_SetItem(void);
void dbg_PyTuple_SET_ITEM_NULL_SET_ITEM(void);
void dbg_PyTuple_SetItem_fails_not_a_tuple(void);
void dbg_PyTuple_SetItem_fails_out_of_range(void);
void dbg_PyTuple_PyTuple_Pack(void);
void dbg_PyTuple_Py_BuildValue(void);
#pragma mark - Lists
void dbg_PyList_SetItem_steals(void);
void dbg_PyList_SET_ITEM_steals(void);
void dbg_PyList_SetItem_steals_replace(void);
void dbg_PyList_SET_ITEM_steals_replace(void);
void dbg_PyList_SetIem_NULL(void);
void dbg_PyList_SET_ITEM_NULL(void);
void dbg_PyList_SetIem_NULL_SetItem(void);
void dbg_PyList_SET_ITEM_NULL_SET_ITEM(void);
void dbg_PyList_SetItem_fails_not_a_tuple(void);
void dbg_PyList_SetItem_fails_out_of_range(void);
void dbg_PyList_Append(void);
void dbg_PyList_Append_fails_not_a_list(void);
void dbg_PyList_Append_fails_NULL(void);
void dbg_PyList_Insert(void);
void dbg_PyList_Insert_Is_Truncated(void);
void dbg_PyList_Insert_Negative_Index(void);
void dbg_PyList_Insert_fails_not_a_list(void);
void dbg_PyList_Insert_fails_NULL(void);
void dbg_PyList_Py_BuildValue(void);
#pragma mark - Dictionaries
void dbg_PyDict_SetItem_increments(void);
void dbg_PyDict_SetItem_fails_not_a_dict(void);
void dbg_PyDict_SetItem_fails_not_hashable(void);
#if ACCEPT_SIGSEGV
void dbg_PyDict_SetItem_SIGSEGV_on_key_NULL(void);
void dbg_PyDict_SetItem_SIGSEGV_on_value_NULL(void);
#endif

#endif //PYTHONEXTENSIONPATTERNS_DEBUGCONTAINERS_H
