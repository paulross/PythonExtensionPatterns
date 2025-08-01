//
// Created by Paul Ross on 01/08/2025.
//

#ifndef PYTHONEXTENSIONPATTERNS_DEBUGLIST_H
#define PYTHONEXTENSIONPATTERNS_DEBUGLIST_H

#include "DebugSigsegv.h"

#pragma mark - Lists
void dbg_PyList_SetItem_steals(void);
void dbg_PyList_SET_ITEM_steals(void);
void dbg_PyList_SetItem_steals_replace(void);
void dbg_PyList_SET_ITEM_steals_replace(void);
void dbg_PyList_SetItem_replace_with_same(void);
void dbg_PyList_SET_ITEM_replace_with_same(void);
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

#if ACCEPT_SIGSEGV
void dbg_PyList_SetItem_SIGSEGV_on_same_value(void);
#endif

#endif //PYTHONEXTENSIONPATTERNS_DEBUGLIST_H
