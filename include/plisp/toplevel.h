#ifndef PLISP_TOPLEVEL_H
#define PLISP_TOPLEVEL_H

#include <plisp/object.h>

void plisp_init_toplevel(void);

void plisp_toplevel_define(plisp_t sym, plisp_t value);
plisp_t *plisp_toplevel_ref(plisp_t sym);

plisp_t plisp_toplevel_eval(plisp_t form);

#endif
