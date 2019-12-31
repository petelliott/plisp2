#ifndef PLISP_BUILTIN_H
#define PLISP_BUILTIN_H

#include <plisp/object.h>

void plisp_init_builtin(void);

void plisp_define_builtin(const char *name, plisp_fn_t fun);

plisp_t plisp_plus(plisp_t a, plisp_t b);
plisp_t plisp_minus(plisp_t a, plisp_t b);

#endif
