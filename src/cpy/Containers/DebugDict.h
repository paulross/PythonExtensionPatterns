//
// Created by Paul Ross on 01/08/2025.
//

#ifndef PYTHONEXTENSIONPATTERNS_DEBUGDICT_H
#define PYTHONEXTENSIONPATTERNS_DEBUGDICT_H

#include "DebugSigsegv.h"

#pragma mark - Dictionaries - setters
void dbg_PyDict_SetItem_increments(void);

void dbg_PyDict_SetItem_fails_not_a_dict(void);
void dbg_PyDict_SetItem_fails_not_hashable(void);
void dbg_PyDict_SetDefault_default_unused(void);
void dbg_PyDict_SetDefault_default_used(void);
void dbg_PyDict_SetDefaultRef_default_unused(void);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
void dbg_PyDict_SetDefaultRef_default_used(void);
void dbg_PyDict_SetDefaultRef_default_unused_result_non_null(void);
#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

#pragma mark - Dictionaries - getters
void dbg_PyDict_GetItem(void);
void dbg_PyDict_Next(void);
#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
void dbg_PyDict_GetItemRef(void);
#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
void dbg_PyDict_GetItemWithError_fails(void);

#pragma mark - Dictionaries - deleters

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13
void dbg_PyDict_Pop_key_present(void);
void dbg_PyDict_Pop_key_absent(void);
#endif // #if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 13

#if ACCEPT_SIGSEGV
void dbg_PyDict_SetItem_SIGSEGV_on_key_NULL(void);
void dbg_PyDict_SetItem_SIGSEGV_on_value_NULL(void);
void dbg_PyDict_GetItem_key_NULL(void);
#endif

#endif //PYTHONEXTENSIONPATTERNS_DEBUGDICT_H
