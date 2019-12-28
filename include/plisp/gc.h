#ifndef PLISP_GC_H
#define PLISP_GC_H

#include <plisp/object.h>


plisp_t plisp_alloc_atomic(size_t len, uintptr_t tags);

plisp_t plisp_alloc_obj(uintptr_t tags);

#endif
