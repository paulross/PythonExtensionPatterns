//
// Created by Paul Ross on 15/12/2024.
//

#ifndef PYTHONEXTENSIONPATTERNS_DEBUGCONTAINERS_H
#define PYTHONEXTENSIONPATTERNS_DEBUGCONTAINERS_H

PyObject *new_unique_string(const char *function_name, const char *suffix);

void dbg_PyTuple_SetItem_steals(void);
void dbg_PyTuple_SET_ITEM_steals(void);
void dbg_PyTuple_SetItem_steals_replace(void);
void dbg_PyTuple_SET_ITEM_steals_replace(void);
void dbg_PyTuple_SetIem_NULL(void);
void dbg_PyTuple_SET_ITEM_NULL(void);
void dbg_PyTuple_SetIem_NULL_SetItem(void);
void dbg_PyTuple_SET_ITEM_NULL_SET_ITEM(void);

#endif //PYTHONEXTENSIONPATTERNS_DEBUGCONTAINERS_H
