#ifndef PLISP_BUILTIN_H
#define PLISP_BUILTIN_H

#include <plisp/object.h>

void plisp_init_builtin(void);

void plisp_define_builtin(const char *name, plisp_fn_t fun);

plisp_t plisp_builtin_plus(size_t nargs, plisp_t a, plisp_t b, ...);
plisp_t plisp_builtin_minus(size_t nargs, plisp_t a, plisp_t b, ...);

plisp_t plisp_builtin_cons(size_t nargs, plisp_t car, plisp_t cdr);
plisp_t plisp_builtin_car(size_t nargs, plisp_t cell);
plisp_t plisp_builtin_cdr(size_t nargs, plisp_t cell);

plisp_t plisp_c_reverse(plisp_t lst);
plisp_t plisp_builtin_reverse(size_t nargs, plisp_t lst, plisp_t onto);
plisp_t plisp_builtin_list(size_t nargs, ...);

plisp_t plisp_builtin_not(size_t nargs, plisp_t obj);
plisp_t plisp_builtin_nullp(size_t nargs, plisp_t obj);
plisp_t plisp_builtin_eq(size_t nargs, plisp_t a, plisp_t b);
plisp_t plisp_builtin_lt(size_t nargs, plisp_t a, plisp_t b);

size_t plisp_c_length(plisp_t lst);
plisp_t plisp_builtin_length(size_t nargs, plisp_t lst);

plisp_t plisp_builtin_display(size_t nargs, plisp_t obj);
plisp_t plisp_builtin_newline(size_t nargs);

#endif
