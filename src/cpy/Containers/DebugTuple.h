//
// Created by Paul Ross on 01/08/2025.
//

#ifndef PYTHONEXTENSIONPATTERNS_DEBUGTUPLE_H
#define PYTHONEXTENSIONPATTERNS_DEBUGTUPLE_H

#include "DebugSigsegv.h"

#pragma mark - Tuples
void dbg_PyTuple_SetItem_steals(void);
void dbg_PyTuple_SET_ITEM_steals(void);
void dbg_PyTuple_SetItem_steals_replace(void);
void dbg_PyTuple_SET_ITEM_steals_replace(void);
void dbg_PyTuple_SetItem_replace_with_same(void);
void dbg_PyTuple_SET_ITEM_replace_with_same(void);
void dbg_PyTuple_SetIem_NULL(void);
void dbg_PyTuple_SET_ITEM_NULL(void);
void dbg_PyTuple_SetIem_NULL_SetItem(void);
void dbg_PyTuple_SET_ITEM_NULL_SET_ITEM(void);
void dbg_PyTuple_SetItem_fails_not_a_tuple(void);
void dbg_PyTuple_SetItem_fails_out_of_range(void);
void dbg_PyTuple_PyTuple_Pack(void);
void dbg_PyTuple_Py_BuildValue(void);

#if ACCEPT_SIGSEGV
void dbg_PyTuple_SetItem_SIGSEGV_on_same_value(void);
#endif

#endif //PYTHONEXTENSIONPATTERNS_DEBUGTUPLE_H
