#ifndef PLISP_COMPILE_H
#define PLISP_COMPILE_H

#include <plisp/object.h>
#include <lightning.h>

void plisp_init_compiler(char *argv0);
void plisp_end_compiler(void);
plisp_fn_t plisp_compile_lambda(plisp_t lambda);

void plisp_free_fn(plisp_fn_t fn);
void plisp_disassemble_fn(plisp_fn_t fn);

#endif
