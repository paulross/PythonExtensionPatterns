//
// Created by Paul Ross on 01/08/2025.
//

#ifndef PYTHONEXTENSIONPATTERNS_DEBUGSTRUCTSEQ_H
#define PYTHONEXTENSIONPATTERNS_DEBUGSTRUCTSEQ_H

#include "DebugSigsegv.h"

#pragma mark - Struct Sequence

void dbg_PyStructSequence_simple_ctor(void);
void dbg_PyStructSequence_setitem_abandons(void);
void dbg_PyStructSequence_n_in_sequence_too_large(void);
void dbg_PyStructSequence_with_unnamed_field(void);

#endif //PYTHONEXTENSIONPATTERNS_DEBUGSTRUCTSEQ_H
