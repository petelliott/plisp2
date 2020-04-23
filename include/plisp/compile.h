#ifndef PLISP_COMPILE_H
#define PLISP_COMPILE_H

#include <plisp/object.h>
#include <lightning.h>

void plisp_init_compiler(char *argv0);
void plisp_end_compiler(void);
plisp_fn_t plisp_compile_lambda(plisp_t lambda, jit_state_t **fn_jit_state);

#endif
