#ifndef PLISP_GC_H
#define PLISP_GC_H

#include <plisp/object.h>


plisp_t plisp_alloc_obj(uintptr_t tags);

bool plisp_heap_allocated(plisp_t obj);

void plisp_gc_permanent(plisp_t obj);
//void plisp_gc_nopermanent(plisp_t obj);

#endif
