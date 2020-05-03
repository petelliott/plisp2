#ifndef PLISP_CONTINUATION_H
#define PLISP_CONTINUATION_H

#include <plisp/object.h>

void plisp_init_continuation(void);


plisp_t plisp_builtin_callcc(plisp_t *clos, size_t nargs, plisp_t proc);

#endif
